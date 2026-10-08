#!/usr/bin/env python3
"""Propose Burnout 2 names for Burnout 3 game functions (D5).

Burnout 3 grew out of Burnout 2's code, and its units keep Burnout 2's functions in the same order. This pairs the
two by anchor and propagate: seeds (names we already adopted, strings and floats unique to one function on both
sides), then link order within a unit/file, vtable slots and direct calls, repeated until nothing new turns up.
Every row is keyed by a Burnout 3 address and says which evidence produced it. Nothing is written to
symbol_addrs.txt: the orchestrator applies names by hand.

Run on the host after configure.py has generated assembly/asm/ (plain Python 3, no container):

    python3 tools/bo2map.py --bo2 PATH              write build/bo2map.tsv and print a summary
    python3 tools/bo2map.py --bo2 PATH --unit U     rows of one unit plus the Burnout 2 sources to give the matcher
    python3 tools/bo2map.py --bo2 PATH --eval       hold out each name-seeded unit and check its seeds come back

PATH is a checkout of https://github.com/b3dllc/burnout2 kept outside the repo (spec:
docs/superpowers/specs/2026-10-08-bo2map-design.md).
"""

import argparse
import struct
import sys
from collections import defaultdict
from dataclasses import dataclass, field
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

import bo2src  # noqa: E402
from bo2src import Fn2, demangle  # noqa: E402

VTABLES = (0x4DDAA0, 0x4E0680)
LIT4 = (0x4E0680, 0x4E1400)
STRINGS = ((0x483F00, 0x4B1500), (0x4B1500, 0x4D3E00))  # .data, .rodata
PROPAGATE_RATIO = 0.5   # order/vtable/call pairs need min(size)/max(size) at least this
MEDIUM_RATIO = 0.75     # one kind of evidence is medium only with sizes within 25%
VT_MIN_SLOTS = 4        # vtable-shape seeds need this many aligned slots
VT_MARGIN = 2.0         # ... and a score this far ahead of the next-best class
KINDS_ORDER = ("name", "str", "const", "vtable", "order", "call")


@dataclass
class Fn3:
    addr: int
    size: int
    name: str
    unit: str
    calls: set = field(default_factory=set)
    strings: set = field(default_factory=set)
    floats: set = field(default_factory=set)


@dataclass
class Row:
    fn3: Fn3
    fn2: Fn2
    kinds: set
    confidence: str
    conflict: bool = False


def ratio(a: int, b: int) -> float:
    return min(a, b) / max(a, b) if a and b else 0.0


def confidence(kinds: set, s3: int, s2: int) -> str:
    if "name" in kinds or len(kinds) >= 2:
        return "high"
    return "medium" if ratio(s3, s2) >= MEDIUM_RATIO else "low"


def align(sizes3: list[int], sizes2: list[int]) -> list[tuple[int, int]]:
    """Needleman–Wunsch over two size sequences with free end gaps; pairs (i, j) of aligned positions.

    A match scores 1 + size ratio and is allowed only when the ratio is at least PROPAGATE_RATIO; an inner gap
    costs 0.3, so a function Burnout 3 added or dropped is skipped rather than forced into a pair."""
    n, m, gap = len(sizes3), len(sizes2), -0.3
    neg = float("-inf")
    score = [[0.0] * (m + 1) for _ in range(n + 1)]
    back = [[0] * (m + 1) for _ in range(n + 1)]   # 0 diag, 1 up (skip i), 2 left (skip j)
    for i in range(1, n + 1):
        back[i][0] = 1
    for j in range(1, m + 1):
        back[0][j] = 2
    for i in range(1, n + 1):
        for j in range(1, m + 1):
            r = ratio(sizes3[i - 1], sizes2[j - 1])
            diag = score[i - 1][j - 1] + 1 + r if r >= PROPAGATE_RATIO else neg
            up = score[i - 1][j] + (0 if j == m else gap)
            left = score[i][j - 1] + (0 if i == n else gap)
            best = max(diag, up, left)
            score[i][j] = best
            back[i][j] = 0 if best == diag else 1 if best == up else 2
    pairs, i, j = [], n, m
    while i > 0 and j > 0:
        if back[i][j] == 0:
            pairs.append((i - 1, j - 1))
            i, j = i - 1, j - 1
        elif back[i][j] == 1:
            i -= 1
        else:
            j -= 1
    return pairs[::-1]


