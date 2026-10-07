# Compiler identification (D2)

**Result:** Metrowerks CodeWarrior for PS2 **Version 3.0.3** (decomp.me `mwcps2-3.0.3-020716`) with
**`-O4 -str readonly -Cpp_exceptions off`**. With these flags, 10 functions in 8 translation units compile to the
original bytes and link into a build with the original SHA-1. The two extra flags came from D3 and are
[explained below](#flags-found-while-mapping-the-binary-d3).

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
- **3.0.3 matches everything that matches anywhere.**

## Open items

- **`-O3` vs `-O4`, speed vs space:** no test function separates them yet (see [the D4 shakedown](#flag-shakedown-d4)).
  `configure.py` uses `-O4`. Functions with loops should settle it.
- **`func_0013AE70` (90%):** the original leaves the conditional branch's delay slot empty and sets the return
  value in the next branch's slot. Every 3.0.x build fills the first slot instead. Notably, EB0017 reproduces the
  original's branch layout, so this may point to a build between EB0017 and 3.0 that isn't on decomp.me, or to a
  source form not found yet. Eight source variants tried.
- **`func_00131CE0` (98.75%):** a single register choice. The original computes a large field offset in `v0`; ours
  uses `at`. Five source variants tried.
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
original pooled literals: `tools/litfix.py` retargets their `R_MIPS_LITERAL` relocations after each compile. Double
arithmetic goes through soft-float helpers (`dpmul`, `dptoli`, …), and the game has no `.lit8` area.

## Inline assembly (D4)

CodeWarrior 3.0.3 compiles MW-style inline assembly, including VU0 macro instructions, in two forms:

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
`jr` delay slot). GCC-style `asm("..." : : "r"(p))` and `__attribute__` are rejected. This is what VU0-heavy game code
and the `pmaxw`/`pminw` clamp in `func_0013B670` need.

## Flag shakedown (D4)

`func_0027A900` (a D3 test, 74.38%) differs only in instruction order: the original loads a `.lit4` literal before a
store that ours puts first. No optimizer setting changes that: `-O3`, `-O4`, `-O4,s` and `-opt level=4,nointrinsics`
all give 74.38%, while `-O2` (54.38%, no scheduling) and `-O4,p` (67.50%) are further off. So it is a question of the
source, to settle once its callers are understood. CodeWarrior has no separate scheduling switch: scheduling comes
with `-opt level=3` and above. `-O3` and `-O4` still produce the same code for everything tried.

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
tools/dock python3 tools/funcmatch.py func_0013B740 c_cpp/src/d2/func_0013B740.c -c compilers/3.0.3-020716 -f=-O4
```
