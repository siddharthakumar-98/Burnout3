#!/usr/bin/env python3
"""Point a CodeWarrior object's float literals at the game's pooled .lit4 entries.

    python3 tools/litfix.py <object.o> <unit.s>

CodeWarrior puts every float literal it can't build with lui (0.85f, FLT_MAX, ...) in a .lit4 section of its own
and loads it with `lwc1 $fN, @lit($gp)` under an R_MIPS_LITERAL relocation. The original linker pooled those
literals from all files into one .lit4 area at 0x4E0680, ordered by first use and only partly deduplicated
(docs/layout.md), so neither the order nor the address can be reproduced from the object alone. Instead, each
R_MIPS_LITERAL relocation is retargeted at a pooled label with the same four bytes, referenced by the original
function, and becomes an ordinary R_MIPS_GPREL16 against it. Prefer the original instruction's label when its
value agrees; otherwise use the nearest original load of that value. The
object's own .lit4 sections are then unreferenced, and the linker script discards them.

A function's original address comes from its name (func_XXXXXXXX) or config/symbol_addrs.txt. Literals with no
verified original value are left alone with a warning, so an incorrect source constant cannot be silently
replaced with a different value. Plain Python, no dependencies; the object is
rewritten in place.
"""

import re
import struct
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SYMBOL_ADDRS = ROOT / "config/symbol_addrs.txt"
ORIG_ROM = ROOT / "orig/SLUS_210.50.rom"
LIT4_START, LIT4_END = 0x4E0680, 0x4E1400
R_MIPS_GPREL16, R_MIPS_LITERAL = 7, 8
SHT_SYMTAB, SHT_REL = 2, 9
INSN = re.compile(r"\s*/\* [0-9A-F]+ ([0-9A-F]{8}) [0-9A-F]{8} \*/.*%gp_rel\(([^)+]+)\)")


def known_addresses() -> dict[str, int]:
    out = {}
    for line in SYMBOL_ADDRS.read_text().splitlines():
        m = re.match(r"\s*(\S+)\s*=\s*0x([0-9A-Fa-f]+)\s*;", line)
        if m:
            out[m.group(1)] = int(m.group(2), 16)
    return out


def func_address(name: str, known: dict[str, int]) -> int | None:
    m = re.fullmatch(r"func_([0-9A-F]{8})", name)
    return int(m.group(1), 16) if m else known.get(name)


def gp_targets(asm: Path) -> dict[int, str]:
    """Instruction address -> symbol of its %gp_rel operand, from the original unit's assembly."""
    out = {}
    for line in asm.read_text().splitlines():
        m = INSN.match(line)
        if m:
            out[int(m.group(1), 16)] = m.group(2)
    return out


def pooled_loads(asm: Path, rom: bytes, known: dict[str, int]) -> dict[str, list[tuple[int, str, bytes]]]:
    """Verified original literal loads, restricted to the immutable .lit4 pool and their owning function."""
    loads: dict[str, list[tuple[int, str, bytes]]] = {}
    function = None
    for line in asm.read_text().splitlines():
        label = re.match(r"^glabel (\S+)$", line)
        if label:
            function = label.group(1)
        elif line.startswith("endlabel "):
            function = None
        match = INSN.match(line)
        if not function or not match:
            continue
        symbol = match.group(2)
        address = known.get(symbol)
        if address is None and re.fullmatch(r"D_[0-9A-F]{8}", symbol):
            address = int(symbol[2:], 16)
        if address is None or not LIT4_START <= address <= LIT4_END - 4:
            continue
        value = rom[address - 0x100000:address - 0x100000 + 4]
        if len(value) == 4:
            loads.setdefault(function, []).append((int(match.group(1), 16), symbol, value))
    return loads


def literal_target(value: bytes, instruction: int, loads: list[tuple[int, str, bytes]]) -> str | None:
    """Never substitute a different bit pattern, including signed zero or a nearby rounded float."""
    matching = [entry for entry in loads if entry[2] == value]
    return min(matching, key=lambda entry: (abs(entry[0] - instruction), entry[0]))[1] if matching else None


