#!/usr/bin/env python3
"""Assign .data, .rodata, .sdata, .sbss, .bss and .vtables to the .text units, giving every unit its own data slices.

Run after configure.py has generated assembly/asm/ (plain Python 3, no container needed):

    python3 tools/dataslice.py               summary
    python3 tools/dataslice.py --yaml        the data subsegment lines for b3.yaml

All five sections are laid out in link order, the same order as the units in .text (docs/layout.md). Each is cut
into consecutive slices, one per unit in .text order (a unit may get none), choosing the cut points that put the
most code references inside their own unit's slice. Only cuts at item starts (labels in the generated assembly)
are allowed. In .data and .rodata they are snapped to 16-byte boundaries, because the generated assembly aligns
items relative to the start of its file; .sdata needs 4-byte boundaries, and .sbss/.bss (only .space) none. An
item no code references goes with the next unit when it starts on an 8-byte boundary after the previous unit's
last referenced item, since every unit's data starts aligned; otherwise it stays with the previous unit. Data
slices that b3.yaml already carves for C units (d2/ to d5/) are kept exactly, and FORCED_CUTS (slice starts set by
hand) override the model.

.vtables (CodeWarrior's section for C++ vtables, `{RTTI*, 0, methods...}`) is in link order too. A vtable goes to
the unit that defines its own methods, the ones no other vtable lists (inherited ones belong to base classes elsewhere);
code that installs a vtable (constructors, destructors) counts as well. Its slices are named `<unit>_vt` and keep
the `.ctor` ordering bucket in b3.yaml. The word at 0x4E0610 (a pointer to `_start`, then padding to `.lit4`) is not a
vtable and stays a piece of its own, `vtables_end`.

Two pieces are not sliced: .lit4, the float literal pool the linker builds across all files (it fills the start
of the small-data area and is not in link order), and the COMMON block at the end of .bss (uninitialized globals
of the ee-gcc libraries newlib and lgkbm, which the linker allocates last).
"""

import argparse
import bisect
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "tools"))
import xref  # noqa: E402

YAML = ROOT / "assembly/splat/b3.yaml"
# stream -> (start, end, cut alignment)
STREAMS = {
    "data": (0x483F00, 0x4B1500, 16),
    "rodata": (0x4B1500, 0x4D3E00, 16),
    "sdata": (0x4E1400, 0x4E2680, 4),
    "sbss": (0x4E2680, 0x4E3000, 1),
    "bss": (0x4E3000, 0x1ECE340, 1),
    "vtables": (0x4DDAA0, 0x4E0610, 16),
}
VTABLES_END = 0x4E0610  # pointer to _start and padding, after the last vtable
LIT4 = 0x4E0680    # .lit4: linker-pooled float literals, 0x4E0680-0x4E1400 (padded to 0x80)
COMMON = 0x1ECE340  # COMMON symbols of the ee-gcc libraries, to the end of .bss
NOLOAD = ("sbss", "bss")
CARVED = ("d2/", "d3/", "d4/", "d5/")  # units carved out of the game units for C (C_UNITS in configure.py)
# Slice starts settled by hand where the references mislead: (stream, VRAM) -> (unit whose slice starts there,
# reason). Cuts the model makes for that unit around it are dropped.
FORCED_CUTS: dict[tuple[str, int], tuple[str, str]] = {
    ("data", 0x4873F0): ("runtime/libc", "newlib's impure_data (struct _reent, 0x4873F8) and its stdin/stdout/"
                         "stderr FILEs follow; no code references them, so snapping would hand them to libcdvd"),
    ("bss", 0x1D6E280): ("game/unit_00222C90", "the memory manager owns the arena 0x67D880-0x1D6D880 (carved up by "
                         "0x222650) and the heap object 0x1D6D880 (0xA00 bytes, built by its __sinit); the object is "
                         "only named by other units' code, which would hand everything after the arena to them"),
}
SUBSEG = re.compile(r"\s*- \[0x([0-9A-F]+), (\w+), (\S+)\]")
NOLOAD_SUBSEG = re.compile(r"\s*- \{ type: (sbss|bss), vram: 0x([0-9A-F]+), name: (\S+) \}")
VT_SUBSEG = re.compile(r"\s*- \{ start: 0x([0-9A-F]+), type: (data), name: (\S+_vt),")