class Matcher:
    def __init__(self, f3: list[Fn3], f2: list[Fn2], vt3: list[list[int]], vt2: dict[str, list[int]]):
        self.f3 = {f.addr: f for f in f3}
        self.f2 = {f.addr: f for f in f2}
        self.vt3, self.vt2 = vt3, vt2
        self.seq3 = sorted(f3, key=lambda f: f.addr)
        self.index3 = {f.addr: i for i, f in enumerate(self.seq3)}
        self.callers3: dict[int, set] = defaultdict(set)
        for f in f3:
            for c in f.calls:
                self.callers3[c].add(f.addr)
        self.files: dict[str, list[Fn2]] = defaultdict(list)
        for f in sorted(f2, key=lambda f: f.addr):
            self.files[f.file].append(f)
        self.by_name: dict[str, list[int]] = defaultdict(list)    # qualified name and bare method name
        for f in f2:
            self.by_name[f.name].append(f.addr)
            if "::" in f.name:
                self.by_name["::" + f.name.split("::")[-1]].append(f.addr)
        self.callees2 = {f.addr: self.resolve(f) for f in f2}
        self.callers2: dict[int, set] = defaultdict(set)
        for a, cs in self.callees2.items():
            for c in cs:
                self.callers2[c].add(a)
        self.props: dict[int, dict[int, set]] = defaultdict(lambda: defaultdict(set))

    def resolve(self, fn: Fn2) -> set:
        """Burnout 2 functions a body may call: a qualified name, or every function or method of a bare name."""
        out = set()
        for name in fn.callees:
            out.update(self.by_name.get(name, ()) if "::" in name
                       else self.by_name.get(name, []) + self.by_name.get("::" + name, []))
        return out

    def propose(self, a3: int, a2: int, kind: str) -> bool:
        kinds = self.props[a3][a2]
        if kind in kinds:
            return False
        kinds.add(kind)
        return True

    # seeds

    def seed(self) -> None:
        mangled = {f.mangled: f.addr for f in self.f2.values() if f.mangled}
        for f in self.f3.values():
            if f.name.startswith(("func_", "D_")):
                continue
            a2 = mangled.get(f.name)
            if a2 is None:
                cands = self.by_name.get(demangle(f.name), [])
                a2 = cands[0] if len(cands) == 1 else None
            if a2 is not None:
                self.propose(f.addr, a2, "name")
        for kind, attr in (("str", "strings"), ("const", "floats")):
            use3, use2 = defaultdict(set), defaultdict(set)
            for f in self.f3.values():
                for v in getattr(f, attr):
                    use3[v].add(f.addr)
            for f in self.f2.values():
                for v in getattr(f, attr):
                    use2[v].add(f.addr)
            for v, a3s in use3.items():
                if len(a3s) == 1 and len(use2.get(v, ())) == 1:
                    self.propose(next(iter(a3s)), next(iter(use2[v])), kind)
        # vtable shape: align each Burnout 3 vtable with every Burnout 2 class by slot sizes; seed from a clear winner
        for run in self.vt3:
            if len(run) < VT_MIN_SLOTS:
                continue
            scored = sorted(((self.vtable_fit(run, slots), cls) for cls, slots in self.vt2.items()
                             if len(slots) >= VT_MIN_SLOTS), reverse=True)
            if not scored:
                continue
            (best, pairs), cls = scored[0]
            second = scored[1][0][0] if len(scored) > 1 else 0.0
            if len(pairs) >= VT_MIN_SLOTS and best - second >= VT_MARGIN:
                for a3, a2 in pairs:
                    self.propose(a3, a2, "vtable")

    def vtable_fit(self, run: list[int], slots: list[int]) -> tuple[float, list[tuple[int, int]]]:
        s3 = [self.f3[a].size if a in self.f3 else 0 for a in run]
        s2 = [self.f2[a].size if a in self.f2 else 0 for a in slots]
        pairs = [(run[i], slots[j]) for i, j in align(s3, s2) if run[i] in self.f3 and slots[j] in self.f2]
        return sum(1 + ratio(self.f3[a].size, self.f2[b].size) for a, b in pairs), pairs

    # rows

    def rows(self) -> dict[int, Row]:
        """Current best pair per Burnout 3 function; disagreeing evidence becomes a low-confidence conflict."""
        by2: dict[int, set] = defaultdict(set)
        for a3, cands in self.props.items():
            for a2 in cands:
                by2[a2].add(a3)
        out = {}
        for a3, cands in self.props.items():
            named = [a2 for a2, k in cands.items() if "name" in k]
            choices = named or list(cands)
            a2 = max(choices, key=lambda a: (len(cands[a]), ratio(self.f3[a3].size, self.f2[a].size)))
            rivals = by2[a2] - {a3}
            rival_named = any("name" in self.props[x].get(a2, ()) for x in rivals)
            # a name seed outranks other evidence; otherwise two candidates either way is a conflict
            conflict = len(choices) > 1 or rival_named or (bool(rivals) and not named)
            kinds = set(cands[a2])
            conf = "low" if conflict else confidence(kinds, self.f3[a3].size, self.f2[a2].size)
            out[a3] = Row(self.f3[a3], self.f2[a2], kinds, conf, conflict)
        return out

    # propagation

    def extend_order(self, anchors: dict[int, Row]) -> bool:
        """Align each anchored Burnout 2 file with Burnout 3's functions in address order, between its anchors.

        Our unit cuts need not follow Burnout 2's files, so the window is Burnout 3's global address order,
        reaching past the outer anchors by about twice as many functions as the file has left on that side."""
        groups = defaultdict(list)
        for row in anchors.values():
            groups[row.fn2.file].append(row)
        changed = False
        for file, rows in groups.items():
            seq2 = self.files[file]
            idx2 = {f.addr: j for j, f in enumerate(seq2)}
            fixed, last_j = [], -1
            for i, j in sorted((self.index3[r.fn3.addr], idx2[r.fn2.addr]) for r in rows):
                if j > last_j:            # keep anchors that agree on order
                    fixed.append((i, j))
                    last_j = j
            (fi, fj), (li, lj) = fixed[0], fixed[-1]
            lo, hi = max(0, fi - 2 * fj - 2), min(len(self.seq3), li + 2 * (len(seq2) - lj) + 2)
            bounds = [(lo - 1, -1)] + fixed + [(hi, len(seq2))]
            for (i0, j0), (i1, j1) in zip(bounds, bounds[1:]):
                s3, s2 = self.seq3[i0 + 1:i1], seq2[j0 + 1:j1]
                if not s3 or not s2 or len(s3) > 4 * len(s2) + 8:
                    continue
                for i, j in align([f.size for f in s3], [f.size for f in s2]):
                    changed |= self.propose(s3[i].addr, s2[j].addr, "order")
        return changed

    def extend_vtables(self, anchors: dict[int, Row]) -> bool:
        changed = False
        for run in self.vt3:
            classes = None
            for k, a3 in enumerate(run):
                row = anchors.get(a3)
                if row is None:
                    continue
                fit = {c for c, slots in self.vt2.items() if k < len(slots) and slots[k] == row.fn2.addr}
                classes = fit if classes is None else classes & fit
            if not classes or len(classes) != 1:
                continue
            slots = self.vt2[next(iter(classes))]
            for k, a3 in enumerate(run[:len(slots)]):
                a2 = slots[k]
                if a3 in self.f3 and a2 in self.f2 and ratio(self.f3[a3].size, self.f2[a2].size) >= PROPAGATE_RATIO:
                    changed |= self.propose(a3, a2, "vtable")
        return changed

    def extend_calls(self, anchors: dict[int, Row]) -> bool:
        """Pair the callees, then the callers, of each anchor where exactly one candidate fits on each side."""
        changed = False
        for row in anchors.values():
            for c3, c2 in (({a for a in row.fn3.calls if a in self.f3}, self.callees2[row.fn2.addr]),
                           (self.callers3[row.fn3.addr], self.callers2[row.fn2.addr])):
                fits = {a3: {a2 for a2 in c2 if ratio(self.f3[a3].size, self.f2[a2].size) >= PROPAGATE_RATIO}
                        for a3 in c3}
                for a3, cands in fits.items():
                    if len(cands) != 1:
                        continue
                    a2 = next(iter(cands))
                    if sum(1 for c in fits.values() if a2 in c) == 1:
                        changed |= self.propose(a3, a2, "call")
        return changed

    def run(self, rounds: int = 20) -> dict[int, Row]:
        self.seed()
        for _ in range(rounds):
            rows = self.rows()
            anchors = {a: r for a, r in rows.items() if r.confidence in ("high", "medium")}
            changed = self.extend_order(anchors)
            changed |= self.extend_vtables(anchors)
            changed |= self.extend_calls(anchors)
            if not changed:
                break
        return self.rows()


