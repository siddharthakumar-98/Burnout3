#!/usr/bin/env python3
"""Generate build.ninja for the Burnout 3 matching decomp.

Run inside the build container (tools/dock python3 configure.py), then `tools/dock ninja`.

Steps:
  1. orig/SLUS_210.50 (your copy, hash-checked) -> orig/SLUS_210.50.rom (raw load segment)
  2. splat splits the rom into assembly/asm/ and assembly/assets/ and writes the linker script into build/
  3. build.ninja assembles every asm unit, compiles every C unit in c_cpp/src/ with CodeWarrior, links
     (C objects replace their asm counterparts once they match), objcopies the load segment and
     rebuilds the ELF container, failing unless the result has the original SHA-1
"""

import argparse
import json
import re
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent
BASENAME = "SLUS_210.50"
ORIG_ELF = Path("orig") / BASENAME
ORIG_ROM = Path("orig") / f"{BASENAME}.rom"
SPLAT_YAML = Path("assembly/splat/b3.yaml")
ASM_DIR = Path("assembly/asm")
ASSET_DIR = Path("assembly/assets")
SRC_DIR = Path("c_cpp/src")
BUILD = Path("build")

CROSS = "mips-linux-gnu-"
AS_FLAGS = "-EL -march=r5900 -mabi=eabi -G 0 -no-pad-sections -I assembly/include"

# CodeWarrior for PS2 3.0.1 build 119 (decomp.me mwcps2-3.0.1b119-040914, 2004-09-14). Of the 19 3.0.x
# builds on decomp.me it is the best or equal best on every function decompiled so far, and the only one that
# matches func_0013AE70; 3.0.3, the D2 choice, misses it and the D4 loops. See docs/compiler.md.
MWCC = Path("compilers/3.0.1b119-040914/mwccps2.exe")
# -O4: ustrToUtf8 is the first function where -O3 differs (76.8% vs 99.9%); -O4,s is identical to -O4.
# -str readonly: the game's string literals sit in .rodata and are addressed with lui/addiu even when
#   short (the default puts them in .data, and short ones in .sdata via $gp).
# -Cpp_exceptions off: the binary has no .exceptix exception tables.
CFLAGS = "-O4 -str readonly -Cpp_exceptions off"

# C/C++ translation units, keyed by path under c_cpp/src/ (.c or .cpp) and assembly/asm/ (.s).
#   linked: link the C object instead of the asm. Set only once objdiff shows 100%; the SHA-1 check
#           then proves it in the full build.
#   data:   carved data pieces this unit owns (path under assembly/asm/, ending in .data, .rodata, .sdata,
#           .sbss or .bss), mapped to the C object's section that replaces them (e.g. a switch's jump table).
#           Float literals need no entry: they stay in the shared .lit4 pool (tools/litfix.py).
C_UNITS = {
    "d2/func_00131AA0": {"linked": True},
    "d2/func_00131CE0": {"linked": True},
    "d2/func_00136E00": {"linked": True},
    "d2/func_0013AE70": {"linked": True},
    "d2/func_0013B670": {"linked": False},  # original uses inline asm (pmaxw/pminw); C is a draft
    "d2/func_0013B740": {"linked": True},
    "d2/func_0013C910": {"linked": True},
    "d2/func_0014DD80": {"linked": True},
    "d2/func_0014E7E0": {"linked": True},
    "d2/func_0014EC30": {"linked": True, "data": {"data/d2/func_0014EC30.rodata": ".rodata"}},
    "d2/func_0028B700": {"linked": True},
    # D3: float literals from the linker's .lit4 pool, retargeted by tools/litfix.py
    "d3/func_002527F0": {"linked": True},
    "d3/func_003EA7E0": {"linked": True},
    # D4: whole game units, with their data slices
    "d4/fs": {"linked": True},  # its data (mode strings, device table) stays in assembly for now
    "d4/pool": {"linked": True},
    "d4/valuedb": {"linked": True, "data": {"data/d4/valuedb_vt.data": ".data"}},  # its vtable
    "d4/vdb": {"linked": True},  # its data (CRC table, .sbss words) stays in assembly for now
    "d4/ustrfmt": {"linked": True},
    "d4/ustring": {"linked": False},  # D4.3 step 2 in progress: 6 of 14 functions at 100% (docs/d4.md)
}

LD_SCRIPT_SPLAT = BUILD / f"{BASENAME}.ld"
LD_SCRIPT_FINAL = BUILD / f"{BASENAME}.final.ld"
LD_SCRIPTS = [
    LD_SCRIPT_FINAL,
    BUILD / "undefined_syms_auto.txt",
    BUILD / "undefined_funcs_auto.txt",
    Path("config/linker_extra.ld"),
]


def run(cmd: list[str]) -> None:
    print("+", " ".join(cmd))
    subprocess.run(cmd, cwd=ROOT, check=True)


