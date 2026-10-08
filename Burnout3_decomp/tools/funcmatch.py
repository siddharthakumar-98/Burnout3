#!/usr/bin/env python3
"""Compare one C/C++ function against the original under one or more compiler flag sets.

Run inside the build container after configure.py has generated assembly/asm/:

    tools/dock python3 tools/funcmatch.py func_0013C930 c_cpp/src/d2/foo.c -f=-O3 -f="-O4 -inline auto"
    tools/dock python3 tools/funcmatch.py --all c_cpp/src/d4/pool.c     one line per function the file defines
    tools/dock python3 tools/funcmatch.py func_0013C930 --variants build/var/func_0013C930
                                                     one line per source in the folder (each a whole compilable file)

The function's assembly is pulled out of assembly/asm/ into its own object (the target), the source is
compiled with CodeWarrior once per flag set (the base), and objdiff reports how well they match. Float literals in
the base are pointed at the original's pooled .lit4 entries first (tools/litfix.py). On a mismatch the two
disassemblies are printed side by side. Nothing outside build/funcmatch/ is touched.
"""

import argparse
import json
import os
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT))
sys.path.insert(0, str(ROOT / "tools"))
from configure import AS_FLAGS, ASM_DIR, CFLAGS, CROSS, MWCC  # noqa: E402
import litfix  # noqa: E402

WORK = Path("build/funcmatch")
ASM_HEADER = '.include "macro.inc"\n\n.set noat\n.set noreorder\n\n.section .text, "ax"\n\n'


def sh(cmd: str, **kw) -> subprocess.CompletedProcess:
    return subprocess.run(cmd, shell=True, cwd=ROOT, capture_output=True, text=True, **kw)


def extract_function(name: str) -> tuple[Path, str]:
    """The unit's assembly file that defines the function, and the function's assembly."""
    for path in sorted((ROOT / ASM_DIR).rglob("*.s")):
        text = path.read_text()
        m = re.search(rf"^glabel {re.escape(name)}\n.*?^endlabel {re.escape(name)}\n", text, re.S | re.M)
        if m:
            return path, m.group(0)
    sys.exit(f"{name}: not found in {ASM_DIR}; run configure.py first")


def build_target(name: str) -> Path:
    s = WORK / f"{name}.target.s"
    o = WORK / f"{name}.target.o"
    (ROOT / s).write_text(ASM_HEADER + extract_function(name)[1])
    r = sh(f"{CROSS}as {AS_FLAGS} -o {o} {s}")
    if r.returncode:
        sys.exit(f"assembling {s} failed:\n{r.stderr}")
    return o


def compile_base(compiler: Path, src: Path, flags: str, tag: int, asm: Path) -> tuple[Path | None, str]:
    o = WORK / f"{src.stem}.{tag}.o"
    env = dict(os.environ, MWCIncludes="c_cpp/include")
    r = sh(f"wibo {compiler} {flags} -c {src} -o {o}", env=env)
    if r.returncode or not (ROOT / o).exists():
        return None, r.stdout + r.stderr
    litfix.fix(ROOT / o, asm)
    return o, ""


def match_percent(target: Path, base: Path, name: str) -> float | None:
    r = sh(f"objdiff-cli diff -1 {target} -2 {base} -o - {name}")
    if r.returncode:
        return None
    data = json.loads(r.stdout)
    for sym in data.get("right", {}).get("symbols", []):
        if sym.get("name") == name:
            return sym.get("match_percent")
    return None


def disasm(obj: Path, name: str) -> list[str]:
    r = sh(f"{CROSS}objdump -dr --no-show-raw-insn -M no-aliases {obj} --disassemble={name}")
    out = []
    for line in r.stdout.splitlines():
        m = re.match(r"\s*[0-9a-f]+:\s+(.*)", line)
        if m:
            out.append(re.sub(r"\s+", " ", m.group(1)).strip())
        elif "R_MIPS" in line:
            out[-1] += "  [" + line.split()[-1] + "]"
    return out


def defined_functions(obj: Path) -> list[str]:
    """Global functions an object defines, in address order of their sections."""
    r = sh(f"{CROSS}nm --defined-only {obj}")
    return [line.split()[-1] for line in r.stdout.splitlines() if line.split()[1:2] == ["T"]]


