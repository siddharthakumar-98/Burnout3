# bo2map.py: Burnout 3 ↔ Burnout 2 function mapping (design)

Date: 2026-10-08. Status: approved in chat ("start work"). First step of D5.

## Purpose

Propose Criterion names for Burnout 3 game functions by pairing them with Burnout 2 functions, and point the
orchestrator at the Burnout 2 source to give the matcher agent. Burnout 2 is input only: the tool parses just what
helps name Burnout 3, and every output row is keyed by a Burnout 3 address. It never writes `symbol_addrs.txt`; the
orchestrator applies names by hand (compile for the mangled name, then reconfigure and check the SHA-1).

## Inputs

- Burnout 3 (generated, not committed): `assembly/asm/**` via `xref.load()` (address, size, unit, `jal` callees, data
  refs); `orig/SLUS_210.50.rom` for strings and `.lit4` floats; `config/symbol_addrs.txt` for existing names. Game
  functions are those whose unit is in a `game` category of `configure.py` (`game/`, `d2/`, `d3/`, `d4/`, `sinit/`).
- Burnout 2: a checkout of `b3dllc/burnout2` passed as `--bo2 PATH` (kept outside the repo, e.g. in a scratchpad).
  - `docs/functions.csv` (DWARF) merged with `docs/symtab_functions.csv` by address: name, size, link order.
  - A Burnout 2 function counts as game code when its definition is found in `src/gamesource`, `src/GameShared` or
    `src/nodebug` (no-debug classes such as `CGameMode` sit far below the DWARF game range).
  - Bodies: definitions found at brace depth 0, brace-matched; regex extracts callee names, string literals and
    float literals.
  - Virtual slot order: `include/types/*.h` comments `virtual ... Name(...); // 0xADDR (vtable +0xNN`.

## Matching (anchor and propagate)

1. Seeds
   - name: a Burnout 3 name in `symbol_addrs.txt` (MW-mangled names demangled to `Class::method`) equal to a
     Burnout 2 name.
   - str / const: a string or `.lit4` float used by exactly one game function on each side.
2. Order: around each pair, align the Burnout 3 unit's functions with the Burnout 2 file's functions in link order
   (Needleman–Wunsch, free end gaps, score from size ratio). Aligned pairs within ±50% size are proposed.
3. Vtable: Burnout 3 vtables are runs of function-start words in 0x4DDAA0–0x4E0680 (slot 0 = first word). If a run
   holds a paired function at slot k and its Burnout 2 class has that method at slot k, the other slots pair up.
4. Calls: for a pair, Burnout 3 `jal` callees are paired with Burnout 2 callee names when exactly one unpaired
   candidate fits (size within ±50%).
5. Repeat 2–4 until nothing new. Only pairs of medium or higher confidence propagate.

Confidence: `high` = name seed, or ≥2 independent kinds of evidence (order, vtable, call, str, const); `medium` = one
kind and size within ±25%; `low` = otherwise. A Burnout 3 function with two different Burnout 2 candidates (or the
reverse) is a conflict: both rows drop to `low` and are flagged.

## Output and CLI

- `python3 tools/bo2map.py --bo2 PATH` writes `build/bo2map.tsv` (gitignored): `bo3_addr bo3_name bo3_unit size3
  bo2_name bo2_file size2 confidence evidence`, and prints counts per confidence and units covered.
- `--unit UNIT` prints that unit's rows plus the Burnout 2 source and header paths for the matcher.
- `--eval` holds out each name-seeded unit in turn and reports how many of its seeds are recovered from the rest.

## Testing and success

- `tools/test_bo2map.py`: synthetic fixtures for alignment, vtable alignment, call propagation, conflict demotion,
  demangling and body extraction.
- Success: `--eval` recovers held-out D4 seeds; `--unit unit_00134570` names `CGameMode`'s methods; a 20-row Ghidra
  spot check of `high` rows finds ≥18 correct; a full run takes under a minute on the host.

## Out of scope

Compiling Burnout 2 and diffing asm; writing names automatically; vendor libraries (rwsdk etc.).