def asm_obj(unit: str) -> Path:
    return BUILD / ASM_DIR / f"{unit}.o"


def c_src(unit: str) -> Path:
    cpp = SRC_DIR / f"{unit}.cpp"
    return cpp if (ROOT / cpp).exists() else SRC_DIR / f"{unit}.c"


def c_obj(unit: str) -> Path:
    return BUILD / SRC_DIR / f"{unit}.o"


# Progress category of each unit, by the first part of its path (see docs/layout.md for the map).
CATEGORIES = {
    "game": ("game", "Burnout 3 game code"),
    "d2": ("game", None),
    "d3": ("game", None),
    "d4": ("game", None),
    "sinit": ("game", None),
    "rw": ("rw", "RenderWare 3.6"),
    "rwa": ("rwa", "RenderWare Audio (EE side)"),
    "sce": ("sce", "Sony libsce"),
    "runtime": ("runtime", "Runtime: crt0, Metrowerks C++ runtime, newlib libc/libm, libgcc"),
    "ea": ("ea", "EA DirtySock"),
    "lg": ("lg", "Logitech device libraries"),
}


def category(unit: str) -> str:
    top = unit.split("/")[0]
    if top not in CATEGORIES:
        sys.exit(f"{unit}: no progress category for '{top}/' (add it to CATEGORIES)")
    return CATEGORIES[top][0]


def start_alignment(asm: Path) -> int:
    """Alignment implied by the unit's original start address (at most 16).

    GNU as gives .text, .data and .bss 16-byte alignment, but library objects (built with ee-gcc) start on
    8-byte boundaries, and small-data and .bss slices start wherever their unit's variables did; their asm
    objects get the smaller alignment so they land where they did."""
    with open(ROOT / asm) as f:
        for line in f:
            # code and data lines carry "/* <rom> <vram> <bytes> */", .bss lines "/* <vram> */"
            m = re.match(r"\s*/\* (?:[0-9A-F]+ )?([0-9A-F]{8}) ", line)
            if m:
                vram = int(m.group(1), 16)
                return next(a for a in (16, 8, 4, 2, 1) if vram % a == 0)
    return 16


def write_final_ld_script() -> None:
    """Copy splat's linker script, leaving out empty sections and pointing linked C units at their objects.

    splat lists every object once per section kind (.text, .data, .rodata, .bss), but an asm unit's code object
    only has code, and each data slice object only its own section. The empty sections are left out: an empty
    .bss is still 16-byte aligned and would move the slices after it. A linked C unit's object replaces the asm
    object's .text line, and its data sections replace the data slices listed under "data" in C_UNITS, so the
    asm object is not pulled into the link at all."""
    script = (ROOT / LD_SCRIPT_SPLAT).read_text()
    slice_obj = re.compile(r"\.(data|rodata|sdata|sbss|bss|ctor)\.o$")
    entry = re.compile(r"\s+(\S+\.o)\((.*)\);$")
    kept = []
    for line in script.splitlines():
        m = entry.match(line)
        if m and m.group(2) != ".text" and not slice_obj.search(m.group(1)):
            continue  # empty data or .bss section of a code object
        kept.append(line)
    script = "\n".join(kept) + "\n"
    for unit, cfg in C_UNITS.items():
        if not cfg["linked"]:
            continue
        old = f"{asm_obj(unit).as_posix()}(.text)"
        if old not in script:
            sys.exit(f"{unit}: {old} not found in {LD_SCRIPT_SPLAT}; is it carved out in {SPLAT_YAML}?")
        script = script.replace(old, f"{c_obj(unit).as_posix()}(.text)")
        for piece, section in cfg.get("data", {}).items():
            old = re.compile(rf"{re.escape((BUILD / ASM_DIR / piece).as_posix())}\.o\([^)]*\)")
            if not old.search(script):
                sys.exit(f"{unit}: data piece {piece} not found in {LD_SCRIPT_SPLAT}")
            script = old.sub(f"{c_obj(unit).as_posix()}({section})", script)
    (ROOT / LD_SCRIPT_FINAL).write_text(script)


