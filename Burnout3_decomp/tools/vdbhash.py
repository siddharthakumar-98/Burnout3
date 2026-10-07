#!/usr/bin/env python3
"""The tuning database's key hash, and a reader for your own Data/vdb.xml (plain Python 3, no container needed).

    python3 tools/vdbhash.py hash NAME GROUP CFG     key of one variable
    python3 tools/vdbhash.py check VDB.XML           header, tables, and which ELF names resolve

Despite its name, Data/vdb.xml is binary, little-endian (docs/d4.md, "Tuning"):

    +0x00  version (2)
    +0x04  n_values      +0x08  ?     +0x0C  n_files     +0x10  offset of the file table
    +0x14  value table:  n_values x {u32 value, s32 key}, sorted by key (signed); the value is the variable itself
                         (int, float, bool) or, for vectors and arrays, an offset into the file
    file table:          n_files x {u32 enabled, s32 hash(cfg path)}, sorted by hash

A variable is registered with a name, a group and the path of the .cfg file that defined it in the tools
(func_0021B9B0, func_0021BB60, func_0021BD80). Its key is hash(name + group + "/" + cfg), the "/" left out when the
group already ends with one; the file table is looked up with hash(cfg) alone (func_0024FDE0). The hash
(func_0021BFA0) is CRC-32's table-driven loop over the standard reflected table (0x49B110), started at 0xFFFFFFFF,
with no final inversion, an arithmetic shift (the CRC is a signed int) and signed characters.

`check` hashes the ELF's .cfg strings against the file table, then every name/group/cfg triple it can read from the
registration calls in the generated assembly against the value table. Nothing is written; vdb.xml stays yours.
"""

import argparse
import re
import struct
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
ROM = ROOT / "orig/SLUS_210.50.rom"
BASE = 0x100000


def _table() -> list[int]:
    out = []
    for i in range(256):
        c = i
        for _ in range(8):
            c = (c >> 1) ^ (0xEDB88320 if c & 1 else 0)
        out.append(c)
    return out


TABLE = _table()


def vdb_hash(s: bytes) -> int:
    """func_0021BFA0: the key of a string, as an unsigned 32-bit value."""
    crc = -1
    for b in s:
        c = b - 256 if b >= 0x80 else b  # char is signed
        crc = (crc >> 8) ^ TABLE[(c ^ (crc & 0xFF)) & 0xFF]  # >> on a negative int is arithmetic
        crc = (crc + 0x80000000) % 0x100000000 - 0x80000000
    return crc & 0xFFFFFFFF


def key(name: bytes, group: bytes, cfg: bytes) -> int:
    return vdb_hash(name + group + (b"" if group.endswith(b"/") else b"/") + cfg)


def read_vdb(path: Path) -> tuple[int, dict[int, int], dict[int, int]]:
    v = path.read_bytes()
    version, n_values, _, n_files, files = struct.unpack_from("<5I", v, 0)
    values = {struct.unpack_from("<I", v, 0x14 + 8 * i + 4)[0]: struct.unpack_from("<I", v, 0x14 + 8 * i)[0]
              for i in range(n_values)}
    cfgs = {struct.unpack_from("<I", v, files + 8 * i + 4)[0]: struct.unpack_from("<I", v, files + 8 * i)[0]
            for i in range(n_files)}
    return version, values, cfgs


def registrations() -> list[tuple[bytes, bytes, bytes]]:
    """(name, group, cfg) of the registration calls whose three strings are visible in the generated assembly:
    the name in $a2 (lui/addiu), the group in $a3 and the cfg path in $t0 (each a pointer loaded through $gp)."""
    rom = ROM.read_bytes()
    cstr = lambda a: rom[a - BASE:rom.index(b"\0", a - BASE)]
    word = lambda a: struct.unpack_from("<I", rom, a - BASE)[0]
    out = []
    call = re.compile(r"jal\s+func_0021(B9B0|BB60|BD80)")
    for path in (ROOT / "assembly/asm").rglob("*.s"):
        lines = path.read_text().splitlines()
        for k, line in enumerate(lines):
            if not call.search(line):
                continue
            regs = {}
            for back in lines[max(0, k - 16):k + 2]:
                m = re.search(r"addiu\s+\$a2, \$a2, %lo\(D_([0-9A-F]{8})\)", back)
                if m:
                    regs["a2"] = int(m.group(1), 16)
                m = re.search(r"lw\s+\$(a3|t0), %gp_rel\(D_([0-9A-F]{8})\)\(\$gp\)", back)
                if m:
                    regs[m.group(1)] = word(int(m.group(2), 16))
            if len(regs) == 3:
                try:
                    out.append((cstr(regs["a2"]), cstr(regs["a3"]), cstr(regs["t0"])))
                except ValueError:
                    pass
    return out


def main() -> None:
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = p.add_subparsers(dest="cmd", required=True)
    h = sub.add_parser("hash", help="key of one variable")
    h.add_argument("name")
    h.add_argument("group")
    h.add_argument("cfg")
    c = sub.add_parser("check", help="read a vdb.xml and resolve what the ELF names")
    c.add_argument("vdb", type=Path)
    args = p.parse_args()
    if args.cmd == "hash":
        k = key(args.name.encode(), args.group.encode(), args.cfg.encode())
        print(f"0x{k:08X}")
        return
    version, values, cfgs = read_vdb(args.vdb)
    print(f"version {version}: {len(values)} values, {len(cfgs)} cfg files")
    rom = ROM.read_bytes()
    paths = set(re.findall(rb"[\x20-\x7e]{3,}\.cfg", rom))
    known = sorted(s for s in paths if vdb_hash(s) in cfgs)
    print(f"{len(known)} of the ELF's {len(paths)} .cfg paths are in the file table:")
    for s in known:
        print(f"  {'on ' if cfgs[vdb_hash(s)] else 'off'} {s.decode()}")
    regs = registrations()
    hits = [r for r in regs if key(*r) in values]
    in_db = [r for r in regs if vdb_hash(r[2]) in cfgs]
    print(f"{len(regs)} registrations with literal strings; {len(in_db)} from a cfg in the file table, "
          f"{len(hits)} of those found in the value table")
    missing = [r for r in in_db if r not in hits]
    for r in missing[:10]:
        print("  not found:", b" | ".join(r).decode(errors="replace"))
    # a few registrations have no value in the shipped database (the game keeps their defaults)
    sys.exit(0 if len(hits) >= 0.95 * len(in_db) else 1)


if __name__ == "__main__":
    main()
