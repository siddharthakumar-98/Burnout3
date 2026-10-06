#!/usr/bin/env python3
"""Draw the decompilation progress map (progress_map.svg) from objdiff's report.

Called by tools/progress.py, which also writes PROGRESS.md and progress.json. Plain Python 3, no dependencies.

The map has three parts: a headline with the share of game code in matching C, a treemap of the whole executable
(one tile per unit, sized by code bytes, grouped by category, shaded by how much of it is matching C), and one row per
category whose bar has a segment per unit in link order. Hand-written assembly that is meant to stay assembly (crt0 and
the VU microcode) is drawn in its own colour.
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
ASM = "#5ec8f2"       # boost blue: assembly that stays assembly
INTENTIONAL = {"runtime/crt0"}
MIN_SEG = 0.6  # narrowest bar segment, in pixels

# Row order (objdiff progress categories, see configure.py), with a title and a one-line description
ROWS = {
    "game": ("Game code", "Burnout 3 itself, built with CodeWarrior 3.0.3"),
    "rwa": ("RenderWare Audio", "Criterion's audio engine, EE side"),
    "rw": ("RenderWare 3.6", "Criterion's engine and the PS2 sky2 driver"),
    "sce": ("Sony libsce", "graphics, DMA, MPEG, kernel, CD/DVD, pads, memory card, network"),
    "runtime": ("Runtime", "crt0, Metrowerks C++ runtime, newlib and libgcc"),
    "ea": ("EA DirtySock", "online networking"),
    "lg": ("Logitech", "wheels and force feedback, USB headset, keyboard and mouse"),
}
SHORT = {"game": "game", "rwa": "RW Audio", "rw": "RenderWare", "sce": "libsce", "runtime": "runtime",
         "ea": "DirtySock", "lg": "Logitech"}


def unit_order() -> tuple[dict[str, int], int]:
    """VRAM start of every asm unit in b3.yaml, and the VU microcode's size in bytes."""
    starts, vu = {}, []
    for line in YAML.read_text().splitlines():
        m = re.match(r"\s*- \[0x([0-9A-F]+), (\w+), (\S+)\]", line)
        if m:
            rom = int(m.group(1), 16)
            if m.group(2) == "asm":
                starts[m.group(3)] = rom
            vu.append((rom, m.group(2) == "textbin"))
        m = re.match(r"\s*- \{ start: 0x([0-9A-F]+), type: asm, name: (\S+),", line)
        if m:
            starts[m.group(2)] = int(m.group(1), 16)
    vu_bytes = sum(b[0] - a[0] for a, b in zip(vu, vu[1:]) if a[1])
    return starts, vu_bytes


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
    starts, vu_bytes = unit_order()
    units = []
    for u in report["units"]:
        m = u.get("measures", {})
        total = int(m.get("total_code", 0))
        if not total:
            continue
        units.append({
            "name": u["name"],
            "cat": u["metadata"]["progress_categories"][0],
            "total": total,
            "matched": int(m.get("matched_code", 0)),
            "fn": int(m.get("total_functions", 0)),
            "fn_ok": int(m.get("matched_functions", 0)),
            "start": starts.get(u["name"], 1 << 30),
        })
    units.sort(key=lambda u: u["start"])
    cats = {c: [u for u in units if u["cat"] == c] for c in ROWS}
    matched = sum(u["matched"] for u in units)
    asm = sum(u["total"] for u in units if u["name"] in INTENTIONAL) + vu_bytes
    pending = sum(u["total"] for u in units) - matched - sum(u["total"] for u in units if u["name"] in INTENTIONAL)
    game = report_cat(report, "game")
    fn_ok, fn = report["measures"].get("matched_functions", 0), report["measures"].get("total_functions", 0)

    def color(u):
        return ASM if u["name"] in INTENTIONAL else shade(u["matched"] / u["total"])

    def tip(u):
        return (f"{escape(u['name'])}: {u['matched']:,} of {u['total']:,} B matching C, "
                f"{u['fn_ok']} of {u['fn']} functions")

    body = []
    # headline and legend
    body.append(text(PAD, 24, f"{fn_ok:,} of {fn:,} functions in matching C &#183; SLUS_210.50 (NTSC-U)", 12))
    body.append(text(PAD, 42, "Burnout 3: Takedown &#183; byte-matching decompilation", 10, DIM))
    for k, (label, fill, value) in enumerate((("matching C", MATCHED, matched), ("intentional asm", ASM, asm),
                                              ("pending asm", "url(#pending)", pending))):
        y = 9 + 17 * k
        body.append(f'<rect x="629" y="{y}" width="10" height="10" rx="2" fill="{fill}"/>')
        body.append(text(644, y + 9, f"{label} &#183; {value:,} B", 10))
    body.append(text(619, 38, pct(int(game["matched_code"]), int(game["total_code"])), 28, MATCHED, 800, "end"))
    body.append(text(619, 52, "of game code in matching C", 9, TEXT, anchor="end"))

    # treemap of the executable: one column per category, units squarified inside
    tx, ty, tw, th = PAD, 62, WIDTH - 2 * PAD, 170
    body.append(f'<rect x="{tx}" y="{ty}" width="{tw}" height="{th}" fill="{PANEL}"/>')
    grand = sum(u["total"] for u in units) + vu_bytes
    x = tx
    labels = []
    for cat in list(ROWS) + ["vu"]:
        members = cats.get(cat, [])
        size = vu_bytes if cat == "vu" else sum(u["total"] for u in members)
        cw = tw * size / grand
        if cat == "vu":
            body.append(f'<rect x="{x:.2f}" y="{ty}" width="{cw:.2f}" height="{th}" fill="{ASM}" '
                        f'stroke="{BG}" stroke-width="1"><title>VU microcode: 27 microprograms, {vu_bytes:,} B'
                        f'</title></rect>')
        else:
            order = sorted(members, key=lambda u: -u["total"])
            for u, (rx, ry, rw, rh) in zip(order, squarify([u["total"] for u in order], x, ty, cw, th)):
                body.append(f'<rect x="{rx:.2f}" y="{ry:.2f}" width="{rw:.2f}" height="{rh:.2f}" fill="{color(u)}" '
                            f'stroke="{BG}" stroke-width="0.6"><title>{tip(u)}</title></rect>')
        if cw > 46:
            labels.append((x + 4, ty + 13, "VU" if cat == "vu" else SHORT[cat]))
        x += cw
    for lx, ly, s in labels:
        body.append(f'<rect x="{lx - 2:.1f}" y="{ly - 10:.1f}" width="{len(s) * 6 + 6}" height="13" rx="2" '
                    f'fill="{BG}" fill-opacity="0.75"/>')
        body.append(text(lx + 1, ly, s.upper(), 9, BRIGHT, 700))
    cx, cy = tx + tw * 0.42, ty + th / 2
    body.append(f'<ellipse cx="{cx:.0f}" cy="{cy + 4:.0f}" rx="150" ry="34" fill="url(#halo)"/>')
    body.append(text(cx, cy + 6, pct(matched, grand), 34, MATCHED, 800, "middle"))
    body.append(text(cx, cy + 24, "of the whole executable in matching C", 10, BRIGHT, anchor="middle"))

    # one row per category
    y = ty + th + 26
    body.append(text(PAD, y, f"BY LIBRARY &#183; {len(units)} units in link order", 10, TEXT, 700))
    y += 12
    rows = [(c, *ROWS[c], cats[c]) for c in ROWS]
    for cat, title, desc, members in rows + [("vu", "VU microcode", "27 microprograms for the vector units; "
                                               "stays assembly, as in the original", [])]:
        y += 22
        body.append(f'<circle cx="{PAD + 4}" cy="{y - 4}" r="3" fill="{ASM if cat == "vu" else MATCHED}"/>')
        body.append(text(PAD + 14, y, f'<tspan font-weight="700" fill="{BRIGHT}">{escape(title)}</tspan> &#183; '
                                      f"{escape(desc)}", 11))
        if cat == "vu":
            body.append(text(WIDTH - PAD, y + 1, "asm", 15, ASM, 800, "end"))
            body.append(text(WIDTH - PAD, y + 16, f"{vu_bytes:,} B", 9, TEXT, anchor="end"))
            body.append(f'<rect x="{PAD + 14}" y="{y + 7}" width="{WIDTH - 2 * PAD - 120}" height="9" rx="1" '
                        f'fill="{ASM}"/>')
        else:
            m = report_cat(report, cat)
            total, ok = int(m.get("total_code", 0)), int(m.get("matched_code", 0))
            body.append(text(WIDTH - PAD, y + 1, pct(ok, total), 15, MATCHED, 800, "end"))
            body.append(text(WIDTH - PAD, y + 16, f"{m.get('matched_functions', 0):,}/{m.get('total_functions', 0):,}"
                                                  f" fn &#183; {total:,} B", 9, TEXT, anchor="end"))
            bx, bw = PAD + 14, WIDTH - 2 * PAD - 120
            gap = 2 if len(members) <= 40 else 1 if len(members) <= 120 else 0
            # every unit gets at least MIN_SEG pixels, the rest of the bar is shared by code size
            room = bw - gap * (len(members) - 1) - MIN_SEG * len(members)
            xx = bx
            for k, u in enumerate(members):
                sw = MIN_SEG + room * u["total"] / total
                fill = color(u)
                if gap == 0 and fill == PLATE and k % 2:
                    fill = "#33242a"  # alternate the plate so neighbouring units stay apart
                body.append(f'<rect x="{xx:.2f}" y="{y + 7}" width="{sw:.2f}" height="9" fill="{fill}">'
                            f"<title>{tip(u)}</title></rect>")
                xx += sw + gap
        y += 18
    height = y + 22
    body.append(text(PAD, height - 6, "Generated by Burnout3_decomp/tools/progress.py from objdiff's report", 8, DIM))

    head = (f'<?xml version="1.0" encoding="UTF-8"?>\n'
            f'<svg xmlns="http://www.w3.org/2000/svg" width="{WIDTH}" height="{height}" '
            f'viewBox="0 0 {WIDTH} {height}" role="img" '
            f'aria-label="Decompilation progress of SLUS_210.50: {fn_ok} of {fn} functions in matching C">\n'
            f"<title>Burnout 3: Takedown - decompilation progress</title>\n"
            f'<rect width="{WIDTH}" height="{height}" fill="{BG}"/>\n'
            f'<defs><linearGradient id="pending" x1="0" y1="0" x2="1" y2="0"><stop offset="0" stop-color="{PLATE}"/>'
            f'<stop offset="1" stop-color="{MATCHED}"/></linearGradient>'
            f'<radialGradient id="halo"><stop offset="0" stop-color="{BG}" stop-opacity="0.85"/>'
            f'<stop offset="0.6" stop-color="{BG}" stop-opacity="0.6"/>'
            f'<stop offset="1" stop-color="{BG}" stop-opacity="0"/></radialGradient></defs>\n')
    return head + "\n".join(body) + "\n</svg>\n"


def report_cat(report: dict, cat: str) -> dict:
    return next((c["measures"] for c in report["categories"] if c["id"] == cat), {})


def badge(report: dict) -> dict:
    """shields.io endpoint badge: share of game code in matching C."""
    game = report_cat(report, "game")
    return {"schemaVersion": 1, "label": "decompiled", "labelColor": "0d1117", "color": MATCHED[1:],
            "message": f"{pct(int(game.get('matched_code', 0)), int(game.get('total_code', 0)))} of game code"}


if __name__ == "__main__":
    print(render(json.loads((ROOT / "build/report.json").read_text())))
