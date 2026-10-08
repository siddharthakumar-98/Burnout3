# Existing solutions for PS2 dependency interfaces

Research checked on 2026-10-08. The active goal is matching C/C++ reconstruction of Burnout 3's PS2 game source.
Public completion tracks only objdiff's `game` category. D11 now supports dependency interfaces and selective reuse;
full vendor-library reconstruction is optional, and the Rust rewrite is a deferred, long-term goal. These are
candidate references, not imported dependencies or verified Burnout 3 matches.

The retail ELF contains game code and statically linked vendor code. Both originated as source, but the vendor
libraries were generally supplied to the studio as compiled archives. Recovering game source and reconstructing
every library in the ELF are different amounts of work. The current Phase 1 gate requires the game's source and
verified interfaces to its preserved dependencies. Keeping vendor libraries in generated assembly preserves the
matching executable without requiring another engine/SDK decompilation. The complete objdiff report still retains
the library categories for diagnostics, but they do not contribute to public game progress.

## Default workflow: reuse existing knowledge

For a game function that calls a library, first consult the references below for names, prototypes, structures,
constants and algorithms. Confirm those details against Burnout 3's callers and data layout, then use them in the
game C/C++ declarations. Preserve the vendor implementation. Only recover library internals when doing so saves
game-source work; matching every library is not a prerequisite for finishing this PS2 milestone.

## Findings