def match_all(compiler: Path, src: Path, flags: str) -> int:
    """Compile once, compare every function; prints `pct name`. Exit 0 only if all are 100%."""
    o = WORK / f"{src.stem}.all.o"
    env = dict(os.environ, MWCIncludes="c_cpp/include")
    r = sh(f"wibo {compiler} {flags} -c {src} -o {o}", env=env)
    if r.returncode or not (ROOT / o).exists():
        print("compile failed\n" + r.stdout + r.stderr)
        return 2
    names = defined_functions(o)
    done = 0
    for name in names:
        try:
            target = build_target(name)
            litfix.fix(ROOT / o, extract_function(name)[0])
        except SystemExit:
            print(f"{'--':>7}  {name} (not in the generated assembly)")
            continue
        pct = match_percent(target, o, name)
        done += pct == 100.0
        print(f"{'missing' if pct is None else f'{pct:6.2f}%':>7}  {name}")
    print(f"{done}/{len(names)} at 100%")
    return 0 if done == len(names) else 1


def match_variants(compiler: Path, name: str, folder: Path, flags: str) -> int:
    """Compile every .c/.cpp in folder and compare one function; prints `pct file`. Exit 0 if any is 100%."""
    target = build_target(name)
    asm = extract_function(name)[0]
    sources = sorted(f for f in (ROOT / folder).iterdir() if f.suffix in (".c", ".cpp"))
    if not sources:
        sys.exit(f"{folder}: no .c/.cpp files")
    best = 0.0
    for i, src in enumerate(sources):
        base, err = compile_base(compiler, src.relative_to(ROOT), flags, 100 + i, asm)
        if base is None:
            lines = err.splitlines()  # MW: "#   Error: <caret>" then "#   <message>"
            msg = next((lines[n + 1].strip("# ") for n, l in enumerate(lines[:-1]) if "Error:" in l), "")
            print(f"{'failed':>7}  {src.name}: {msg}")
            continue
        pct = match_percent(target, base, name)
        best = max(best, pct or 0.0)
        print(f"{'missing' if pct is None else f'{pct:6.2f}%':>7}  {src.name}")
    return 0 if best == 100.0 else 1


def main() -> None:
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("function", nargs="?", help="symbol name in the generated assembly, e.g. func_0013C930")
    p.add_argument("source", type=Path, nargs="?", help="C or C++ file defining that symbol")
    p.add_argument("--variants", type=Path, metavar="DIR",
                   help="instead of one source, compare the function in every .c/.cpp in DIR (first flag set only)")
    p.add_argument("--all", action="store_true",
                   help="compare every function the source defines (first flag set only), one line each, no diffs")
    p.add_argument("-f", "--flags", action="append", help=f"compiler flags to try, written -f=FLAGS (default: {CFLAGS!r})")
    p.add_argument("-q", "--quiet", action="store_true", help="don't print disassembly on mismatch")
    p.add_argument("-c", "--compiler", type=Path, default=MWCC,
                   help=f"compiler executable, or a folder under compilers/ (default: {MWCC})")
    args = p.parse_args()

    compiler = args.compiler
    if (ROOT / compiler).is_dir():
        compiler = compiler / "mwccps2.exe"
    (ROOT / WORK).mkdir(parents=True, exist_ok=True)
    if args.variants:
        if not args.function:
            p.error("--variants needs a function")
        sys.exit(match_variants(compiler, args.function, args.variants, (args.flags or [CFLAGS])[0]))
    if args.all and args.source is None and args.function:
        args.function, args.source = None, Path(args.function)  # --all SOURCE: the one positional is the source
    if args.source is None:
        p.error("give a source file")
    if args.all:
        sys.exit(match_all(compiler, args.source, (args.flags or [CFLAGS])[0]))
    if not args.function:
        p.error("give a function, or --all")
    target = build_target(args.function)
    asm = extract_function(args.function)[0]
    best = 0.0
    for i, flags in enumerate(args.flags or [CFLAGS]):
        base, err = compile_base(compiler, args.source, flags, i, asm)
        if base is None:
            print(f"{flags:16} compile failed\n{err}")
            continue
        pct = match_percent(target, base, args.function)
        shown = "symbol missing" if pct is None else f"{pct:6.2f}%"
        print(f"{flags:16} {shown}")
        best = max(best, pct or 0.0)
        if pct != 100.0 and not args.quiet:
            left, right = disasm(target, args.function), disasm(base, args.function)
            for n in range(max(len(left), len(right))):
                a = left[n] if n < len(left) else ""
                b = right[n] if n < len(right) else ""
                print(f"   {'  ' if a == b else '!='} {a:48} | {b}")
    sys.exit(0 if best == 100.0 else 1)


if __name__ == "__main__":
    main()
