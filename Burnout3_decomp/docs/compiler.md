# Compiler identification (D2, revised in D4)

**Result:** Metrowerks CodeWarrior for PS2 **3.0.1 build 119** (decomp.me `mwcps2-3.0.1b119-040914`, dated
2004-09-14) with **`-O4 -str readonly -Cpp_exceptions off`**. D2 chose 3.0.3 from the version stamp and the first
ten test functions; D4's loops showed the 2003-2005 builds come closer, and a sweep of every function in C under all
19 `3.0.x` builds ([below](#build-119-d4)) puts build 119 first or equal first on every one. Both D2 near-misses
match with it. The two extra flags came from D3 and are [explained below](#flags-found-while-mapping-the-binary-d3).

## The version stamp narrows it to four builds

The game's `.comment` section reads `MW MIPS C Compiler (2.4.1.01)`. That is the code generator's version, and
several releases share it. Compiling the same file with every PS2 CodeWarrior build on decomp.me:

| Build | Stamp | Note |
|---|---|---|
| 2.3.3 (Sep 2000) | `2.3.1.01` | ruled out |
| **2.4 Engineering Build 0017** (Dec 2000) | `2.4.1.01` | candidate |
| **3.0** (Nov 2001) | `2.4.1.01` | candidate |
| **3.0.1** (Jan 2002) | `2.4.1.01` | candidate |
| **3.0.3** (Jul 2002) | `2.4.1.01` | candidate |
| 3.0 builds 38–52, 3.0.1 builds 44–210 (Mar 2003 – Mar 2006) | `3.0.0` | ruled out |

## Test functions decide between them

Each function was compiled at `-O3`, `-O4`, `-O4 -opt speed` and `-O4 -opt space` with `tools/funcmatch.py`. All
four flag sets gave the same result for every function, so the table shows one value per compiler.

| Function | What it tests | 2.4 EB0017 | 3.0 | 3.0.1 | 3.0.3 |
|---|---|---|---|---|---|
| `func_0013C910`, `func_0013C930` | leaf, two functions in one file | 100% | 100% | 100% | **100%** |
| `func_00131AA0` | leaf, array of structs | 100% | 100% | 100% | **100%** |
| `func_0014E830` | float loads and stores | 100% | 100% | 100% | **100%** |
| `func_0014E7E0` | float to int | 0% (calls `fptosi`) | 0% | 0% | **100%** (inline `cvt.w.s`) |
| `func_0014DD80` | float return with a branch | 100% | 100% | 100% | **100%** |
| `func_00136E00` | global through `$gp` | 100% | 100% | 100% | **100%** |
| `func_0013B740` | call with stack frame | 82% (`sq`/`lq` saves, `paddub` moves) | 100% | 100% | **100%** |
| `func_0014EC30` | switch with jump table | not tested | not tested | not tested | **100%** |
| `__ct__12CUnk0028B700Fv` | C++ constructor installing a vtable | not tested | not tested | not tested | **100%** |
| `func_00131CE0` | large struct offsets | 87.5% | 94.7% | 98.75% | 98.75% |
| `func_0013AE70` | call or return 0 | 70.8% | 90.4% | 90.4% | 90.4% |
| `func_0013B670` | clamp with `pmaxw`/`pminw` | 0% | 0% | 0% | 0% |

- **EB0017 is ruled out.** It keeps registers 128-bit, using `paddub` moves and `sq`/`lq` saves where the game
  uses `daddu` and `sd`/`ld`. `#pragma processor VR5000` fixes the moves but not the saves.
- **3.0 and 3.0.1 are ruled out.** They call the `fptosi` helper for float-to-int, where the game has an inline
  `cvt.w.s`.
- **3.0.3 matches everything that matches anywhere** among these four. The later builds, ruled out here by their
  stamp, turned out to match better still ([D4](#build-119-d4)).

## Open items

These were the D2 open items; build 119 ([D4](#build-119-d4)) settled the first three.

- **`-O3` vs `-O4`:** settled in D4 (`-O4`; `ustrToUtf8` separates them).
- **`func_0013AE70` (90% under every 2002 build):** the original leaves the conditional branch's delay slot empty
  and sets the return value in the next branch's slot. 100% under build 119 with the D2 source.
- **`func_00131CE0` (98.75%):** a single register choice (`v0` vs `at` for a large field offset). 100% under
  build 103 and later with the D2 source.
- **`func_0013B670`:** CodeWarrior never emits `pmaxw`/`pminw` from C (`MIN`/`MAX` macros and
  `#pragma conditional_move` both produce branches), and the trailing `pextlw` looks hand-written. The original is
  almost certainly an inline-asm clamp helper. CodeWarrior's `asm` blocks can express it
  ([below](#inline-assembly-d4)); it stays in assembly until it is rewritten that way.

## Flags found while mapping the binary (D3)

| Flag | Evidence |
|---|---|
| `-str readonly` | The game's string literals sit in `.rodata`, and even 6-byte ones (`"%s/%s"` at `0x4BB808`) are loaded with `lui`/`addiu`. By default CodeWarrior puts literals in `.data` and short ones in `.sdata`, loaded through `$gp`, which changes the code. |
| `-Cpp_exceptions off` | The binary has no `.exceptix` exception tables, which CodeWarrior emits for any function with destructors when exceptions are on. The code was identical in the cases tested; the flag keeps C++ units from emitting a section the original doesn't have. |

All D2 results above are unchanged with the two flags.

## Small data and float literals (D3)

Test compiles with these flags show how CodeWarrior lays out the data that the `$gp` area holds, which is what the
small-data map in [layout.md](layout.md#small-data-and-the-lit4-pool) rests on:

| Source | Section | Access |
|---|---|---|
| Initialized global or `static` of 8 bytes or less (`float g = 2.5f;`, `double`, `char[8]`) | `.sdata` | `$gp`-relative, `R_MIPS_GPREL16` |
| Uninitialized, or initialized to zero, 8 bytes or less (`int x;`, `int x = 0;`, `long long`) | `.sbss` | `$gp`-relative |
| Larger objects | `.data` / `.bss` | `lui`/`addiu` |
| Float constant that `lui` can't build (`0.85f`, `FLT_MAX`, a `static const float`) | `.lit4`, pooled per file | `lwc1 $fN, 0($gp)`, `R_MIPS_LITERAL` |
| `1.0f`, `0.5f`, `4.5f` (low 16 bits zero) | none | `lui` + `mtc1` |
| Global `const float` | `.rodata` | `lui`/`lwc1` |

Every variable and every literal gets a section of its own, and the linker decides the final order. The game's
linker kept `.sdata`, `.sbss` and `.bss` in link order but pooled `.lit4` across files, so C units keep loading the
original pooled literals: `tools/litfix.py` verifies their four-byte values against the original function's pool
loads and retargets their `R_MIPS_LITERAL` relocations after each compile. Draft instruction positions may differ;
an unverified value is left unchanged with a warning (D5 regression tests cover PAL/NTSC values and addends). Double
arithmetic goes through soft-float helpers (`dpmul`, `dptoli`, …), and the game has no `.lit8` area.

## Inline assembly (D4)

CodeWarrior (3.0.3 and build 119 alike) compiles MW-style inline assembly, including VU0 macro instructions, in two forms:

```c
void zero(register float *p)
{
    asm {
        vsub.xyzw vf1, vf0, vf0
        sqc2 vf1, 0(p)      /* C variables can be operands */
    }
}

asm void zero_asm(register float *p)
{
    vsub.xyzw vf1, vf0, vf0
    sqc2 vf1, 0(a0)
    jr ra
    nop
}
```

Instructions in an `asm` block are scheduled with the surrounding code (in the first example the `sqc2` lands in the
`jr` delay slot). GCC-style `asm("..." : : "r"(p))` is rejected. `__attribute__((aligned(16)))` works on a typedef
(struct or `float[4]`); `__declspec(align(16))` fails, and a plain struct of floats is only 4-aligned. This is what
VU0-heavy game code and the `pmaxw`/`pminw` clamp in `func_0013B670` need.

**VU0 helpers (D4 open question 1, settled 2026-10-08).** The first two VU0 functions (`func_00130BF0`, sets `w` to 0;
`func_001AB010`, clears `xyz` of five vectors) match at 100% both as whole `asm` functions and as C calling
`static inline` helpers from `include/vu0.h` (`Vec4`, `vu0SetW`, `vu0ZeroXYZ`), each helper one MW `asm { }` block on
`register` pointer and float parameters. The build uses the helpers: they read like the inline vector methods the
original used, and the compiler still schedules their instructions into the caller (`daddu v0,a0,zero` lands between
the VU ops, `sqc2` fills the `jr` delay slot). Each call emits its own block, so repeated operations stay repeated;
a `for` loop over them is not unrolled (0%). A float can't be a `qmtc2` operand directly: the compiler silently
emits the wrong GPR (`qmtc2 a0`). Move it with `mfc1 v0, w` first, which also reproduces the original's folded
`daddu v0,zero,zero` and hazard `nop` for a constant. No mwccps2 VU0 builtins are known. Whole `asm` functions stay
for code no C shape reproduces.

## Flag shakedown (D4)

`func_0027A900` (a D3 test, 74.38%) differs only in instruction order: the original loads a `.lit4` literal before a
store that ours puts first. No optimizer setting changes that: `-O3`, `-O4`, `-O4,s` and `-opt level=4,nointrinsics`
all give 74.38%, while `-O2` (54.38%, no scheduling) and `-O4,p` (67.50%) are further off. So it is a question of the
source, to settle once its callers are understood. CodeWarrior has no separate scheduling switch: scheduling comes
with `-opt level=3` and above. `-O3` and `-O4` still produce the same code for everything tried.

**Settled by build 119 (2026-10-08):** the plain source (store `value`, then `0.85f * value`, then
`value * D_004E1AE4`) matches at 100% under b119 with the default flags, as do three of the seven variants tried under
3.0.3; the literal-before-store order is b119's scheduler, not the source.

## A later build? (D4)

`ustrFromUtf8` (`0x213BE0`, a `ConvertUTF.c`-style loop) keeps its loop test at the top, with the second condition
as an assembler pseudo-branch (`slti $at`; `bnez $at`) and both delay slots empty. 3.0.3 rotates the loop or fills
the slots with every source shape tried (11 variants, all flag sets: `-O3`, `-O4`, `-O4,s`, `-O4,p` give 70.9% for the
`while (*src && size >= 2)` form). Compiled with every build on decomp.me, the 2003-2005 `3.0.1` builds come much
closer, and **3.0.1 build 119 (2004-09-14) reaches 99.8%**: only the register of that one comparison differs (`v1`
instead of `$at`). Those builds were ruled out above only by their `3.0.0` version stamp, but the stamp in the
game's `.comment` may come from a prebuilt library object (the MW runtime is built with an older compiler), and
Burnout 3 shipped in September 2004. `func_0013AE70`'s empty delay slot (above) is the same symptom. To settle next:
run every matched function and the D2 tests through build 119 and its neighbours (b103, b145), and look for a build
or flag that gives `$at` there.

## Build 119 (D4)

`build/sweep/sweep.py` (a scratch script, not in the repo) compiled every function in C, the D2 and D3 tests and the
14 of `d4/ustring`, with each of the 19 `3.0.x` builds on decomp.me and compared it with `tools/funcmatch.py`'s
method. The source was the version tuned for 3.0.3. Selected columns (match %, default flags):

| Function | 3.0.1 (2002) | 3.0.3 | b95 | b103 | **b119** | b145 | b198 |
|---|---|---|---|---|---|---|---|
| `func_00131CE0` | 98.8 | 98.8 | 100 | 100 | **100** | 100 | 92.5 |
| `func_0013AE70` | 90.4 | 90.4 | 90.4 | 90.4 | **100** | 100 | 100 |
| `func_0014E7E0` (float to int) | 0 | 100 | 87.8 | 100 | **100** | 100 | 100 |
| `ustrncat` | 89.7 | 100 | 100 | 100 | **100** | 81.4 | 81.4 |
| `ustrncpy` | 100 | 100 | 100 | 100 | **100** | 74.3 | 74.3 |
| `ustrFromIntNoSep` | 64.9 | 67.7 | 98.4 | 98.4 | **99.5** | 92.3 | 60.3 |
| `ustrFromUtf8` | 69.9 | 69.9 | 92.1 | 92.1 | **96.5** | 92.1 | 59.6 |
| `ustrToUtf8` | 43.5 | 43.5 | 96.9 | 96.9 | **99.9** | 99.9 | 53.4 |

- Every function linked from C so far is also 100% under build 119, and the full build with it reproduces the SHA-1.
  `func_00131CE0` (the `v0`/`at` register choice) and `func_0013AE70` (the empty delay slot) now match and are
  linked.
- Build 119 is the only build at or above every other build on every function. Its neighbours bracket it: b103
  (2004-05) loses `func_0013AE70`, b145 (2005-02) loses `ustrncat` and `ustrncpy`. Burnout 3 went gold in
  August 2004, so the exact build was probably between b103 and b119; b119 is the closest one available.
- The game's `MW MIPS C Compiler (2.4.1.01)` stamp therefore does not come from the game's own objects (b119 stamps
  `3.0.0`); a prebuilt library object, most likely the Metrowerks runtime built with an older compiler, supplied it.
- **`-O3` vs `-O4` is settled:** `ustrToUtf8` gives 76.8% at `-O3` and 99.9% at `-O4`. `-O4,s` is identical to `-O4`
  everywhere; `-O4,p` is far off. The build uses `-O4`.
- `func_0014EC30` reports 99.7% under every build because objdiff compares its jump-table relocation by symbol;
  it links byte-identical.
- **Small `switch` order:** a `switch` too small for a jump table becomes compares in *reverse* source order, with
  the case bodies laid out in source order. The memory manager's switches (`memMgrRelease`, `memMgrTake`) compare
  4, 0x1A, 0x19, 0x18, 0x17, so the source lists `case 0x17` … `case 0x1A`, then `case 4`; with `case 4` first they
  scored 10–34%. Read the compare chain backwards to get the case order. This holds for sparse switches too: 36 cases spread over 0–99 returning six
  values (`func_003E8C20`) still become a compare chain, not a jump table, and match once the case groups are in the
  order the reversed chain implies (`default` returns the value whose block falls through).
- **Function pointers:** calling through a cast function pointer (`((void (*)(…))f)(…)`) gives `lui/addiu/jalr`, not
  `jal`; declare the callee with the right prototype instead.

## Not everything is CodeWarrior

Only the game was built with this compiler. Sony's libraries, RenderWare 3.6, EA DirtySock and the Logitech
libraries were built with ee-gcc: their functions are 8-byte aligned and use branch-likely instructions, which
CodeWarrior never does (see [layout.md](layout.md)). The Metrowerks C++ runtime is CodeWarrior code but saves
registers with 128-bit `sq`/`lq` like 2.4 EB0017, so it was prebuilt with an older compiler. RenderWare Audio's
EE side is CodeWarrior code inside the game ranges; whether it used the game's compiler and flags is untested. Matching those
libraries in D11 needs their own compilers.

## Reproducing

All builds above can be fetched from https://github.com/decompme/compilers/releases into `compilers/<version>/`
(gitignored), then compared with:

```bash
tools/dock python3 tools/funcmatch.py func_0013B740 c_cpp/src/d2/func_0013B740.c -c compilers/3.0.1b119-040914 -f=-O4
```