def match(f3: list[Fn3], f2: list[Fn2], vt3: list[list[int]], vt2: dict[str, list[int]]) -> dict[int, Row]:
    return Matcher(f3, f2, vt3, vt2).run()


# Burnout 3 side

def load3() -> tuple[list[Fn3], list[list[int]]]:
    import configure
    import xref
    funcs = xref.load()
    rom = xref.ROM.read_bytes()

    def word(a: int) -> int:
        return struct.unpack_from("<I", rom, a - xref.BASE)[0]

    def cstring(a: int) -> str | None:
        end = rom.find(b"\0", a - xref.BASE, a - xref.BASE + 256)
        raw = rom[a - xref.BASE:end] if end >= 0 else b""
        return raw.decode() if len(raw) >= 4 and all(0x20 <= c < 0x7F for c in raw) else None

    starts = {f.addr for f in funcs}
    out = []
    for f in funcs:
        unit = str(f.file.relative_to(xref.ASM_DIR).with_suffix(""))
        if configure.CATEGORIES.get(unit.split("/")[0], ("",))[0] != "game":
            continue
        g = Fn3(f.addr, f.size, f.name, unit, set(f.calls))
        for r in f.refs:
            if LIT4[0] <= r < LIT4[1]:
                g.floats.add(struct.unpack("<f", struct.pack("<I", word(r)))[0])
            elif any(lo <= r < hi for lo, hi in STRINGS) and (s := cstring(r)):
                g.strings.add(s)
        out.append(g)
    heads = sorted({r for f in funcs for r in f.refs if VTABLES[0] <= r < VTABLES[1]}) + [VTABLES[1]]
    vt3 = []
    for h, nxt in zip(heads, heads[1:]):
        slots = [word(a) for a in range(h + 8, nxt, 4)]
        while slots and slots[-1] == 0:
            slots.pop()
        if slots and all(w == 0 or w in starts for w in slots):
            vt3.append(slots)
    return out, vt3


