#!/usr/bin/env python3
"""Draw the decompilation progress map (progress_map.svg) from objdiff's report.

Called by tools/progress.py, which also writes PROGRESS.md and progress.json. Plain Python 3, no dependencies.

Only the objdiff `game` category contributes to the headline, treemap and link-order bar. Vendor libraries,
runtime and VU microcode remain in the matching build but are outside the game-source completion metric.
"""

import json
import re
from html import escape
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
YAML = ROOT / "assembly/splat/b3.yaml"

WIDTH, PAD = 800, 10
FONT = "ui-sans-serif, -apple-system, 'Segoe UI', Roboto, Helvetica, Arial, sans-serif"
BG, PANEL, TEXT, BRIGHT, DIM = "#0d1117", "#161b22", "#a6adc8", "#e6edf3", "#6e7681"
MATCHED = "#ff4b3e"   # takedown red
PLATE = "#2b1d22"     # pending code, no C yet
GAME_CATEGORY = "game"
MIN_SEG = 0.6  # narrowest bar segment, in pixels


def unit_order() -> dict[str, int]:
    """ROM start of every asm unit in b3.yaml, for the link-order bar."""
    starts = {}
    for line in YAML.read_text().splitlines():
        m = re.match(r"\s*- \[0x([0-9A-F]+), (\w+), (\S+)\]", line)
        if m:
            rom = int(m.group(1), 16)
            if m.group(2) == "asm":
                starts[m.group(3)] = rom
        m = re.match(r"\s*- \{ start: 0x([0-9A-F]+), type: asm, name: (\S+),", line)
        if m:
            starts[m.group(2)] = int(m.group(1), 16)
    return starts


def shade(frac: float) -> str:
    """Pending plate to matched red, by the share of a unit's code that is matching C."""
    if frac >= 1.0:
        return MATCHED
    frac = 0.15 + 0.6 * frac if frac > 0 else 0.0
    a, b = int(PLATE[1:], 16), int(MATCHED[1:], 16)
    mix = [round(((a >> s) & 255) * (1 - frac) + ((b >> s) & 255) * frac) for s in (16, 8, 0)]
    return "#" + "".join(f"{c:02x}" for c in mix)


def squarify(sizes: list[float], x: float, y: float, w: float, h: float) -> list[tuple[float, float, float, float]]:
    """Squarified treemap (Bruls et al.) of sizes, which must be sorted largest first."""
    total = sum(sizes)
    if not sizes or total <= 0:
        return []
    areas = [s * w * h / total for s in sizes]
    out, row = [], []

    def worst(r: list[float], side: float) -> float:
        s = sum(r)
        return max(max(side * side * a / (s * s), s * s / (side * side * a)) for a in r)

    def lay(r: list[float], x, y, w, h):
        s = sum(r)
        if w >= h:  # column on the left
            cw = s / h
            yy = y
            for a in r:
                out.append((x, yy, cw, a / cw))
                yy += a / cw
            return x + cw, y, w - cw, h
        rh = s / w
        xx = x
        for a in r:
            out.append((xx, y, a / rh, rh))
            xx += a / rh
        return x, y + rh, w, h - rh

    for a in areas:
        side = min(w, h)
        if not row or worst(row + [a], side) <= worst(row, side):
            row.append(a)
        else:
            x, y, w, h = lay(row, x, y, w, h)
            row = [a]
    if row:
        lay(row, x, y, w, h)
    return out


def text(x, y, s, size=10, fill=TEXT, weight=None, anchor=None, extra="") -> str:
    attrs = f'x="{x:.1f}" y="{y:.1f}" font-family="{FONT}" font-size="{size}" fill="{fill}"'
    if weight:
        attrs += f' font-weight="{weight}"'
    if anchor:
        attrs += f' text-anchor="{anchor}"'
    return f"<text {attrs}{extra}>{s}</text>"


def pct(part: int, whole: int) -> str:
    return f"{100 * part / whole:.2f}%" if whole else "-"


