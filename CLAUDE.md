# CLAUDE.md

Matching decomp of Burnout 3: Takedown (PS2, SLUS_210.50, SHA-1 `332be40d…`), then a Rust rewrite.
`ROADMAP.md` is the source of truth for plan and status; keep its Status table current. D4 work is tracked in
`Burnout3_decomp/docs/d4.md` (subsystem map, unit boundaries, progress log, open questions).

## Rules
- Never commit game-derived or proprietary files: ELF, `orig/`, `assembly/asm/`, `assembly/assets/`, `build/`,
  `compilers/`, `tools/bin/`, extracted strings or tables. The repo holds source, headers, names, configs, docs.
- Commit or push only when asked, with the user's exact message, ending with
  `Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>`.
- Every change must keep the build printing `build/SLUS_210.50: 332be40d… OK`.
- Don't delete generated build files to force a clean build; clean-build checks go in a fresh clone in the scratchpad.
- The repo is the source of truth; Ghidra mirrors it (never the other way round without review).
- Time-box a stuck function at ~20 attempts, record the best % and the lead in `docs/d4.md`, move on.

## Build (from `Burnout3_decomp/`)
```
tools/dock python3 configure.py           # hash check, splat split, build.ninja (--no-split: skip splat)
tools/dock ninja                          # assemble, compile C_UNITS, link, check SHA-1
tools/dock python3 tools/progress.py      # PROGRESS.md, progress_map.svg, progress.json (commit these)
tools/dock python3 tools/funcmatch.py <func> c_cpp/src/<unit>.c [-c compilers/<ver>] [-f=FLAGS]
```
`tools/dock` runs in the `b3-build` Docker image (linux/amd64). Ghidra sync runs on the host, not in the container:
`python3 tools/ghidra_sync.py status|push|pull` (ghidra-mcp at 127.0.0.1:8089, program SLUS_210.50).

## Layout
- `Burnout3_decomp/assembly/splat/b3.yaml`: the split. Game units `game/unit_<VRAM>`; carved units under `d2/`,
  `d3/`, `d4/` survive regeneration.
- `Burnout3_decomp/configure.py`: `C_UNITS` (`linked: True` only at 100%), `CATEGORIES`, compiler and `CFLAGS`.
- `Burnout3_decomp/config/symbol_addrs.txt`: names. Functions `type:func`; data needs a real type plus size
  (`type:u8 size:0x100`, `u16`, `u32`, `f32`); `type:data` breaks splat.
- `Burnout3_decomp/c_cpp/{src,include}`: decompiled code; `src/<unit>.c` replaces `assembly/asm/<unit>`.
- `Burnout3_decomp/tools/`: `tusplit.py` (unit boundaries; hand fixes in `FORCED_CUTS`/`MERGES` with reasons),
  `dataslice.py` (per-unit data slices; data `FORCED_CUTS` keyed by `(stream, VRAM)`), `litfix.py` (`.lit4` pool),
  `xref.py`, `elf.py`, `progress*.py`, `ghidra_sync.py`.
- `Burnout3_decomp/docs/`: `layout.md` (memory map, units), `compiler.md` (compiler identification), `d4.md`.
- `Burnout3_rust/`: Phase 2 (not started; ISO extractor `iso_extract` lives here).

## Adding or decompiling a unit
1. Carve it in `b3.yaml` (16-byte aligned start/end), or regenerate with `tusplit.py --yaml` / `dataslice.py --yaml`.
2. Name functions/data in `symbol_addrs.txt`; write C in `c_cpp/src/<unit>.c`, prototypes in `c_cpp/include/`.
3. `funcmatch.py` until 100%, then `linked: True` in `C_UNITS`; reconfigure, build, check SHA OK.
4. `progress.py`, `ghidra_sync.py push`, update `docs/d4.md` (and README/ROADMAP counts if they changed).

## Compiler notes
- CodeWarrior PS2 (`MW MIPS C Compiler 2.4.1.01`), built with decomp.me `3.0.3-020716`,
  `-O4 -str readonly -Cpp_exceptions off`; runs under wibo. Other builds sit beside it in `compilers/`.
- Open lead: 3.0.1 b119 (2004-09-14) matches `ustrFromUtf8` at 99.8% vs 70.9% on 3.0.3. Test all matched functions
  under b119/b103/b145 before writing more C (see `docs/compiler.md`, "A later build?").
- No system headers: declare libc prototypes yourself. Fixed-size types in `c_cpp/include/types.h`; wide chars are
  `u16`. C headers used from C++ need `extern "C"`.
- Float literals stay in the shared `.lit4` pool; the build repoints them (`litfix.py`), nothing to do in C.
- A renamed function breaks old callers that used `func_XXXXXXXX`: include the new header and call it by name.

## Ghidra
Ghidra 12.1.4 + ghidra-emotionengine-reloaded, project `~/Desktop/Ghidra/Burnout3_decomp`, served by ghidra-mcp
v7.0.0 (bethington fork of LaurieWired's). Keep Strict Naming Enforcement off (or pass `strict_mode=off`). Functions
carry `unit:<path>` tags from `push`. Known: 0x1FFFD8 is still typed as data; functions at 0x4843D0–0x4854E8 are
libkernl kernel patches in `.data` (expected).

## Related
MC3DER (sister decomp, waits for the MC3 ISO) and GameMerge (`~/Desktop/GameMerge`, consumes this repo's symbols and
the Rust core). This repo never depends on them.