| Library | Best candidate | What was checked | Value and limit |
|---|---|---|---|
| RenderWare 3.6 graphics | [DK22Pac/plugin-sdk](https://github.com/DK22Pac/plugin-sdk), [gta-reversed](https://github.com/gta-reversed/gta-reversed), plus the sources below | Versioned headers and implementation bodies | Useful types, constants, prototypes and selected algorithms. No complete, verified PS2 3.6 engine source or matching decomp was found. |
| Sony libsce | [nathanialf/ico](https://github.com/nathanialf/ico), inspected at `5499dee` | `sce/` source families, `graph009.c`, `cdvd005.c`, `libdma.c`, progress and compiler scripts | Actual reconstructed PS2 library bodies and matching-toolchain evidence. Older SDK revisions; coverage and compatibility must be checked per function. |
| EA DirtySock | [deadbeef7/DirtySDK](https://github.com/deadbeef7/DirtySDK), inspected at `512fe5e`; [kitsilanosoftware/DirtySDK](https://github.com/kitsilanosoftware/DirtySDK) | File trees, implementation bodies and version files | The first reconstructs a small subset for 4.7.0/5.6.2. The second provides later 7.5.3 source. Neither establishes a match for Burnout 3's PS2 library. |
| Logitech devices | [b3dllc/burnout2](https://github.com/b3dllc/burnout2), inspected at `61e466e` | `lgdevPS2.c`, its header and the matching-status notes | Recovered EE-side device RPC and force-feedback code, including 19 beta entry points. Strong reference for `lgdev`; it does not provide all of Burnout 3's `lgaud`, `lgkbm` and codec category. |

## RenderWare: engine bodies versus wrappers

- [plugin-sdk's rwplcore.h](https://github.com/DK22Pac/plugin-sdk/blob/master/plugin_sa/game_sa/rw/rwplcore.h)
  identifies its library version as `0x36003` (3.6.0.3). Its
  [RenderWare.cpp](https://github.com/DK22Pac/plugin-sdk/blob/master/plugin_sa/game_sa/RenderWare.cpp) largely calls
  functions at addresses in GTA's executable. Those wrappers expose interfaces, not the called engine bodies.
- [gta-reversed's rwcore.cpp](https://github.com/gta-reversed/gta-reversed/blob/master/source/game_sa/RenderWare/rw/rwcore.cpp)
  also retains many address-based wrappers, including `RwFrameCreate` and the `RxHeap` functions. It supplies useful
  interface knowledge and selected recovered functions, not a complete independent 3.6 engine.
- [SilentPatch](https://github.com/CookiePLMonster/SilentPatch) explicitly mentions RW 3.6 for GTA:SA but requires
  the corresponding SDK to be installed separately. That mention does not mean it bundles the engine.
- [sigmaco/rwsrc-v3.7.0.2](https://github.com/sigmaco/rwsrc-v3.7.0.2) contains actual shared graphics-engine source,
  including `babinary.c` and `p2heap.c`. Its driver tree contains common, D3D8, D3D9, null and stub drivers, not sky2.
  [CoolManBob/ALReborn's rwversion.h](https://github.com/CoolManBob/ALReborn/blob/master/RWSDK/src/plcore/rwversion.h)
  was another source-tree lead, but declares 3.7.0.2 too; references to older versions there are compatibility checks.
- [b3dllc/burnout2](https://github.com/b3dllc/burnout2) has recovered RenderWare core, world and PS2 sky2 bodies:
  `basky.c`, `baskytran.c`, `skyinst.c`, `nodeps2matinstance.c`, `nodeps2matbridge.c` and others. Its
  [version header](https://github.com/b3dllc/burnout2/blob/master/include/rwsdk/rwversion.h) identifies 3.4.0.1.
  The project reports about 70% matching overall, not complete fidelity. This is a closer platform reference than
  Windows graphics code, but a different library version and potentially different compiler.
- [aap/librw](https://github.com/aap/librw) contains a real reimplementation and PS2-related code. It can explain
  algorithms and formats; equivalent behavior does not imply the original instruction sequence.

There is a concrete example of source-assisted matching:
[Persona 4's babinary.c](https://github.com/Raikaru/Persona4-Decompilation/blob/main/src/renderware/plcore/babinary.c)
states that the 3.7.0.2 source was verified byte-exact with MWCCPS2 3.0.1 b119, and its compiler-unit configuration
selects that compiler for RenderWare units. This is evidence for that game's 3.7 library, not a verification of our
3.6 library. Burnout 3's graphics region is ee-gcc output, so reusing the algorithm still requires the correct
library compiler, flags and data layout.

Persona 4's [sky2 research](https://github.com/Raikaru/Persona4-Decompilation/blob/main/docs/sky2/README.md) also
describes using Burnout Revenge's link map and NBA Ballers' debug information to identify vendor functions.
Its measurements explicitly warn that cross-game name proposals can be wrong. The naming and attribution method
is useful; its address mappings must not be transferred directly to Burnout 3.

## Sony libraries

ICO's `sce/` tree contains reconstructed `libgraph`, `libcdvd`, `libdma`, `libipu`, `libkernl`, `libmc`, `libmpeg`,
`libpad`, `libscf` and other families, plus runtime code. Concrete implementation examples are
[sceGsSetDefDBuff](https://github.com/nathanialf/ico/blob/main/sce/libgraph/graph009.c),
[sceCdRead](https://github.com/nathanialf/ico/blob/main/sce/libcdvd/cdvd005.c) and
[libdma](https://github.com/nathanialf/ico/blob/main/sce/libdma/libdma.c).

Its progress report says the full `.text` is identical and comes from tracked sources; that definition includes
assembly as well as C. Its [compiler script](https://github.com/nathanialf/ico/blob/main/tools/compile_c.sh) documents
ee-gcc 2.9-991111, archive-specific assembler behavior, SDK version tags 2200/2240 and `-G 0` for SDK libraries.
Our layout includes later tags such as `PsIIlibgraph2800`, and newer families such as `libpad2`, `libmc2` and network
libraries, so ICO is a substantial starting point rather than a complete replacement. Its matching claims were
inspected, not independently reproduced here.

[PS2SDK](https://github.com/ps2dev/ps2sdk) is another useful implementation and interface reference. It is an open
homebrew SDK, not automatically the same source as the retail Sony libraries. Conversely, the inspected
[OpenRAC sceCdRead](https://github.com/OpenRAC/OpenRAC/blob/master/games/rac1/ntsc/src/assembly/sdk/library/sceCdRead.c)
keeps expected assembly in the matching path and places its C body under `NON_MATCHING`; a `.c` filename alone
does not establish a completed match.

## DirtySock and Logitech

[deadbeef7/DirtySDK's README](https://github.com/deadbeef7/DirtySDK/blob/main/README.md) calls it experimental and
targets 4.7.0 and 5.6.2. Its inspected tree contains `commudp.c`, headers and a 5.6.2 `dirtylib.c`, plus limited
tests. This can help recover transport state, packet rules and function signatures. It does not contain the full
554-function library or its PS2 RPC backend. Burnout 3's exact DirtySock version remains to be established.
[kitsilanosoftware's version file](https://github.com/kitsilanosoftware/DirtySDK/blob/master/version.txt) says 7.5.3,
June 23, 2009; use it as a later reference, not a known 2004 match.

[Burnout 2's lgdevPS2.c](https://github.com/b3dllc/burnout2/blob/master/src/gamesource/toolkits/lgdevPS2.c) defines
synchronous and asynchronous calls, RPC packet fields, device descriptors and force-effect structures. Its own
notes record instruction differences and GCC provenance, so the 19 implemented beta entry points must not be
reported as 19 matching Burnout 3 functions. Burnout 3 already has named functions in the same families, including
`lgDevEnumerate`, `lgDevRead` and force-effect calls; it also has multi-device and other changed entry points.

An additional lead is Burnout 2's partial RenderWare Audio reconstruction, including
[rwarpc.c](https://github.com/b3dllc/burnout2/blob/master/src/rwsdk/rwaudio/source/ps2/spu2/common/rwarpc.c), EE SPU2
files and core/object modules. Other files explicitly contain stubs. This improves the earlier search result for
audio, but is not a complete verified engine. Original [Speex releases](https://www.speex.org/downloads/) remain
a candidate for the codec portion of our Logitech category; neither reference completes `lgaud` or `lgkbm`.

## Applying these references

Interface recovery is the default. If a source replacement would help, test it one unit at a time:

1. Identify the Burnout 3 function and library revision from its code, callers, constants and data references.
2. Compare the candidate's control flow, structure offsets, error paths and callees against our binary.
3. Recover the correct compiler and flags for that library. Do not assume the game's CodeWarrior profile applies.
4. Adapt only the evidenced source and declarations, then compare the generated object with the original.
5. Link only at 100% matching, check the executable hash and regenerate progress through the existing tools.

The vendor implementations are often C. Keep their C compilation and linkage where the binary requires it;
our C/C++ decomp does not require rewriting every library as C++ classes. External source presence, name recovery
and functional equivalence each help the work, but none alone counts as a matched function. Optional vendor
matches do not add game-code progress, and native replacement engines are outside the current PS2 scope.