def render(report: dict) -> str:
    starts = unit_order()
    units = []
    for u in report["units"]:
        if GAME_CATEGORY not in u.get("metadata", {}).get("progress_categories", []):
            continue
        m = u.get("measures", {})
        total = int(m.get("total_code", 0))
        if not total:
            continue
        units.append({
            "name": u["name"],
            "total": total,
            "matched": int(m.get("matched_code", 0)),
            "fn": int(m.get("total_functions", 0)),
            "fn_ok": int(m.get("matched_functions", 0)),
            "named": sum(1 for f in u.get("functions", []) if not AUTO_NAME.match(f["name"])),
            "start": starts.get(u["name"], 1 << 30),
        })
    units.sort(key=lambda u: u["start"])
    game = report_cat(report, GAME_CATEGORY)
    total, matched = int(game.get("total_code", 0)), int(game.get("matched_code", 0))
    pending = total - matched
    fn_ok, fn = int(game.get("matched_functions", 0)), int(game.get("total_functions", 0))
    game_named = named(report).get(GAME_CATEGORY, (0, 0))[0]

    def color(u):
        return shade(u["matched"] / u["total"])

    def tip(u):
        return (f"{escape(u['name'])}: {u['matched']:,} of {u['total']:,} B matching C/C++, "
                f"{u['fn_ok']} of {u['fn']} functions, {u['named']} named")

    body = []
    body.append(text(PAD, 24, f"{fn_ok:,} of {fn:,} game functions in matching C/C++", 13, BRIGHT, 700))
    body.append(text(PAD, 44, f"SLUS_210.50 (NTSC-U) &#183; {pct(fn_ok, fn)} of functions &#183; "
                              f"{game_named:,} game functions named", 10, TEXT))
    body.append(text(WIDTH - PAD, 37, pct(matched, total), 30, MATCHED, 800, "end"))
    body.append(text(WIDTH - PAD, 54, "of game code bytes matched", 10, TEXT, anchor="end"))
    for x, label, fill, value in ((PAD, "matching C/C++", MATCHED, matched),
                                  (270, "pending game code", PLATE, pending)):
        body.append(f'<rect x="{x}" y="65" width="10" height="10" rx="2" fill="{fill}"/>')
        body.append(text(x + 16, 74, f"{label} &#183; {value:,} B", 10))

    # One tile per game unit; libraries and microcode do not occupy the map.
    tx, ty, tw, th = PAD, 88, WIDTH - 2 * PAD, 200
    body.append(f'<rect x="{tx}" y="{ty}" width="{tw}" height="{th}" fill="{PANEL}"/>')
    order = sorted(units, key=lambda u: -u["total"])
    for u, (rx, ry, rw, rh) in zip(order, squarify([u["total"] for u in order], tx, ty, tw, th)):
        body.append(f'<rect x="{rx:.2f}" y="{ry:.2f}" width="{rw:.2f}" height="{rh:.2f}" fill="{color(u)}" '
                    f'stroke="{BG}" stroke-width="0.6"><title>{tip(u)}</title></rect>')
    body.append(text(PAD, 314, f"GAME SOURCE &#183; {len(units):,} units in link order", 10, TEXT, 700))
    body.append(text(PAD, 334, "Burnout 3: Takedown &#183; CodeWarrior 3.0.1 b119", 11, BRIGHT))
    body.append(text(WIDTH - PAD, 334, f"{matched:,} / {total:,} B matched", 10, TEXT, anchor="end"))
    min_seg = min(MIN_SEG, tw / len(units)) if units else 0
    room, xx = tw - min_seg * len(units), tx
    unit_total = sum(u["total"] for u in units)
    for k, u in enumerate(units):
        sw = min_seg + room * u["total"] / unit_total
        fill = color(u)
        if fill == PLATE and k % 2:
            fill = "#33242a"
        body.append(f'<rect x="{xx:.2f}" y="345" width="{sw:.2f}" height="9" fill="{fill}">'
                    f"<title>{tip(u)}</title></rect>")
        xx += sw
    body.append(text(PAD, 379, "Game code only; vendor libraries, runtime and VU microcode are outside this metric.", 9, DIM))
    height = 406
    body.append(text(PAD, height - 6, "Generated by Burnout3_decomp/tools/progress.py from objdiff's report", 8, DIM))

    head = (f'<?xml version="1.0" encoding="UTF-8"?>\n'
            f'<svg xmlns="http://www.w3.org/2000/svg" width="{WIDTH}" height="{height}" '
            f'viewBox="0 0 {WIDTH} {height}" role="img" '
            f'aria-label="PS2 game-source decompilation of SLUS_210.50: {fn_ok} of {fn} functions in matching C/C++">\n'
            f"<title>Burnout 3: Takedown - PS2 game-source decompilation progress</title>\n"
            f'<rect width="{WIDTH}" height="{height}" fill="{BG}"/>\n')
    return head + "\n".join(body) + "\n</svg>\n"


AUTO_NAME = re.compile(r"func_[0-9A-F]{8}$")


def named(report: dict) -> dict[str, tuple[int, int]]:
    """Functions with a real name (not func_XXXXXXXX), as (named, total) per category and under "all". Names come
    from config/symbol_addrs.txt: identified library functions and decompiled game functions."""
    out: dict[str, list[int]] = {}
    for u in report["units"]:
        cat = u.get("metadata", {}).get("progress_categories", ["?"])[0]
        for f in u.get("functions", []):
            for key in (cat, "all"):
                n = out.setdefault(key, [0, 0])
                n[0] += not AUTO_NAME.match(f["name"])
                n[1] += 1
    return {k: (v[0], v[1]) for k, v in out.items()}


def report_cat(report: dict, cat: str) -> dict:
    return next((c["measures"] for c in report["categories"] if c["id"] == cat), {})


def badge(report: dict) -> dict:
    """shields.io endpoint badge: share of game code in matching C."""
    game = report_cat(report, GAME_CATEGORY)
    return {"schemaVersion": 1, "label": "PS2 game decomp", "labelColor": "0d1117", "color": MATCHED[1:],
            "message": f"{pct(int(game.get('matched_code', 0)), int(game.get('total_code', 0)))} of game code"}


if __name__ == "__main__":
    print(render(json.loads((ROOT / "build/report.json").read_text())))