def write_ninja(asm_files: list[Path]) -> None:
    elf = BUILD / f"{BASENAME}.elf"
    rom = BUILD / f"{BASENAME}.rom"
    out = BUILD / BASENAME

    lines = [
        "# Generated by configure.py. Do not edit.",
        "ninja_required_version = 1.10",
        "",
        "rule as",
        f"  command = {CROSS}as {AS_FLAGS} -o $out $in",
        "  description = AS $in",
        "",
        "rule as_aligned",
        f"  command = {CROSS}as {AS_FLAGS} -o $out.tmp $in && {CROSS}objcopy "
        + " ".join(f"--set-section-alignment {sec}=$align"
                   for sec in (".text", ".data", ".rodata", ".sdata", ".sbss", ".bss"))
        + " $out.tmp $out && rm $out.tmp",
        "  description = AS $in (align $align)",
        "",
        # litfix points the object's float literals at the original's pooled .lit4 entries ($asm is the unit's
        # original assembly)
        "rule cc",
        f"  command = MWCIncludes=c_cpp/include wibo {MWCC} {CFLAGS} -c $in -o $out "
        "&& python3 tools/litfix.py $out $asm",
        "  description = CC $in",
        "",
        "rule ld",
        f"  command = {CROSS}ld -EL {' '.join(f'-T {s}' for s in LD_SCRIPTS)} "
        f"-Map {BUILD}/{BASENAME}.map --no-check-sections -o $out",
        "  description = LD $out",
        "",
        "rule objcopy",
        f"  command = {CROSS}objcopy -O binary -j .main $in $out",
        "  description = OBJCOPY $out",
        "",
        "rule elf",
        "  command = python3 tools/elf.py rebuild $in $out --check",
        "  description = ELF $out",
        "",
    ]

    objs = []
    for s in asm_files:
        o = BUILD / s.with_suffix(".o")
        objs.append(o)
        align = start_alignment(s)
        if align == 16:
            lines.append(f"build {o}: as {s}")
        else:
            lines += [f"build {o}: as_aligned {s}", f"  align = {align}"]
    for unit in C_UNITS:
        o = c_obj(unit)
        objs.append(o)
        lines += [f"build {o}: cc {c_src(unit)} | tools/litfix.py", f"  asm = {ASM_DIR / unit}.s"]
    lines += [
        "",
        f"build {elf}: ld | {' '.join(str(o) for o in objs)} {' '.join(str(s) for s in LD_SCRIPTS)}",
        f"build {rom}: objcopy {elf}",
        f"build {out}: elf {rom}",
        "",
        f"default {out}",
        "",
    ]
    (ROOT / "build.ninja").write_text("\n".join(lines))


def write_objdiff(asm_files: list[Path]) -> None:
    """objdiff units: target objects come from splat asm; C units add a base object to diff against."""
    units = []
    for s in asm_files:
        if s.relative_to(ASM_DIR).parts[0] == "data":
            continue
        unit = s.relative_to(ASM_DIR).with_suffix("").as_posix()
        entry = {
            "name": unit,
            "target_path": (BUILD / s.with_suffix(".o")).as_posix(),
            "metadata": {"progress_categories": [category(unit)]},
        }
        if unit in C_UNITS:
            entry["base_path"] = c_obj(unit).as_posix()
            entry["metadata"]["source_path"] = c_src(unit).as_posix()
        units.append(entry)
    config = {
        "min_version": "2.0.0",
        "custom_make": "ninja",
        "build_target": False,
        "build_base": True,
        "watch_patterns": ["*.c", "*.cpp", "*.h", "*.hpp", "*.s", "*.inc"],
        "progress_categories": [{"id": cid, "name": name} for cid, name in CATEGORIES.values() if name],
        "units": units,
    }
    (ROOT / "objdiff.json").write_text(json.dumps(config, indent=2) + "\n")


def main() -> None:
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("--no-split", action="store_true", help="reuse existing assembly/asm/ instead of re-running splat")
    args = p.parse_args()

    if not (ROOT / ORIG_ELF).exists():
        sys.exit(f"missing {ORIG_ELF}: extract SLUS_210.50 from your disc first (see README.md)")
    if not (ROOT / ORIG_ROM).exists():
        run([sys.executable, "tools/elf.py", "extract", str(ORIG_ELF), str(ORIG_ROM)])
    if C_UNITS and not (ROOT / MWCC).exists():
        sys.exit(f"missing {MWCC}: C units need the CodeWarrior compiler (see README.md)")

    if not args.no_split:
        # splat neither deletes files of units that were renamed nor rewrites existing binary pieces, so the
        # generated folders start empty on every split.
        for generated in (ASM_DIR, ASSET_DIR):
            shutil.rmtree(ROOT / generated, ignore_errors=True)
        run([sys.executable, "-m", "splat", "split", str(SPLAT_YAML)])

    asm_files = sorted(p.relative_to(ROOT) for p in (ROOT / ASM_DIR).rglob("*.s"))
    if not asm_files:
        sys.exit(f"no asm files found in {ASM_DIR}; run without --no-split")
    for unit in C_UNITS:
        if not (ROOT / c_src(unit)).exists():
            sys.exit(f"C unit {unit}: missing {c_src(unit)}")
    write_final_ld_script()
    write_ninja(asm_files)
    write_objdiff(asm_files)
    print(f"wrote build.ninja and objdiff.json ({len(asm_files)} asm files, {len(C_UNITS)} C units)")


if __name__ == "__main__":
    main()
