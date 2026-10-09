# Burnout 2 as a reference for Burnout 3

[b3dllc/burnout2](https://github.com/b3dllc/burnout2) (inspected at `61e466e`, 2026-09-07) decompiles the Burnout 2
August 5, 2002 beta (`SLUS_204.97`), which shipped with **complete DWARF1 debug info**. It is 100% decompiled and
about 70% matching under CodeWarrior 3.0.1. Its `docs/functions.csv` (2,311 functions with signatures and source
files), `docs/symtab_functions.csv` (1,756 more from the symbol table, mangled and demangled), `docs/globals.csv` and
`include/types/` (one header per recovered class/struct, with field offsets) give Criterion's own names for the
codebase Burnout 3 was built on. Its `docs/mwcc-*-model.md` notes model CodeWarrior's register allocator and scheduler
in detail, which is relevant to our tail functions.

## Spike (2026-10-08): do our D4 units exist in Burnout 2?

Every D4 unit checked has a Burnout 2 counterpart, with the same functions in the same order:

| Our unit | Burnout 2 | Evidence |
|---|---|---|
| `d4/fs` (18 functions) | `CGTFileSystem` (`M:\GameShared\GameClasses\FileSystem\GTFileSystem.cpp`) | Same order: `FExist`, `FFlush`, `FSeek`, `FEof`, `FPuts`, `FGets`, `FWrite`, `FRead`, `FClose`, `FOpen`, `Open`, `Init`, `SetAsDefaultFilesystem`, `BuildFileName`, `GetFileNameFromDeviceName`, `GetFileSystemFromFileName`, `InstallToRenderWare`. Sizes equal for `FExist` 0x48, `FFlush` 0x8, `FEof` 0x14, `FGets` 0x8, `FClose` 0x28, `Init` 0xC8, `SetAsDefaultFilesystem` 0x8; the rest grew a little. The `dvd:` device (`unit_001D2EE0`) is `CGTFileSystemPS2DVD`/`CGTFilePS2DVD` (`GameShared\...\FileSystem\ps2\`). |
| `d4/pool` (8) | `GtLList` / `GtLListPool` (no debug info; names from the symbol table) | Same order: `GtLListRemoveAllLinks`, `RemoveLink`, `TakeLinkFromList`, `AddLink`, `Initialise`, `PoolSortFree`, `PoolCalculateSize` (0x28, equal), `PoolCreateInMemory(GtLListPoolTag *, void *, int, int, int, void *)`. |
| load queue, `unit_0013CE20` (5) | `CAsyncLoadManager` (`Update`, `Abort`, `QueueLoadRequest(const char *, int *, void *, unsigned)`, `Init`) | `QueueLoadRequest` 0x2A0 vs our 0x2A4. Same layout: `PendingFileRequestTag maRequests[]` of 0x50 bytes (`macName`, `mpbCompletionReturn`, `mpBuffer`, `munReadLen`, `munId`), then `mpCurrentStream`, `mbAbortInProgress`, `mbPendingRead`, `munHead`, `munTail`, `munCurrentId`. Burnout 3 grew the ring from 16 to 24 and made the two flags bytes. Burnout 2's C bodies (`src/nodebug/CAsyncLoadManager.cpp`) are a direct lead for our three near-misses. |
| `d4/vdb`, `d4/valuedb` | `CValueDatabase` (`Init`) with per-class `RegisterValues()` (`CCamera`, `CAICar`, `CInGameCamera`, ...); Burnout 2 also has `CSoundValueDatabase` | Burnout 3 generalised the registry; the registration pattern is the same. |
| Mode lifecycle (D5) | `CGameMode` (`Init`, `Enter`, `Exit`, `Update`, `Pause`/`Resume`, `ActualPause`/`ActualResume`, `Restart`/`ActualRestart`, `Destroy`) plus `COnePlayerGameMode`, `CTwoPlayerSSGameMode`, `CTurnBasedGameMode` | Two-player methods are at `0x13C940`; common lifecycle helpers are at `0x134600`–`0x134930`. The preceding eight default stubs are shared with physics/object vtables. Burnout 3's frontend, preview, single-player and network modes have reworked layouts; retain anonymous names until individual correspondences are established (see `d5.md`). |
| memory manager, heap | not found by name (Burnout 2 has `linearmalloc.cpp` and RenderWare's heaps) | Likely new or reworked for Burnout 3. |

## How to use it

- **Mapping tool:** `python3 tools/bo2map.py --bo2 PATH` proposes pairs for the whole game into `build/bo2map.tsv`;
  `--unit UNIT` lists one unit's pairs and the Burnout 2 files to give the matcher (results in `d5.md`).
- **Names:** adopt Criterion's class, method and field names where the correspondence is evidenced (same order,
  similar size, same callees/strings). Methods get MW-mangled names in `symbol_addrs.txt`.
- **Matcher context:** for each unit, find the Burnout 2 counterpart first and give its header and C++ body to the
  matcher agent. Burnout 2's bodies are not all matching (see its notes), and Burnout 3 changed some layouts, so
  they are drafts, not answers.
- **Unit boundaries:** Burnout 2's `source_file` column shows which functions shared a file (`GameShared\...` vs
  `gamesource\...`), a strong prior for `tusplit.py` cuts.
- Burnout 2's own README lists "Burnout 3 (July 24, 2004 beta)" as a later target. If that beta carries debug info
  like Burnout 2's, it would name Burnout 3 directly.

## Applied

- 2026-10-08, D5: checked Burnout 2's mode hierarchy and lifecycle bodies before reconstructing Burnout 3's boot,
  game state machine and five embedded modes. Ghidra/call-site offsets establish the shared mode prefix and
  next-mode pointer. The C++ lifecycle inventory is complete with ten recorded matching tails; full results and
  presentation boundaries are in [d5.md](d5.md). Vendor internals remain preserved assembly.
- 2026-10-08: `d4/pool` renamed to `GtLList*` (now `pool.cpp`, C++ functions); 6 of its 8 mangled names equal
  Burnout 2's, the other two differ only in parameter types Burnout 3 changed. `d4/fs` rewritten as `CGTFileSystem` /
  `CGTFile` with real methods and Criterion's field names; `FOpen`, `FExist`, `FWrite`, `Open` and `Init` mangle exactly
  as in Burnout 2. Both still 100% and linked, SHA-1 unchanged. `options.cpp` now uses `fs.h`.
- 2026-10-08: the load queue rewritten as `CAsyncLoadManager` (real class, Criterion's fields) with Burnout 2's bodies
  as context: `Abort` went from 98.97% to 100% (3 of 5 now). Burnout 3 differences: 24 requests, `bool` flags and
  `bool *` completion flag, a new call in `Update`, `Abort` also aborting on status 3, `Init` returning `void`, and a new
  `Shutdown`.
