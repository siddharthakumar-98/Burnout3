# CLAUDE.md

Matching C/C++ reconstruction of Burnout 3's PS2 game code (SLUS_210.50, SHA-1 `332be40d…`) is the current goal.
Progress and completion use only objdiff's `game` category. Vendor libraries/runtime remain preserved assembly;
full library reconstruction is optional and does not block the game-source gate. Use the existing sources and
decomps in `Burnout3_decomp/docs/library-references.md` first for library names, types and algorithms. Any vendor
source adopted into the matching build still needs its own 100% match. A Rust rewrite is a deferred, long-term goal.
`ROADMAP.md` is the source of truth for plan and status; keep its Status table current. Each milestone keeps a log
in `Burnout3_decomp/docs/` (`d4.md`: subsystem map, unit boundaries, progress, open questions; D5 gets `d5.md`).

## Rules
- Never commit game-derived or proprietary files: ELF, `orig/`, `assembly/asm/`, `assembly/assets/`, `build/`,
  `compilers/`, `tools/bin/`, extracted strings or tables. The repo holds source, headers, names, configs, docs.
- Commit or push only when asked, with the user's exact message, ending with
  `Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>`.
- Every change must keep the build printing `build/SLUS_210.50: 332be40d… OK`.
- Don't delete generated build files to force a clean build; clean-build checks go in a fresh clone in the scratchpad.
- The repo is the source of truth; Ghidra mirrors it (never the other way round without review).
- Time-box a stuck function at ~8 attempts, record the best % and the lead in the milestone's doc, move on. A milestone
  closes with at most 10 such tail functions (ROADMAP "Tail policy"); tail runs clear them later.

## Build (from `Burnout3_decomp/`)
```
tools/dock python3 configure.py           # hash check, splat split, build.ninja (--no-split: skip splat)
tools/dock ninja                          # assemble, compile C_UNITS, link, check SHA-1
tools/dock python3 tools/progress.py      # PROGRESS.md, progress_map.svg, progress.json (commit these)
tools/dock python3 tools/funcmatch.py <func> c_cpp/src/<unit>.c [-c compilers/<ver>] [-f=FLAGS]
tools/dock python3 tools/funcmatch.py --all c_cpp/src/<unit>.c        # one line per function
tools/dock python3 tools/funcmatch.py <func> --variants build/var/<func>   # one line per source variant
```
`tools/dock` runs in the `b3-build` Docker image (linux/amd64). Ghidra tools run on the host, not in the container:
`python3 tools/ghidra_sync.py status|push|pull` and the read-only `python3 tools/ghidra_read.py fn|xrefs|mem`
(ghidra-mcp at 127.0.0.1:8089, program SLUS_210.50).

## Layout
- `Burnout3_decomp/assembly/splat/b3.yaml`: the split. Game units `game/unit_<VRAM>`; carved units under `d2/`,
  `d3/`, `d4/`, `d5/` survive regeneration.
- `Burnout3_decomp/configure.py`: `C_UNITS` (`linked: True` only at 100%), `CATEGORIES`, compiler and `CFLAGS`.
- `Burnout3_decomp/config/symbol_addrs.txt`: names. Functions `type:func`; data needs a real type plus size
  (`type:u8 size:0x100`, `u16`, `u32`, `f32`); `type:data` breaks splat.
- `Burnout3_decomp/c_cpp/{src,include}`: decompiled code; `src/<unit>.c` replaces `assembly/asm/<unit>`.
- `Burnout3_decomp/tools/`: `tusplit.py` (unit boundaries; hand fixes in `FORCED_CUTS`/`MERGES` with reasons),
  `dataslice.py` (per-unit data slices; data `FORCED_CUTS` keyed by `(stream, VRAM)`), `litfix.py` (`.lit4` pool),
  `xref.py`, `elf.py`, `progress*.py`, `ghidra_sync.py`, `bo2map.py` (Burnout 2 pairs: `--bo2 PATH [--unit U]`).
- `Burnout3_decomp/docs/`: `layout.md` (memory map, units), `compiler.md` (compiler identification), `d4.md`,
  `burnout2.md` (Burnout 2 mapping), `library-references.md`.
- `Burnout3_rust/`: Phase 2 (deferred; long-term goal; ISO extractor `iso_extract` lives here).

## Adding or decompiling a unit
0. Find its Burnout 2 counterpart (`Burnout3_decomp/docs/burnout2.md`): Criterion's names, layouts and a draft body.
1. Carve it in `b3.yaml` (16-byte aligned start/end), or regenerate with `tusplit.py --yaml` / `dataslice.py --yaml`.
2. Name functions/data in `symbol_addrs.txt`; write C in `c_cpp/src/<unit>.c`, prototypes in `c_cpp/include/`.
3. `funcmatch.py` until 100%, then `linked: True` in `C_UNITS`; reconfigure, build, check SHA OK.
4. `progress.py`, `ghidra_sync.py push`, update `docs/d4.md` (and README/ROADMAP counts if they changed).

## Compiler notes
- CodeWarrior PS2, decomp.me `3.0.1b119-040914` (settled in D4 by a sweep of all 19 `3.0.x` builds; the ELF's
  `2.4.1.01` stamp comes from a library object), `-O4 -str readonly -Cpp_exceptions off`; runs under wibo. Other
  builds sit beside it in `compilers/`. Don't switch compilers to chase one function (`docs/compiler.md`).
- Matching agents: `.claude/agents/decomp-matcher.md` (one unit per agent; agents edit only their C file and header,
  never run `configure.py`, and only read Ghidra). Run one agent at a time (parallel runs exhaust the usage limit);
  the main session only orchestrates (dispatch, carve, name, link, build, docs) and doesn't match functions itself.
  Check a unit's boundaries in Ghidra (`ghidra_read.py fn <addrs> callers,callees,refs`) before dispatching it.
- No system headers: declare libc prototypes yourself. Fixed-size types in `c_cpp/include/types.h`; wide chars are
  `u16`. C headers used from C++ need `extern "C"`.
- Float literals stay in the shared `.lit4` pool; the build repoints them (`litfix.py`), nothing to do in C.
- A renamed function breaks old callers that used `func_XXXXXXXX`: include the new header and call it by name.

## Ghidra
Ghidra 12.1.4 + ghidra-emotionengine-reloaded, project `~/Desktop/Ghidra/Burnout3_decomp`, served by ghidra-mcp
v7.0.0 (bethington fork of LaurieWired's). Keep Strict Naming Enforcement off (or pass `strict_mode=off`). Functions
carry `unit:<path>` tags from `push`. Known: 0x1FFFD8 is still typed as data; functions at 0x4843D0–0x4854E8 are
libkernl kernel patches in `.data` (expected).

## Plugins
- **superpowers**: new milestones or tooling go brainstorming → spec → writing-plans → execution; small fixes skip
  the spec. Specs and plans live in `docs/superpowers/{specs,plans}/`. This file wins on conflicts: skills that commit
  their own docs don't here (commit only when asked, with the user's message).
- **claude-mem**: cross-session recall. Search it (`mem-search`) before re-deriving earlier findings (compiler
  experiments, unit evidence). It is recall, not the record: findings still go in `docs/` and `ROADMAP.md`. Keep its
  cloud sync off for this repo; its observations can hold disassembly and game strings.

## Related
MC3DER (`~/Desktop/MC3DER`, https://github.com/siddharthakumar-98/MC3DER: sister decomp, starts from a copy of this
repo's tooling; port tool improvements there) and GameMerge (`~/Desktop/GameMerge`, consumes this repo's symbols and
the Rust core). This repo never depends on them.
