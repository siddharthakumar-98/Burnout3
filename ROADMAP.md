# Roadmap: Burnout 3: Takedown decompilation

This file is the single source of truth for the project's plan and status. Update it whenever a milestone completes or
the approach changes.

## Status

> **Done:** Burnout 3 matching build (byte-identical, from assembly), compiler locked, binary mapped ·
> **Currently:** PS2 game-source C/C++: 137 / 5,654 game functions match (90 of them linked; the rest sit in units
> that link once their remaining functions match). D4 done with a tail of 6; D5 done with a recorded tail of 10;
> D6 vehicle physics is in progress, with tail runs completing the remaining matches ·
> **Dependencies:** preserved vendor assembly, with existing solutions guiding interfaces · **Long-term goal:** Rust rewrite

| Phase | Milestone | State | Updated | Notes |
|---|---|---|---|---|
| 1 Decomp | D0 Environment | **done** | 2026-10-05 | Tools installed, build image works, ISO and ELF hashes verified, PCSX2 boots the ISO. Ghidra project set up (see Machine state). |
| 1 Decomp | D1 Matching build | **done** | 2026-10-05 | `tools/dock ninja` rebuilds `SLUS_210.50` byte-identical from splat assembly (8,948 functions, symbolic relocations). The rebuilt ELF boots in PCSX2 to the menu and into a race. |
| 1 Decomp | D2 Compiler and flags locked | **done** | 2026-10-08 | CodeWarrior **3.0.1 build 119** (decomp.me `3.0.1b119-040914`) at `-O4 -str readonly -Cpp_exceptions off`. Locked at 3.0.3 in D2; in D4 a sweep of all 19 `3.0.x` builds over every C function put b119 first or equal first on all of them, and it fixed D2's two near-misses and D3's `func_0027A900`. `-O4` over `-O3` settled by `ustrToUtf8` (99.9% vs 76.8%). The ELF's `2.4.1.01` stamp comes from a library object. See `Burnout3_decomp/docs/compiler.md`. |
| 1 Decomp | D3 Map the binary | **done** | 2026-10-06 | Libraries fenced off by compiler fingerprint and split one library per unit (libmpeg/libipu, libpad2/libdbc, libinsck/libmrpc, libmc2/netcnfif/libscf, newlib/libgcc); VU microcode split into 27 microprograms; every section boundary confirmed, including the small-data area: `.lit4` (float literals the linker pooled across files) `0x4E0680`, `.sdata` `0x4E1400`, `.sbss` `0x4E2680`, `.bss` `0x4E3000` with the libraries' COMMON block at its end. Game code split into 358 provisional translation units by `tools/tusplit.py`, and each unit given its own `.data`, `.rodata`, `.sdata`, `.sbss` and `.bss` slices by `tools/dataslice.py`; all 922 objects link at their original addresses. `tools/litfix.py` lets C units use the pooled literals, proven by two more linked functions. Progress per category in `Burnout3_decomp/PROGRESS.md`. See `Burnout3_decomp/docs/layout.md`. |
| 1 Decomp | D4 Core infrastructure | **done (tail: 6)** | 2026-10-08 | All planned D4 work is in C: file system (`CGTFileSystem`, 18), tuning registry and database (`d4/vdb`, `d4/valuedb`; `Data/vdb.xml` format and key hash in `tools/vdbhash.py`), link pool (`GtLList`), memory manager (`d4/memmgr`), load queue (`CAsyncLoadManager`), UTF-16 strings and formatting, the first VU0 functions (`vu0.h` inline-asm helpers); 66 functions in 21 files linked with the SHA-1 intact. Tail (see [tail policy](#tail-policy)): ustring 12/14, heap 10/13 (one real miss), options 4/5, load queue 3/5. Burnout 2's DWARF-named decomp (`Burnout3_decomp/docs/burnout2.md`) matched every D4 unit checked, which gave Criterion's names. Library functions named, libcdvd callers classified, C++ conventions in `c_cpp/README.md`. See `Burnout3_decomp/docs/d4.md`. |
| 1 Decomp | D5 Main loop and game flow | **done (tail: 10)** | 2026-10-08 | Boot, main loop/state machine, loading/shutdown and the frontend, preview, single-player, two-player and network mode lifecycles are in C++. 51/61 D5 functions match, 16 are linked; ten matching tails are recorded in `docs/d5.md`. Audio setup and drawing implementations remain D10 work. The mapping tool is done (42 high, 0 wrong in the hold-out check). Literal-pool retargeting now verifies values; all linked code preserves the original SHA-1. |
| 1 Decomp | D6 Vehicle physics and handling | **in progress** | 2026-10-08 | Ten functions recovered; eight exact and linked, torque update 85.78% and RPM selection 99.13% unlinked. Physics step, suspension, steering, drift and boost remain open. See `Burnout3_decomp/docs/d6.md`. |
| 1 Decomp | D7–D10 Game subsystems | not started | 2026-10-08 | Gameplay, modes, AI and game-side presentation/platform integration; game-code category only |
| 1 Decomp | D11 Dependency interfaces | ongoing supporting work | 2026-10-08 | Prefer existing library sources/decomps for names, types and behavior. Preserve vendor assembly; full library reconstruction is not a completion requirement. |
| 2 Rust rewrite | R1–R6 | deferred; long-term goal | 2026-10-08 | Requires the verified Phase 1 decomp; not current work |

## Overview

| Phase | Goal | Where |
|---|---|---|
| **1. Decompile Burnout 3's PS2 game code** (active) | Matching game C/C++, linked with preserved dependencies into a byte-identical `SLUS_210.50` | `Burnout3_decomp/` |
| **2. Rewrite Burnout 3 in Rust** (long-term goal) | A native Rust port of the finished decomp, with the same gameplay and assets from your disc | `Burnout3_rust/` |

Phase 1 covers the game's source code, including its integration with rendering, audio, networking and devices.
The public progress denominator is objdiff's `game` category; it currently contains 5,654 functions, 2,674,804 code
bytes and 393 units. Vendor libraries/runtime (3,453 functions), startup assembly and VU microcode are preserved
dependencies outside that metric. The full ELF must still match the original SHA-1.

Use existing source, headers and decomps before recovering a vendor interface from scratch. Full library
reconstruction is optional supporting work, not a gate. Phase 2 remains a deferred, long-term goal; game-source
completion alone does not supply a native renderer, audio engine or platform layer.

### Related projects
This repo is one of three. Dependencies point one way: this repo never depends on the others.

| Repo | Relationship |
|---|---|
| [MC3DER](https://github.com/siddharthakumar-98/MC3DER) | Sister project: the same decomp and Rust rewrite for Midnight Club 3: DUB Edition Remix (D0 in progress). It starts from a copy of this repo's tooling; when a tool improves here, the change is ported there. `ee` and `platform` are expected to be shared with it. |
| [GameMerge](https://github.com/siddharthakumar-98/GameMerge) | Consumes this repo: symbols and headers (Track A), and the Rust `burnout3-core` crate at tagged releases (Track B). |

This repo was split out of GameMerge on 2026-10-06. The history of D0–D3 (commits up to `b5044e0`) stays in
[GameMerge](https://github.com/siddharthakumar-98/GameMerge), where these folders were `Burnout3_decomp/` and
`Burnout3_rust/` at the top level. The folder names are unchanged, so internal links work as they did there.

### What we publish
- **Committed:** decompiled C/C++ source, headers, symbol names, build configs, Rust code and docs.
- **Never committed:** ISOs, BIOS, ELFs, splat-generated assembly and data, extracted assets, VU microcode, compilers,
  savestates and traces. Each user supplies their own disc, BIOS and compiler. The build tools regenerate everything
  else locally from the user's ELF after a hash check.
- Publishing decompiled code carries some legal risk. The project accepts it, follows the usual decomp-community
  practice above, keeps trademark and no-affiliation notices, and honors takedown requests.

### Machine state (2026-10-08)
| Item | State | Action needed |
|---|---|---|
| Burnout 3 ISO | `~/Desktop/ps2_games/Burnout 3 - Takedown (USA).iso`, hash verified | — |
| PCSX2 | `/Applications/PCSX2-v2.4.0.app` | — |
| BIOS | SCPH-39001 (USA v1.60) in `~/Desktop/ps2_bios usa/SCPH-39001_BIOS_V7_USA_160_(NTSC)/`, which PCSX2's BIOS folder setting points at | — |
| PINE | Off (`EnablePINE = false`, slot 28011) | Enable for Phase 2 trace capture |
| Docker | Image `b3-build` (linux/amd64: binutils 2.42, wibo 1.2.0, objdiff-cli 3.8.2, splat 0.50.0) | — |
| Rust | rustc 1.99 stable via Homebrew `rustup` (`/opt/homebrew/opt/rustup/bin`, on `PATH` via `~/.zshrc`) | — |
| Ghidra | 12.1.4 + OpenJDK 21 (Homebrew), ghidra-emotionengine-reloaded v2.1.38 enabled. Project `Burnout3_decomp` (outside the repo, in `~/Desktop/Ghidra/`) has `SLUS_210.50` imported as `r5900:LE:32:default`, with `gp = 0x4E8670`, a `bss` block `0x4E2680`–`0x1ECE9FF`, and `SECTION4` split into `0x100000`–`0x469DFF` (code), `vu_microcode` `0x469E00`–`0x483EFF` (not executable) and `data` `0x483F00`–`0x4E267F`. ghidra-mcp v7.0.0 (bethington) serves it on port 8089; `tools/ghidra_sync.py` keeps it in step with the repo, and matcher agents read it through the read-only `tools/ghidra_read.py` (plain HTTP, no MCP schemas). Turn off **Strict Naming Enforcement** (Edit > Tool Options > GhidraMCP HTTP Server) so library names and struct fields aren't rejected or prefixed. | — |
| Decomp helpers | Python venv at `Burnout3_decomp/.venv`, objdiff GUI and m2c in `Burnout3_decomp/tools/bin/` (gitignored) | — |
| Compiler | `Burnout3_decomp/compilers/3.0.1b119-040914/` (gitignored) is the build in use. The other decomp.me PS2 builds sit beside it for comparison (`2.3.3`, `2.4.0-build0017`, `3.0`, `3.0.1`, `3.0.3`, and the 2003–2006 `3.0`/`3.0.1` builds). All run under wibo. | — |

---

## Phase 1: Decompile Burnout 3 (active)

**Goal:** every function in objdiff's `game` category is matching C/C++, while the complete executable links with
preserved vendor dependencies, reproduces the original bytes and boots and plays in PCSX2 with your disc providing
the assets. Hand-written assembly and VU microcode remain assembly.

**Where it stands:** the build pipeline is complete and byte-identical, the compiler is locked (D2) and the binary
is mapped into libraries, sections and provisional translation units with their own data (D3). The current report
has 137 / 5,654 game functions in matching C/C++ (2.42%), covering 20,808 / 2,674,804 code bytes (0.78%). D4 (core
infrastructure) and D5 (main loop/game flow) are closed with recorded tails of 6 and 10. D6-D10 and tail runs recover the remaining game source, with
Burnout 2's DWARF-named decomp supplying Criterion's names and layouts wherever the code is shared; D11 supports the dependency interfaces rather than rebuilding every library.

### What the binary is
- Disc `SYSTEM.CNF`: `BOOT2 = cdrom0:\SLUS_210.50;1`, `VER = 1.00`, NTSC. ISO SHA-1 `11a7f335a37d2f3f5c13b967f3072c84f8eded02`.
- `SLUS_210.50` SHA-1 `332be40d6081b8b5055a6ea01194ad6ff662a863`. Stripped MIPS R5900 ELF, entry `0x100008`, with
  **one merged PT_LOAD** at `0x100000` (filesz `0x3E2680`, memsz `0x1DCEA00`) and `_gp = 0x4E8670` from `.reginfo`.
- **Game compiler:** CodeWarrior PS2 3.0.1 build 119 at the locked flags. The ELF's `2.4.1.01` stamp comes from a
  library object; vendor code has multiple compiler families (see `docs/compiler.md`).
- **Language and libraries:** C++ (MW-style RTTI) on top of RenderWare 3.6 (including the PS2 `sky2` driver),
  RenderWare Audio and Sony libsce.
- **Recovered layout** (details in [Burnout3_decomp/docs/layout.md](Burnout3_decomp/docs/layout.md)):

  | VRAM | Region |
  |---|---|
  | `0x100000`–`0x469E00` | `.text`: game (CodeWarrior) interleaved with RenderWare, libsce, runtime, DirtySock and Logitech libraries (ee-gcc) |
  | `0x469E00`–`0x483F00` | VU microcode (27 microprograms) |
  | `0x483F00`–`0x4B1500` | `.data` |
  | `0x4B1500`–`0x4D3E00` | `.rodata` |
  | `0x4D3E00`–`0x4DD820` | `.init`: C++ static initializers |
  | `0x4DD820`–`0x4DDAA0` | `.ctor` |
  | `0x4DDAA0`–`0x4E0680` | `.vtables` |
  | `0x4E0680`–`0x4E1400` | `.lit4`: float literals, pooled by the linker across files |
  | `0x4E1400`–`0x4E2680` | `.sdata` |
  | `0x4E2680`–`0x4E3000` | `.sbss` |
  | `0x4E3000`–`0x1ECEA00` | `.bss`, ending with the libraries' COMMON symbols |

- Disc contents and asset formats: [Burnout3_rust/PLAN.md](Burnout3_rust/PLAN.md).

### Repository layout: `Burnout3_decomp/`
```
assembly/                 the assembly side
  splat/b3.yaml           segment split of the load segment, plus carved-out units
  include/                asm macros
  asm/ assets/            (gitignored) generated by splat from your ELF
c_cpp/                    the C/C++ side
  src/                    decompiled C/C++; each file replaces the asm unit with the same path
  include/                shared headers
configure.py              one combined build: extracts, splits, assembles, compiles (C_UNITS, CFLAGS), links
tools/funcmatch.py        compares functions with the original: one, every function in a file (--all), or source variants (--variants)
tools/litfix.py           points a C object's float literals at the original's pooled .lit4 entries
tools/xref.py             cross-references, compiler fingerprints and strings, for mapping the binary
tools/ghidra_sync.py      keeps the Ghidra project's functions, names and unit tags in step with the repo
tools/ghidra_read.py      read-only Ghidra queries (decompilation, callers/callees, refs) for matcher agents
tools/vdbhash.py          Data/vdb.xml format and key hash; checks the code's key names against your own copy
tools/tusplit.py          proposes game translation-unit boundaries and writes the .text block of b3.yaml
tools/dataslice.py        cuts .data/.rodata/.sdata/.sbss/.bss into per-unit slices and writes that block of b3.yaml
tools/progress.py         writes PROGRESS.md, progress_map.svg and progress.json from objdiff's report
PROGRESS.md               generated progress table (progress_map.svg: the README's map; progress.json: its badge)
config/                   symbol names, relocation overrides, extra linker script (shared by both sides)
tools/elf.py              hash-checked extraction and exact ELF container rebuild
tools/dock                runs a command in the build container
docker/Dockerfile         linux/amd64 build image
docs/                     layout.md, compiler.md, d4.md (milestone log), burnout2.md (Burnout 2 mapping),
                          library-references.md (dependency references)
orig/ build/ compilers/   gitignored: your ELF, build output, your compiler
```
The matcher agent's instructions are in `.claude/agents/decomp-matcher.md` at the repo root.

### Tooling
| Purpose | Tool |
|---|---|
| Compiler | CodeWarrior PS2 `mwccps2` **3.0.1 build 119** (2004-09), run through **wibo** in the linux/amd64 image. Best or equal best on every C function among all 19 `3.0.x` builds; the game was built between b103 and b119 ([docs/compiler.md](Burnout3_decomp/docs/compiler.md)). |
| Assemble and link | GNU binutils (`mips-linux-gnu-as -march=r5900`, `ld`, `objcopy`). `tools/elf.py rebuild` wraps the linked segment in the original ELF container. |
| Split and disassemble | splat (`platform: ps2`, `compiler: MWCCPS2`) and spimdisasm (R5900: MMI, `lq`/`sq`, VU0 macro ops) |
| Diff and progress | objdiff (macOS GUI and CLI reports), asm-differ, decomp-permuter, decomp.me scratches |
| First-draft C | Burnout 2's counterpart, when there is one (see [Method per unit](#method-per-unit)); then Ghidra 12.1.x + ghidra-emotionengine-reloaded (read by agents through `tools/ghidra_read.py`) and m2c. `tools/ghidra_sync.py` mirrors the repo's functions, names (`config/symbol_addrs.txt`) and units into Ghidra. |
| Game-code reference | [b3dllc/burnout2](https://github.com/b3dllc/burnout2): Burnout 2's August 2002 beta, decompiled from full DWARF debug info (2,311 named functions with signatures and source files, about 1,000 types with offsets, 249 classes), built with CodeWarrior 3.0.1. Mapping and results in [docs/burnout2.md](Burnout3_decomp/docs/burnout2.md). Its `docs/mwcc-*-model.md` notes model CodeWarrior's register allocator and scheduler. |
| Matching | One `decomp-matcher` subagent at a time (`.claude/agents/decomp-matcher.md`): at most about 8 functions per run, an 8-attempt time-box per function, `funcmatch.py --variants` to try several source forms per turn. The main session orchestrates (boundaries, carving, names, linking, docs) and doesn't match functions itself. |
| Runtime | PCSX2 2.x: `PCSX2 -elf build/SLUS_210.50 -- <ISO>` boots the rebuilt ELF with the disc inserted from the start. Debugger and PINE for spot checks. |
| Library references | Burnout 2's recovered PS2 RenderWare/audio/Logitech, ICO's Sony libraries/runtime, shared RenderWare 3.7 source, Persona 4's matching methods, plugin-sdk/librw, DirtySDK and upstream newlib/fdlibm/libgcc/Speex. Verify versions and call-site layouts before use; no proprietary SDKs are copied. |

### Milestones
| ID | Milestone | Exit criterion | State |
|---|---|---|---|
| **D0** | Environment | Tools installed, image builds, ELF extracted and hashes verified, PCSX2 boots the ISO | **done** |
| **D1** | Matching build | Section boundaries recovered. `ninja` builds `build/SLUS_210.50` entirely from generated assembly with SHA-1 `332be40d…`. | **done** |
| D2 | Compiler and flags locked | At least 10 functions across at least 3 TUs byte-match: a leaf C function, float math, a C++ ctor/vtable, and a switch/jump table. Flags recorded in `configure.py`. | **done**: 10 functions in 8 files; compiler since refined to 3.0.1 b119 `-O4` (D4) |
| D3 | Map the binary | Libraries/runtime and VU microcode fenced off; game TU/data boundaries carved. Internal objdiff categories retain the complete binary; public completion tracks only `game`. | **done**: original library boundaries retained, per-unit data slices, `.lit4` pool handled for C units |
| D4 | Core infrastructure | Memory/heaps, math (vector/matrix, VU0 paths), file I/O and streaming, the tuning-variable system (`VDB.XML` key hash), strings/localization. Done when the units listed in [docs/d4.md](Burnout3_decomp/docs/d4.md) are in C with at most a recorded tail ([tail policy](#tail-policy)), core headers exist in `c_cpp/include/`, library functions are named, and the SHA-1 still matches. | **done (tail: 6)**: 66 functions linked; ustring, heap, options and load queue wait on 6 near-misses |
| D5 | Main loop and game flow | Starts with `tools/bo2map.py`, which proposes Burnout 3 ↔ Burnout 2 function pairs from order, size, vtables, callees/callers, strings and floats (done). Then boot, main loop, game state machine (`CGameMode` and its subclasses), mode/stage loading, frontend flow. | **done (tail: 10)**: lifecycle inventory in [docs/d5.md](Burnout3_decomp/docs/d5.md); 51/61 match, 16 linked |
| D6 | Vehicle physics and handling | Physics step, suspension, steering, drift, transmission, boost kick | **in progress**: 8/10 recovered functions exact and linked; torque update and RPM helper drafts. See [docs/d6.md](Burnout3_decomp/docs/d6.md). |
| D7 | Gameplay rules | Boost economy, scoring, takedown detection and types, crash state machine, Impact Time, aftertouch, crash cameras | |
| D8 | Game modes and progression | Race, Road Rage, Crash mode (pickups, multipliers, Crashbreaker), Eliminator, Burning Lap, Face-Off, World Tour, save data | |
| D9 | AI, traffic, camera | Racer AI and arbitration, traffic, follow/bumper/replay cameras | |
| D10 | Presentation and platform integration | Game-side rendering, deformation, particles, audio/EA Trax integration, frontend UI, video, memory-card, network and device callers. Preserve middleware internals. | |
| D11 | Dependency interfaces and reuse | Verify names, prototypes, structures and behavior needed by game callers using existing solutions. Keep vendor assembly unless a useful replacement matches under its own toolchain; full vendor decomp is optional. | ongoing supporting work |
| **Gate** | PS2 game-source decomp complete | 100% of `game` functions matching and their units linked (no tail left), the full ELF reproduces the original SHA-1, every vendor function called from game C has a named prototype in `c_cpp/include/` checked against its call sites, and [Testing Phase 1](#testing-phase-1) passes. No vendor-library matching quota. | |

Work runs infrastructure first, then gameplay, then game-side presentation/platform integration. D11 supports
each stage as its callers need library interfaces. Headers and structures grow outward from core code; work one
translation unit at a time, smallest functions first.

### Tail policy
Most functions match within a few attempts; a small tail resists (register swaps, delay slots, loop entry). A
milestone closes when all its units are in C and at most 10 functions remain short of 100%, each recorded with its
best % and lead in the milestone's log. Units with a tail function stay in assembly until it matches. Tail runs,
batched across milestones, clear them later. The Phase 1 gate still requires 100%: nothing is left behind, it is
only scheduled differently.

### Method per unit
1. **Boundaries:** confirm the unit in Ghidra (`ghidra_read.py fn <addrs> callers,callees,refs`) before any work.
2. **Burnout 2 first:** find the counterpart in Burnout 2 (by function order, size, callees, strings and
   constants). Where it exists, take Criterion's class, method and field names and its layout, and give its body
   to the matcher as a draft. Burnout 3 changes some layouts (the load queue grew from 16 to 24 requests, the pool
   struct shrank), so check every offset against the assembly.
3. **Match:** one matcher run per unit (≤8 functions), then carve, name (mangled C++ names in `symbol_addrs.txt`),
   link, check the SHA-1, regenerate progress and record results in the milestone's log.

### Prefer existing dependency solutions

The evidence and exact limitations are in [library references](Burnout3_decomp/docs/library-references.md).

| Dependency | First references to inspect | Use in this milestone |
|---|---|---|
| RenderWare graphics | [Burnout 2](https://github.com/b3dllc/burnout2) (3.4 PS2 sky2), [3.7 source](https://github.com/sigmaco/rwsrc-v3.7.0.2), [Persona 4](https://github.com/Raikaru/Persona4-Decompilation), [plugin-sdk](https://github.com/DK22Pac/plugin-sdk), [librw](https://github.com/aap/librw) | Recover 3.6 interfaces from headers and compare shared algorithms/driver behavior. No complete verified PS2 3.6 engine was found. |
| RenderWare Audio | [Burnout 2's audio/RPC reconstruction](https://github.com/b3dllc/burnout2/tree/master/src/rwsdk/rwaudio) | Use EE/SPU2 and object/stream references to understand game callers; distinguish implemented bodies from stubs. |
| Sony libsce/runtime | [ICO](https://github.com/nathanialf/ico), [PS2SDK](https://github.com/ps2dev/ps2sdk), original newlib/fdlibm/libgcc | Start from reconstructed library bodies, upstream algorithms and API records. Verify our newer SDK layouts. |
| EA DirtySock | [partial 4.7.0/5.6.2](https://github.com/deadbeef7/DirtySDK), [7.5.3 source](https://github.com/kitsilanosoftware/DirtySDK) | Reuse transport/protocol knowledge; identify Burnout 3's exact version and PS2 RPC differences. |
| Logitech devices/codecs | [Burnout 2 lgdevPS2.c](https://github.com/b3dllc/burnout2/blob/master/src/gamesource/toolkits/lgdevPS2.c), [Speex](https://www.speex.org/downloads/) | Start from recovered device/RPC structures and codec algorithms; headset and keyboard/mouse coverage is incomplete. |

For each needed interface: inspect these references, confirm prototypes and offsets against our game callers,
record the evidence, and leave the vendor implementation in preserved assembly. Only attempt library source
matching when it saves game-source effort. An optional adopted library function still needs the correct compiler,
100% byte matching and the full-ELF hash check; its match does not add game-code progress.

### Risks and open questions
| Risk / question | Mitigation |
|---|---|
| Vendor versions and compilers differ from references | Preserve their original assembly; use existing work to verify game-call interfaces. Full vendor reconstruction is outside the gate. Optional source matches require the library's own compiler and flags. |
| Is b119 the exact compiler? | The build is between b103 (loses `func_0013AE70`) and b145 (loses `ustrncat`/`ustrncpy`); b119 is the closest available and matches everything matched so far, including D2's former near-misses. Revisit if a tail function resists every source form. |
| Section and TU boundaries had to be inferred from one merged segment | Recovered in D3 from compiler fingerprints, link order in `.data`, `.rodata`, `.sdata`, `.sbss` and `.bss`, version tags, `$Id` strings, vtables and static initializers; every object links at its original address. Game units are provisional and get merged or split as decompiling uncovers files. |
| GNU ld standing in for the MW linker | Match the load segment, then rebuild the container in `tools/elf.py`. Already proven in D1. |
| R5900-specific code (MMI, VU0 macro, 128-bit loads/stores) | Keep it as inline asm where the original most likely was. Hand-decompile the rest. |
| C++ under CodeWarrior (mangling, vtables, inlining order) | Patterns settled in D2/D4 (`c_cpp/README.md`). Class names, methods and layouts come from Burnout 2's debug info where the code is shared, otherwise from RTTI strings and the code. |
| Burnout 2 is a draft, not an answer | Burnout 3 changed some layouts and signatures, and Burnout 2's own decomp is about 70% matching. Check every offset and signature against Burnout 3's assembly; only Burnout 3's bytes decide. |
| The tail grows with every milestone | Tail policy above: record, schedule tail runs, use Burnout 2's bodies and its CodeWarrior codegen notes as leads. |
| Scale: 2,674,804 game-code bytes in the current report | Prioritize game source, reuse library knowledge and track only `game` progress. Keep the complete binary report for diagnostics. |
| Whether public CI should verify matching | Open. Options: local-only, or a private runner holding your ELF. |

---

## Testing Phase 1

How we confirm the decomp builds, and behaves, exactly like the original. A byte-identical executable behaves
identically by construction, so most of these checks guard the build itself. The runtime checks then confirm the
boot path and catch anything the hash can't, such as a wrong load procedure.

### 1. The build reproduces the original
| Check | How | State |
|---|---|---|
| SHA-1 match | `tools/dock ninja` prints `build/SLUS_210.50: 332be40d… OK` and fails otherwise | **passing** |
| Clean rebuild | Build in a fresh scratch clone with the supplied ELF/compiler, then run `tools/dock python3 configure.py` and `tools/dock ninja`; preserve generated files in the working checkout. | **passing** |
| Byte comparison | `cmp build/SLUS_210.50 orig/SLUS_210.50` reports no differences | **passing** |
| Fresh-clone build | Follow [Burnout3_decomp/README.md](Burnout3_decomp/README.md) from a fresh clone with only the ISO present | **passing** (2026-10-06: a fresh clone with only the ELF and compiler added builds byte-identical and regenerates identical progress files; the ELF was copied in, not re-extracted from the ISO) |
| Nothing derived is tracked | `git status --ignored` shows `orig/`, `asm/`, `assets/`, `build/`, `compilers/` ignored, and `git ls-files` lists no binaries | **passing** |

### 2. The build is genuinely relinkable
| Check | How | State |
|---|---|---|
| Symbolic relocations | The generated assembly uses `jal`/`%hi`/`%lo`/`%gp_rel` symbol references, not hard-coded addresses (about 40k `jal`, 36k `%hi`, 11.7k `%gp_rel`) | **passing** |
| Shift build | Insert padding early in `.text`, rebuild with the hash check disabled, and boot it in PCSX2. It must still reach the menu and load a race, which proves no address is baked in as a plain number. First, the 116 symbols that still resolve to absolute addresses (`_end`, and references into the middle of functions or strings; [layout.md](Burnout3_decomp/docs/layout.md#open-items)) must become real symbols. | to do |

### 3. Each decompiled function matches
| Check | How | State |
|---|---|---|
| Per function | objdiff shows 100% for each game function accepted as matched; a whole unit must match before linking | **passing** (90 game functions linked; 137 matched in the current report) |
| Progress | `tools/dock python3 tools/progress.py` writes a game-only table, map and badge; changing vendor counts must not change these metrics | **passing** |
| No regressions | The full-ELF SHA-1 stays green after every unit lands, including the preserved vendor code | **passing** (current build matches the original) |

### 4. It runs like the original
All of these run the rebuilt `build/SLUS_210.50` (no `.elf` extension; `build/SLUS_210.50.elf` is an unfinished
intermediate file) with your ISO inserted from the start. Starting an ELF from PCSX2's menu boots without a disc, and
the game stalls on a black screen because it loads its modules from `cdrom0:` at startup. Use:

```bash
/Applications/PCSX2-v2.4.0.app/Contents/MacOS/PCSX2 -elf ~/Desktop/Burnout3/Burnout3_decomp/build/SLUS_210.50 -- ~/Desktop/ps2_games/"Burnout 3 - Takedown (USA).iso"
```

In `~/Library/Application Support/PCSX2/logs/emulog.txt`, `Serial: SLUS-21050` must appear before
`ELF Loading: host:…/build/SLUS_210.50`.

| Check | Pass criterion | State |
|---|---|---|
| Boot | Reaches the title screen and main menu | **passing** (2026-10-05) |
| Race | Loads a track and finishes a race. Boost, takedowns and crashes work. | partial: a race loads and plays. Finishing one with boost, takedowns and crashes is still to check. |
| Modes | Road Rage, Crash mode (pickups, Crashbreaker) and Eliminator each play through to their results screen | to do |
| Save data | A memory-card save from the original loads in the rebuilt build, and the reverse | to do |
| Side by side | The same savestate and input recording in the original and the rebuilt ELF give the same RAM at fixed frames (compared over PINE) | to do |

**The PS2 game-source milestone is verified** when every check above passes, all `game` functions match from C/C++
and their units are linked, and the dependency interfaces used by game code are verified. Vendor libraries may
remain original assembly. Their reconstruction is not part of the completion denominator.

---

## Phase 2: Rewrite Burnout 3 in Rust (long-term goal)

**Goal:** a native Rust version of Burnout 3, in `Burnout3_rust/`, that plays the same as the original. It loads assets
from your ISO at runtime and is ported from verified game source. It uses idiomatic Rust wherever that
doesn't change gameplay.

**Status:** deferred. The verified Phase 1 decomp is a prerequisite; passing its gate does not automatically start
the rewrite. Preserved PS2 vendor code still needs native replacements or platform shims; this is a separate
future scope, not a requirement of the current decomp. Prefer existing rendering/audio/input solutions when that
work is explicitly started.

<details>
<summary>Deferred Rust rewrite planning</summary>

### Tooling and structure
| Crate / tool | Role |
|---|---|
| `formats` | RenderWare binary stream, TXD, BGV/BTV, tracks, VDB, strings, audio |
| `ee` | EE float semantics (no denormals/inf/NaN, EE rounding) and RNG |
| `burnout3-core` | `no_std` gameplay core: simulation, rules, modes. A library with a stable public API (see below). |
| `rw` | RenderWare runtime replacement, with librw as a reference |
| `platform` | wgpu renderer, cpal audio, gilrs input, and disc/asset file system. **Contains nothing Burnout-specific.** |
| `burnout3` | The thin app binary that wires `burnout3-core`, `rw` and `platform` together |
| `tools/iso_extract` | Already exists. Used as a library so assets stream straight from your ISO. |
| C oracle | Matched game functions compiled natively only after their PS2 interfaces, pointer layouts and dependencies have suitable shims, then called from Rust tests |
| PCSX2 + PINE | Golden per-frame state traces from the matched ELF, using savestates and input recordings |

### Crate boundaries (decide before R1)
These rules keep the rewrite usable as a library, which GameMerge's native track depends on. They cost little now and
a lot to retrofit.

- **`burnout3-core` is a library first.** The app is only one of its users. No global state: the whole simulation
  lives in a `World` value, so two cores can run in one process.
- **One deterministic step:** `World::step(&mut self, input: &FrameInput) -> FrameEvents`. Takedowns, boost changes,
  crashes and camera requests come out as data in `FrameEvents`, not as side effects.
- **Rendering is an output, not a call.** The core produces draw and audio requests. `rw` and `platform` turn them
  into frames and sound. The core never touches wgpu or cpal.
- **External cars are allowed.** The core accepts car states that it doesn't simulate (puppets) through its public
  API. Burnout itself never needs this, but it's what lets another game drive Burnout's rules.
- **`platform` stays generic.** When MC3DER's Rust work starts, `platform` moves to its own repo and both games
  depend on it. Until then it lives here with no dependency on any other Burnout crate.
- **Releases are tagged** (`core-v0.x`). Downstream projects pin tags, never `main`.

### Port order
1. Formats and assets
2. Math and EE float types
3. Tuning system
4. Headless deterministic sim: physics → gameplay rules → modes → AI/traffic
5. Camera
6. Renderer (replacing the sky2/VU pipelines)
7. Audio
8. Frontend/UI and video
9. Save data

**Idiom policy:** use ownership instead of raw pointers, enums for state machines, `Result` for I/O, and traits instead
of vtables, wherever results don't change. **Float operation order, EE float semantics, RNG sequences and frame timing
are preserved exactly.**

### Milestones
| ID | Milestone |
|---|---|
| R1 | A track loads from the ISO and a fly-cam works |
| R2 | A drivable car with `VDB.XML` tuning |
| R3 | Boost, near misses and takedowns against AI and traffic |
| R4 | Crash mode at one junction |
| R5 | UI, Road Rage and World Tour |
| R6 | The full game is playable with all modes |

### Verification
- **Unit:** each ported function is differential-tested against the C oracle on fuzzed and recorded inputs. Results
  must be bit-identical.
- **System:** golden traces from PCSX2 (matched ELF, scripted input, savestates, read over PINE) are replayed into the
  headless Rust sim. The per-frame gameplay-state diff must be zero across the scenario suite.
- **Visual:** renderer output is compared by screenshot against PCSX2 for the same frame.
- `cargo test --workspace` runs format round-trips, oracle diffs and trace replays. Traces and extracted data stay local
  and gitignored, and tests that need them skip with a clear message.

### Risks
- **EE FPU vs IEEE** is the biggest fidelity risk. Use an `ee` soft-float type wherever trace diffs require it.
- The oracle needs a platform shim and 32-bit pointer assumptions, so scope it to pure logic TUs.
- Renderer parity (VU1 lighting, skinning, deformation) can only be judged by screenshot.
- Open: platforms beyond macOS (wgpu keeps Windows and Linux possible).

</details>

---

## What other projects need from this repo

[GameMerge](https://github.com/siddharthakumar-98/GameMerge) depends on this repo at specific milestones. Its own
roadmap tracks those dependencies. This table exists so that the order of work here can take them into account.

| GameMerge needs | From milestone here | Used by |
|---|---|---|
| Symbols, structs and function signatures for gameplay rules (boost, takedowns, crash state, Impact Time) | D7 | Track A (PCSX2 as linker) |
| Car slots and the race/mode state machine | D6, D8 | Track A |
| `burnout3-core` with the step API and puppet cars | R3 | Track B (native Rust) |
| Crash mode in `burnout3-core` | R4 | Track B |

Track B is on hold while Phase 2 is deferred. Track A is unaffected, and Burnout 2's names reach it early: the
gameplay classes it needs exist in Burnout 2 with Criterion's names.

None of this changes the decomp's own order of work (infrastructure → gameplay → game-side presentation/platform). It only
matters when choosing between two units of similar value.