def text_units() -> list[tuple[int, str]]:
    """(VRAM start, name) of every .text asm unit in b3.yaml, in order."""
    out = []
    for line in YAML.read_text().splitlines():
        m = SUBSEG.match(line)
        if m and m.group(2) == "asm":
            out.append((int(m.group(1), 16) + xref.BASE, m.group(3)))
    return out


def fixed_slices() -> dict[str, list[tuple[int, int, str]]]:
    """Data slices of C units already carved in b3.yaml: stream -> [(start, end, name)]."""
    text = YAML.read_text().splitlines()
    # (VRAM, type, name) of every subsegment; .sbss/.bss lines give a VRAM, the others a file offset
    lines = [(int(m.group(1), 16) + xref.BASE, m.group(2), m.group(3))
             for m in map(SUBSEG.match, text) if m]
    lines += [(int(m.group(1), 16) + xref.BASE, m.group(2), m.group(3)) for m in map(VT_SUBSEG.match, text) if m]
    lines += [(int(m.group(2), 16), m.group(1), m.group(3)) for m in map(NOLOAD_SUBSEG.match, text) if m]
    lines.sort()
    out = {s: [] for s in STREAMS}
    vt_lo, vt_hi, _ = STREAMS["vtables"]
    for k, (start, kind, name) in enumerate(lines):
        stream = "vtables" if kind == "data" and vt_lo <= start < vt_hi else kind
        if stream in STREAMS and name.startswith(CARVED):
            end = lines[k + 1][0] if k + 1 < len(lines) else STREAMS[stream][1]
            out[stream].append((start, end, name.removesuffix("_vt")))
    return out


def version_tags() -> set[int]:
    """Items no code references but that start a library's data: Sony version tags ("PsIIlibgraph2800") at the
    start of each library's .data, and RenderWare "@@(#)$Id: ...$" strings at the start of a source file's
    .rodata. The disassembler often leaves them inside the previous item."""
    rom = (ROOT / xref.ROM).read_bytes()
    lo, hi = STREAMS["data"][0], STREAMS["rodata"][1]
    tags = {lo + m.start() for m in re.finditer(rb"PsIIlib", rom[lo - xref.BASE:hi - xref.BASE])}
    rcsid = {(lo + m.start()) & ~7 for m in re.finditer(rb"@@\(#\)\$Id", rom[lo - xref.BASE:hi - xref.BASE])}
    return tags | rcsid


def labels(lo: int, hi: int) -> list[int]:
    """Start addresses of the data items in [lo, hi), from the generated assembly, plus version tags."""
    out = {a for a in version_tags() if lo <= a < hi}
    for path in (ROOT / "assembly/asm/data").rglob("*.s"):
        for line in path.read_text().splitlines():
            m = re.match(r"dlabel \w*?([0-9A-F]{8})$", line)
            if m and lo <= int(m.group(1), 16) < hi:
                out.add(int(m.group(1), 16))
    return sorted(out | {lo})


def assign(items: list[int], users: dict[int, set[int]], nunits: int) -> list[int]:
    """Monotone assignment of items to units maximizing references that land in their own unit."""
    NEG = float("-inf")
    best = [0.0] * nunits
    back = []
    for a in items:
        us = users.get(a, set())
        # prefix maximum: the item may stay in the same unit as the previous one or move forward
        pm, arg, choice = NEG, 0, [0] * nunits
        nb = [0.0] * nunits
        for u in range(nunits):
            if best[u] > pm:
                pm, arg = best[u], u
            choice[u] = arg
            nb[u] = pm + (1.0 if u in us else 0.0)
        back.append(choice)
        best = nb
    u = max(range(nunits), key=lambda k: (best[k], -k))
    out = [0] * len(items)
    for i in range(len(items) - 1, -1, -1):
        out[i] = u
        u = back[i][u]
    return out


