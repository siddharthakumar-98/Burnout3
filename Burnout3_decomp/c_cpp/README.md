# c_cpp/

The C/C++ side of the Burnout 3 decomp: source that CodeWarrior (`../compilers/3.0.1b119-040914/`) compiles into exactly
the original bytes.

The active completion scope is the game's `game` progress category. Vendor libraries stay in the matching assembly
build; consult [existing library references](../docs/library-references.md) for names, types and algorithms before
recovering an interface from scratch. Optional library matches use their own compiler and do not change game progress.

| Path | What it is |
|---|---|
| `src/` | Decompiled C/C++ (`.c` or `.cpp`). Each file replaces the assembly unit with the same path under `../assembly/asm/`. |
| `include/` | Shared headers (passed to the compiler as `MWCIncludes`) |

## Adding a unit

1. Carve the function(s) out of their `game/unit_<VRAM>` in `../assembly/splat/b3.yaml`, and name the remainder
   after them `game/unit_<VRAM>`. CodeWarrior functions always start on 16-byte boundaries, so the unit's start
   and end are too. (`tools/tusplit.py --yaml` regenerates the game units but keeps `d2/`, `d3/` and `d4/` carves, and the
   boundaries set by hand in its `FORCED_CUTS` and `MERGES`; `d5/` and `d6/` carves also survive.)