class Elf:
    def __init__(self, data: bytes):
        self.d = bytearray(data)
        if self.d[:4] != b"\x7fELF" or self.d[4] != 1 or self.d[5] != 1:
            raise ValueError("not a 32-bit little-endian ELF")
        (self.shoff,) = struct.unpack_from("<I", self.d, 0x20)
        self.shentsize, self.shnum, self.shstrndx = struct.unpack_from("<HHH", self.d, 0x2E)
        self.sh = [list(struct.unpack_from("<10I", self.d, self.shoff + i * self.shentsize))
                   for i in range(self.shnum)]

    # section header fields: name, type, flags, addr, offset, size, link, info, addralign, entsize
    def data(self, i: int) -> bytes:
        return bytes(self.d[self.sh[i][4]:self.sh[i][4] + self.sh[i][5]])

    def cstr(self, sec: int, off: int) -> str:
        blob = self.data(sec)
        return blob[off:blob.index(b"\0", off)].decode()

    def replace(self, i: int, blob: bytes) -> None:
        """Give section i new contents, appended at the end of the file (the old bytes stay, unreferenced)."""
        while len(self.d) % 4:
            self.d.append(0)
        self.sh[i][4], self.sh[i][5] = len(self.d), len(blob)
        self.d += blob

    def write(self) -> bytes:
        for i, h in enumerate(self.sh):
            struct.pack_into("<10I", self.d, self.shoff + i * self.shentsize, *h)
        return bytes(self.d)


def fix(obj: Path, asm: Path) -> tuple[int, list[str]]:
    elf = Elf(obj.read_bytes())
    symtab = next(i for i, h in enumerate(elf.sh) if h[1] == SHT_SYMTAB)
    strtab = elf.sh[symtab][6]
    syms = [list(struct.unpack_from("<IIIBBH", elf.data(symtab), k * 16)) for k in range(elf.sh[symtab][5] // 16)]
    names = [elf.cstr(strtab, s[0]) for s in syms]
    known = known_addresses()
    # function symbol of each code section (CodeWarrior emits one .text section per function)
    # (Elf32_Sym: name, value, size, info, other, shndx; type STT_FUNC = 2)
    funcs = {s[5]: names[k] for k, s in enumerate(syms) if s[3] & 0xF == 2 and s[1] == 0}
    loads = pooled_loads(asm, ORIG_ROM.read_bytes(), known) if ORIG_ROM.exists() else {}
    new_syms, strings = [], bytearray(elf.data(strtab))
    index = {n: k for k, n in enumerate(names) if n}
    fixed, warnings = 0, []
    for i, h in enumerate(elf.sh):
        if h[1] != SHT_REL or h[6] != symtab:
            continue
        rel = bytearray(elf.data(i))
        before = fixed
        for k in range(len(rel) // 8):
            off, info = struct.unpack_from("<II", rel, k * 8)
            if info & 0xFF != R_MIPS_LITERAL:
                continue
            func = funcs.get(h[7])
            base = func_address(func, known) if func else None
            symbol = syms[info >> 8]
            target = None
            if base is not None and 0 < symbol[5] < len(elf.sh):
                instruction = struct.unpack_from("<I", elf.data(h[7]), off)[0]
                addend = instruction & 0xFFFF
                if addend & 0x8000:
                    addend -= 0x10000
                offset = symbol[1] + addend
                value = elf.data(symbol[5])[offset:offset + 4] if offset >= 0 else b""
                if len(value) == 4:
                    target = literal_target(value, base + off, loads.get(func, []))
            if target is None:
                warnings.append(f"{obj}: literal at {func or '?'}+0x{off:X} has no verified original value; left as is")
                continue
            if target not in index:
                index[target] = len(syms) + len(new_syms)
                new_syms.append(struct.pack("<IIIBBH", len(strings), 0, 0, 0x10, 0, 0))  # GLOBAL NOTYPE UNDEF
                strings += target.encode() + b"\0"
            struct.pack_into("<I", rel, k * 8 + 4, (index[target] << 8) | R_MIPS_GPREL16)
            # The selected label already addresses symbol+addend's value. Carrying the old addend into
            # the new relocation would load a different word from the shared pool.
            struct.pack_into("<I", elf.d, elf.sh[h[7]][4] + off, instruction & 0xFFFF0000)
            fixed += 1
        if fixed > before:
            elf.replace(i, bytes(rel))
    if new_syms:
        elf.replace(symtab, elf.data(symtab) + b"".join(new_syms))
        elf.replace(strtab, bytes(strings))
    if fixed:
        obj.write_bytes(elf.write())
    return fixed, warnings


def main() -> None:
    if len(sys.argv) != 3:
        sys.exit(__doc__)
    fixed, warnings = fix(Path(sys.argv[1]), Path(sys.argv[2]))
    for w in warnings:
        print(f"warning: {w}", file=sys.stderr)


if __name__ == "__main__":
    main()