# CLI

def kinds_str(row: Row) -> str:
    return "+".join(k for k in KINDS_ORDER if k in row.kinds) + ("!conflict" if row.conflict else "")


def write_tsv(rows: dict[int, Row], path: Path) -> None:
    path.parent.mkdir(exist_ok=True)
    with open(path, "w") as f:
        f.write("bo3_addr\tbo3_name\tbo3_unit\tsize3\tbo2_name\tbo2_file\tsize2\tconfidence\tevidence\n")
        for a3 in sorted(rows):
            r = rows[a3]
            f.write(f"{a3:08X}\t{r.fn3.name}\t{r.fn3.unit}\t{r.fn3.size:#x}\t{r.fn2.name}\t{r.fn2.file}\t"
                    f"{r.fn2.size:#x}\t{r.confidence}\t{kinds_str(r)}\n")


def summary(rows: dict[int, Row], n3: int) -> None:
    by = defaultdict(int)
    for r in rows.values():
        by[r.confidence] += 1
    units = {r.fn3.unit for r in rows.values() if r.confidence != "low"}
    print(f"{len(rows)} of {n3} Burnout 3 game functions paired: high {by['high']}, medium {by['medium']}, "
          f"low {by['low']} ({sum(r.conflict for r in rows.values())} conflicts); {len(units)} units covered")