def unit_index(units: list[tuple[int, str]]):
    """Function address -> rank among the units that own data. C units (d2/ to d5/) own only their carved slices,
    so their references count for the unit before them."""
    starts = [a for a, _ in units]
    owners = [k for k, (_, name) in enumerate(units) if not name.startswith(CARVED)]
    rank = {u: i for i, u in enumerate(owners)}
    return owners, lambda addr: rank[max(u for u in owners if u <= bisect.bisect_right(starts, addr) - 1)]


def references(stream: str, units, funcs) -> tuple[list[int], dict[int, set[int]], list[int]]:
    """Items of a stream, the units (ranks) referencing each item, and the owner rank list."""
    lo, hi, _ = STREAMS[stream]
    owners, unit_of = unit_index(units)
    starts = [a for a, _ in units]
    users: dict[int, set[int]] = {}
    for f in funcs:
        # code in .text only (.init code can't be placed in a unit), and not crt0, whose references are section
        # bounds (it clears .sbss/.bss from their start), not variables of its own
        if f.addr < 0x469E00 and units[bisect.bisect_right(starts, f.addr) - 1][1] != "runtime/crt0":
            for a in f.refs:
                if lo <= a < hi:
                    users.setdefault(a, set()).add(unit_of(f.addr))
    items = labels(lo, hi)
    if stream == "vtables":
        add_vtable_methods(items, users, unit_of)
    # a reference into the middle of an item counts for the item
    for a in list(users):
        if a not in items:
            k = bisect.bisect_right(items, a) - 1
            users.setdefault(items[k], set()).update(users.pop(a))
    return items, users, owners


def add_vtable_methods(items: list[int], users: dict[int, set[int]], unit_of) -> None:
    """Count each vtable's own methods (listed in no other vtable) as uses by the units defining them."""
    rom = (ROOT / xref.ROM).read_bytes()
    bounds = items + [STREAMS["vtables"][1]]
    words = {}
    for a, b in zip(bounds, bounds[1:]):
        words[a] = [int.from_bytes(rom[x - xref.BASE:x - xref.BASE + 4], "little") for x in range(a + 8, b, 4)]
    count: dict[int, int] = {}
    for ws in words.values():
        for w in ws:
            count[w] = count.get(w, 0) + 1
    for a, ws in words.items():
        own = [w for w in ws if 0x100270 <= w < 0x469E00 and count[w] == 1]
        users.setdefault(a, set()).update(unit_of(w) for w in own)


def slices(stream: str, units: list[tuple[int, str]], funcs) -> list[tuple[int, str]]:
    lo, hi, align = STREAMS[stream]
    items, users, owners = references(stream, units, funcs)
    owner = [owners[u] for u in assign(items, users, len(owners))]
    referenced = [i for i, a in enumerate(items) if a in users]
    # cut points: where the owner of referenced items changes, moved back over unreferenced items to a library's
    # version tag if there is one, else to the first 8-byte-aligned one after the previous unit's last referenced
    # item
    tags = version_tags()
    cuts = {items[0]: owner[referenced[0]] if referenced else 0}
    for p, q in zip(referenced, referenced[1:]):
        if owner[p] != owner[q]:
            cut = items[q]
            between = items[p + 1:q]
            for a in [a for a in between if a in tags] or [a for a in between if a % 8 == 0][:1]:
                cut = a
                break
            cuts[cut] = owner[q]
    # Snap each cut to the nearest item start before or after it that has the stream's alignment (the generated
    # assembly aligns items relative to the start of its own file) and lies between its neighbours, or drop it if
    # there is none. Of the two, the one that moves fewer referenced items across the cut wins, then the nearer.
    aligned = [a for a in items if a % align == 0]
    owner_at = {a: owner[i] for i, a in enumerate(items) if a in users}
    ordered = sorted(cuts)
    snapped = {}
    for k, c in enumerate(ordered):
        if c % align == 0 or k == 0:
            snapped[c] = cuts[c]
            continue
        lo_c = max(snapped) if snapped else items[0]
        hi_c = ordered[k + 1] if k + 1 < len(ordered) else hi
        cands = [a for a in aligned if lo_c < a < hi_c]
        cands = [x for x in (max((a for a in cands if a < c), default=None), min((a for a in cands if a > c),
                                                                                default=None)) if x is not None]
        moved = lambda a: sum(1 for x, u in owner_at.items()
                              if (a <= x < c and u != cuts[c]) or (c <= x < a and u == cuts[c]))
        if cands:
            snapped[min(cands, key=lambda a: (moved(a), abs(a - c), a))] = cuts[c]
    cuts = snapped
    rank = {name: k for k, (_, name) in enumerate(units)}
    for (st, at), (name, _) in FORCED_CUTS.items():
        if st != stream:
            continue
        if at not in items or name not in rank:
            sys.exit(f"dataslice: forced cut {stream} 0x{at:08X} {name}: no item starts there, or no such unit")
        r = rank[name]
        own = lambda c: cuts[c] if isinstance(cuts[c], int) else rank.get(cuts[c], -1)
        before = [c for c in sorted(cuts) if c < at]
        while before and own(before[-1]) >= r:  # earlier starts of this unit or of later ones
            del cuts[before.pop()]
        for c in sorted(c for c in cuts if c > at):
            if own(c) > r:
                break
            del cuts[c]  # later starts of this unit or of earlier ones, up to the next unit's
        cuts[at] = rank[name]
    for start, end, name in fixed_slices()[stream]:
        prev = max((c for c in cuts if c < start), default=items[0])
        after = cuts[max(c for c in cuts if c <= end)] if any(c <= end for c in cuts) else cuts[prev]
        cuts = {c: u for c, u in cuts.items() if not start <= c <= end}
        cuts[start] = name
        cuts[end] = after
    out = []
    for c in sorted(cuts):
        u = cuts[c]
        name = u if isinstance(u, str) else units[u][1]
        if out and out[-1][1] == name:
            continue
        out.append((c, name))
    return out