2. Write the C/C++ in `src/` with the same path.
3. Iterate with `tools/dock python3 tools/funcmatch.py <function> c_cpp/src/<unit>.c` until it reports 100%.
4. Add the unit to `C_UNITS` in `../configure.py` with `linked: True`. If it owns data, such as a switch's jump
   table, also carve that data and map it under `data`. Game units already have their own slices of `.data`,
   `.rodata`, `.sdata`, `.sbss` and `.bss` (`data/<unit>.data`, `data/<unit>.sbss`, …), so a C file that replaces
   a whole unit maps those. Float literals need nothing: they stay in the shared `.lit4` pool, and the build points
   the object at them (`../tools/litfix.py`, see [../docs/layout.md](../docs/layout.md#small-data-and-the-lit4-pool)).
5. Run `tools/dock python3 configure.py && tools/dock ninja`. The build must still print `332be40d… OK`.

## Current units (D2 compiler tests, D3 small-data tests)

| Unit | Functions | Tests | Status |
|---|---|---|---|
| `src/d2/func_0013C910.c` | `func_0013C910`, `func_0013C930` | leaf, two functions in one file | 100%, linked |
| `src/d2/func_00131AA0.c` | `func_00131AA0` | leaf, array of structs | 100%, linked |
| `src/d2/func_0014E7E0.c` | `func_0014E7E0`, `func_0014E830` | float to int, float copies | 100%, linked |
| `src/d2/func_0014DD80.c` | `func_0014DD80` | float return with a branch | 100%, linked |
| `src/d2/func_00136E00.c` | `func_00136E00` | global through `$gp` | 100%, linked |
| `src/d2/func_0013B740.c` | `func_0013B740` | call with stack frame | 100%, linked |
| `src/d2/func_0014EC30.c` | `func_0014EC30` | switch, jump table placed in data | 100%, linked |
| `src/d2/func_0028B700.cpp` | `CUnk0028B700::CUnk0028B700()` | C++ constructor and vtable | 100%, linked |
| `src/d2/func_00131CE0.c` | `func_00131CE0` | large struct offsets | 100%, linked (build 119) |
| `src/d2/func_0013AE70.c` | `func_0013AE70` | call or return 0 | 100%, linked (build 119) |
| `src/d2/func_0013B670.c` | `func_0013B670` | min/max clamp | draft; the original is inline asm |
| `src/d3/func_002527F0.c` | `func_002527F0` | float literal from the `.lit4` pool | 100%, linked |
| `src/d3/func_003EA7E0.c` | `func_003EA7E0` | float literal in a delay slot | 100%, linked |
| `src/d3/func_0027A900.c` | `func_0027A900` | literal load scheduled before a store | 100%, linked (build 119) |

Class and struct names such as `CUnk0028B700` are placeholders until the real ones are known. Details of how the
compiler was identified are in [../docs/compiler.md](../docs/compiler.md).

## Conventions (settled in D4)

- **Virtual calls need C++.** MW loads a vtable slot into `$t9` only for a real C++ virtual call; a C function
  pointer goes through `$v0`. So a unit that makes virtual calls is a `.cpp` file, and the class is declared in its
  header with `virtual` methods in slot order (`include/vdb.h`, `include/fs.h`). MW vtables start with two header
  words, so the first virtual sits at `+0x08`.
- **Methods can stay C names.** A class's own methods may be written as `extern "C"` functions that take `self`
  (same code as a method), so they keep their `func_XXXXXXXX` or chosen C names in `symbol_addrs.txt`. Real methods
  use MW mangling (`__ct__12CUnk0028B700Fv`, vtable `__vt__12CUnk0028B700`), and those mangled names go in
  `symbol_addrs.txt` as they are.
- **Vtables as data.** A vtable can be emitted from C as a plain `void *name[] = { 0, 0, (void *)method, … }` and
  mapped onto the unit's `.vtables` slice with `"data": {"data/d4/<unit>_vt.data": ".data"}` in `configure.py`
  (`src/d4/valuedb.cpp`); `tools/dataslice.py` cuts `.vtables` into one `<unit>_vt` slice per unit.
- **Externs reached with `lui`.** A global the original reaches with `lui`/`addiu` (not `$gp`) must be declared as
  an incomplete array (`extern u8 D_004EE040[];`), otherwise CodeWarrior assumes small data and uses `$gp`.
- **Criterion's names.** Where Burnout 2's debug info has the same class or function ([docs/burnout2.md](../docs/burnout2.md)),
  use its names: real C++ classes and member functions, mangled names in `symbol_addrs.txt` taken from what our
  declarations compile to. Inside a member function use `this` and members directly: a local alias
  (`CGTFileSystem *dev = this;`) changed register allocation (`CGTFileSystem::Init`, 94.9% until removed). `long` is
  64-bit here, so a Burnout 2 signature with `long` may need `s32`.
- **Address order.** Functions (and data) in a file are defined in address order: the object's order is the link
  order. `funcmatch.py` can't see a wrong order; only the SHA-1 check can.
- **Relocation-only differences.** An objdiff score under 100% that differs only in relocation names (jump tables
  `jtbl_…` vs `@N`, splat branch labels) is a real match; the link and SHA-1 decide.

## D4 units (core infrastructure)

| Source | Unit (start) | What | Status |
|---|---|---|---|
| `src/d4/pool.cpp` | `0x2B6C40` | Criterion's `GtLList` link pool | 8/8, linked |
| `src/d4/vdb.cpp` | `0x21B8C0` | value registry (`VdbRegistry`), CRC table | 8/8, linked |
| `src/d4/valuedb.cpp` | `0x24FDE0` | tuning database (`Data/vdb.xml`), vtable from C | 6/6, linked |
| `src/d4/fs.cpp` | `0x212580` | Criterion's `CGTFileSystem`/`CGTFile`, RenderWare file interface | 18/18, linked |
| `src/d4/ustrfmt.c` | `0x212E30` | UTF-16 `%N` substitution | 2/2, linked |
| `src/d4/ustring.c` | `0x2130F0` (carved) | UTF-16 strings | 12/14 |
| `src/d4/heap.c` | `unit_003E7850`, `_003E7AD0`, `_003E7D40` | heap core (slot table) | 9/13 |
| `src/d4/options.cpp` | `unit_0021B5E0` | options | 4/5 |
| `src/d4/memmgr.cpp` | `0x222300` | memory manager (the heap object's vtable), arena layout | 7/7, linked |
| `src/d4/loadqueue.cpp` | `unit_0013CE20` | Criterion's `CAsyncLoadManager` | 3/5 |
| `src/d4/func_00130BF0.c`, `func_001AB010.c` | `0x130BF0`, `0x1AB010` | first VU0 functions (helpers in `include/vu0.h`) | 100%, linked |
| `src/d4/gamemode.cpp` | `d4/gamemode`, `0x13C940` | two-player mode (D5 work, matched early) | 8/9, registered but not linked |

## D5 units (main loop and game flow)

D5 is closed under the roadmap's tail policy: **51/61 lifecycle/helper functions match, 16 are linked, and ten
unmatched functions remain**. The whole project now has 141 matched game functions, 94 linked after the D6 collision/body-helper batch. The complete executable
still reproduces the original SHA-1. [The D5 log](../docs/d5.md) records layouts, boundaries, individual tail scores
and the D6/D10 presentation routines outside this lifecycle inventory.

| Source | What | Status |
|---|---|---|
| `src/d5/boot.cpp` | game entry and boot/load/main-loop polling | 1/1, linked |
| `src/d5/unit_00131AC0.cpp` | startup, separators, rate scaling and current-mode test | 4/4, linked |
| `src/d5/unit_00131D20.cpp` | game object, init, frame state machine, load and shutdown | 14/16 |
| `src/d5/mode_base.cpp` | shared mode lifecycle and player/opponent/traffic setup | 3/4 |
| `src/d5/play_mode.cpp`, `src/d5/play_transition.cpp` | single-player lifecycle and loading/transitions | 10/10, linked |
| `src/d5/network_mode.cpp` | network mode lifecycle | 3/4; drawing remains assembly |
| `src/d4/gamemode.cpp` | two-player lifecycle | 8/9 |
| `src/d5/preview_exit.cpp`, `src/d5/preview_flow.cpp` | model-preview exit, load and init | 2/3; exit linked |
| `src/d5/frontend_flow.cpp` | frontend/movie lifecycle and stage loading | 6/10 D5; two further drafts belong to D10 |

Draft units compile for comparison and progress reporting, while `linked: False` retains their original assembly.
Float retargeting verifies the exact literal bits against the original function's pool loads; incorrect or
unverified constants are left unchanged with a warning (`tools/test_litfix.py`).

## D6 units (vehicle physics and handling, in progress)

| Source | What | Status |
|---|---|---|
| `src/d6/transmission_init.cpp` | initialize transmission state and find top gear | 1/1, linked |
| `src/d6/transmission_rpm.cpp` | engine angular-speed selection from forward ratios | 99.13%, unlinked |
| `src/d6/transmission_update.cpp` | clutch, automatic shifts, rev limiter, torque and boost modulation | 85.78%, unlinked |
| `src/d6/vehicle_resource.cpp` | release pooled vehicle resource | 1/1, linked |
| `src/d6/vehicle_shutdown.cpp`, `src/d6/vehicle_reset.cpp` | conditional callback and reset wrapper | 2/2, linked |
| `src/d6/vehicle_body_state.cpp` | signed body-state transition | 1/1, linked |
| `src/d6/vehicle_collision.cpp` | select fixed/local collision direction | 2/2, linked |
| `src/d6/vehicle_timer.cpp` | set vehicle deadline | 1/1, linked |
| `src/d6/vehicle_activate.cpp`, `src/d6/vehicle_register.cpp` | activation, list registration and initial height | 2/2, linked |
| `src/d6/vehicle_body_parts.cpp` | one-time batch allocation and single-part state transition | 2/2, linked |
| `src/d6/vehicle_cache_append.cpp` | append triangle vertices, normal and separate surface tag | 99.67%, unlinked |
| `src/d6/vehicle_collision_query.cpp` | query sphere, world lookup and optional triangle cache additions | 73.16%, unlinked |
| `src/d6/vehicle_collision_filter.cpp` | surface-tag, velocity/normal and normal-height filtering | 56.84%, unlinked |

`include/transmission.h` and `include/vehicle_physics.h` record the recovered retail layouts. [The D6 log](../docs/d6.md)
tracks evidence, the RPM register mismatch and caller-invariant questions, collision matching leads and the next physics paths.