def show_unit(rows: dict[int, Row], f3: list[Fn3], unit: str, bo2: Path) -> None:
    funcs = [f for f in f3 if f.unit == unit]
    if not funcs:
        sys.exit(f"{unit}: no game functions (expected e.g. game/unit_00134570)")
    files, classes = set(), set()
    for f in funcs:
        r = rows.get(f.addr)
        if r is None:
            print(f"{f.addr:08X} {f.size:#6x} {f.name}")
            continue
        print(f"{f.addr:08X} {f.size:#6x} {f.name:32} {r.fn2.name:40} {r.fn2.size:#6x} {r.confidence:6} "
              f"{kinds_str(r)}")
        if r.confidence != "low":
            files.add(r.fn2.file)
            if "::" in r.fn2.name:
                classes.add(r.fn2.name.split("::")[0])
    print("\nBurnout 2 sources:")
    for p in sorted(files):
        print(f"  {bo2 / p}")
    for c in sorted(classes):
        if (bo2 / "include/types" / f"{c}.h").exists():
            print(f"  {bo2 / 'include/types' / f'{c}.h'}")


def evaluate(f3: list[Fn3], f2: list[Fn2], vt3, vt2) -> None:
    """Hold out the name seeds of one unit at a time and check the rest of the evidence finds them again."""
    base = match(f3, f2, vt3, vt2)
    seeded = defaultdict(list)
    for a3, r in base.items():
        if "name" in r.kinds:
            seeded[r.fn3.unit].append((a3, r.fn2.addr))
    total = found = wrong = 0
    for unit, pairs in sorted(seeded.items()):
        held = {a3 for a3, _ in pairs}
        trial = [Fn3(f.addr, f.size, f"func_{f.addr:08X}", f.unit, f.calls, f.strings, f.floats)
                 if f.addr in held else f for f in f3]
        rows = match(trial, f2, vt3, vt2)
        ok = sum(1 for a3, a2 in pairs if a3 in rows and rows[a3].fn2.addr == a2 and rows[a3].confidence != "low")
        bad = sum(1 for a3, a2 in pairs if a3 in rows and rows[a3].fn2.addr != a2 and rows[a3].confidence != "low")
        print(f"{unit:28} {ok}/{len(pairs)} recovered, {bad} wrong")
        total, found, wrong = total + len(pairs), found + ok, wrong + bad
    print(f"total {found}/{total} recovered, {wrong} wrong")


def main() -> None:
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("--bo2", type=Path, required=True, help="checkout of b3dllc/burnout2")
    ap.add_argument("--unit", help="show one unit, e.g. game/unit_00134570")
    ap.add_argument("--eval", action="store_true", help="hold-out check of the name seeds")
    ap.add_argument("--out", type=Path, default=Path(__file__).resolve().parent.parent / "build/bo2map.tsv")
    args = ap.parse_args()
    f2 = bo2src.load(args.bo2)
    vt2 = bo2src.vtables(args.bo2)
    f3, vt3 = load3()
    if args.eval:
        evaluate(f3, f2, vt3, vt2)
        return
    rows = match(f3, f2, vt3, vt2)
    if args.unit:
        unit = args.unit if "/" in args.unit else f"game/{args.unit}"
        show_unit(rows, f3, unit, args.bo2)
        return
    write_tsv(rows, args.out)
    summary(rows, len(f3))
    print(f"wrote {args.out}")


if __name__ == "__main__":
    main()