def own_share(stream: str, sl: list[tuple[int, str]], units, funcs) -> tuple[int, int]:
    """(references landing in their own unit's slice, all references) for a stream's slices."""
    lo, hi, _ = STREAMS[stream]
    starts = [a for a, _ in units]
    unit_name = lambda addr: units[bisect.bisect_right(starts, addr) - 1][1]
    cut = [a for a, _ in sl]
    own = total = 0
    for f in funcs:
        if f.addr >= 0x469E00 or unit_name(f.addr).startswith(CARVED):
            continue
        for a in f.refs:
            if lo <= a < hi:
                total += 1
                own += sl[bisect.bisect_right(cut, a) - 1][1] == unit_name(f.addr)
    return own, total


def yaml_line(stream: str, start: int, name: str) -> str:
    if stream == "vtables":
        name = name if name == "vtables_end" else f"{name}_vt"
        return f"      - {{ start: 0x{start - xref.BASE:06X}, type: data, name: {name}, linker_section_order: .ctor }}"
    if stream in NOLOAD:
        return f"      - {{ type: {stream}, vram: 0x{start:08X}, name: {name} }}"
    return f"      - [0x{start - xref.BASE:06X}, {stream}, {name}]"


def main() -> None:
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("--yaml", action="store_true", help="print the data subsegment lines for b3.yaml")
    args = p.parse_args()
    funcs = xref.load()
    units = text_units()
    result = {s: slices(s, units, funcs) for s in STREAMS}
    if args.yaml:
        for stream, sl in result.items():
            if stream == "vtables":
                continue
            if stream == "sdata":
                print(yaml_line("sdata", LIT4, "lit4"))
            for start, name in sl:
                print(yaml_line(stream, start, name))
        print(yaml_line("bss", COMMON, "common"))
        print("# .vtables: these lines replace the vtables block after the ctor line")
        for start, name in result["vtables"]:
            print(yaml_line("vtables", start, name))
        print(yaml_line("vtables", VTABLES_END, "vtables_end"))
        return
    for stream, sl in result.items():
        names = [n for _, n in sl]
        dup = len(names) - len(set(names))
        own, total = own_share(stream, sl, units, funcs)
        print(f".{stream}: {len(sl)} slices for {len(set(names))} units"
              + (f" ({dup} units have more than one slice)" if dup else "")
              + f"; {own} of {total} code references ({100 * own / max(total, 1):.1f}%) land in their own unit")


if __name__ == "__main__":
    main()
