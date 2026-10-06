# SLUS_210.50 memory layout

The executable is a CodeWarrior (`MW MIPS C Compiler (2.4.1.01)`) ELF with **one merged PT_LOAD**. The linker kept no
section headers, so the boundaries below were recovered by hand. They are encoded in
[`../assembly/splat/b3.yaml`](../assembly/splat/b3.yaml), and the build reproduces the original SHA-1 with them.
`tools/xref.py` (function/data cross-references, compiler fingerprints, strings) regenerates the evidence.

ELF: entry `0x100008`, load segment file offset `0x100`, vaddr `0x100000`, filesz `0x3E2680`, memsz `0x1DCEA00`.
`.reginfo` gives `_gp = 0x4E8670`.

## Sections

| VRAM | Rom offset | Section | Evidence |
|---|---|---|---|
| `0x100000`–`0x469E00` | `0x000000` | `.text` (8,946 functions) | Valid R5900 code throughout. The last `jr $ra` is at `0x469D88`, then zero padding to a 0x80 boundary. |
| `0x469E00`–`0x483F00` | `0x369E00` | VU microcode in DMA/VIF chains | 27 microprograms, [below](#vu-microcode). Data ends at `0x483EA0`, then zero padding. |
| `0x483F00`–`0x4B1500` | `0x383F00` | `.data` | Starts with Sony library version tags (`PsIIlibgraph2800`, …). Code writes here; no `__sinit` writes. |
| `0x4B1500`–`0x4D3E00` | `0x3B1500` | `.rodata` | Starts with the MW runtime's `std::exception` RTTI. Holds strings, `const` tables, RTTI, jump tables, and `const` objects that `__sinit` fills in. |
| `0x4D3E00`–`0x4DD820` | `0x3D3E00` | `.init`: C++ static initializers (`__sinit_*`) | Code placed after data; reached only through `.ctor`. A test compile shows CodeWarrior emits `__sinit_*` into `.init`. |
| `0x4DD820`–`0x4DDAA0` | `0x3DD820` | `.ctor`: 158 pointers into `.init`, a null terminator and padding | |
| `0x4DDAA0`–`0x4E0680` | `0x3DDAA0` | `.vtables` | Entries are `{RTTI*, 0, methods…}`; the first is `std::exception`'s. A test compile shows CodeWarrior emits vtables into `.vtables`. |
| `0x4E0680`–`0x4E1400` | `0x3E0680` | `.lit4`: pooled float literals | Starts at `_gp - 0x7FF0`, the lowest `$gp`-relative access in the code. 843 words, all floats, only ever loaded (`lwc1`, a few `lw`), never written. Ends at `0x4E13AC`, then zero padding to a 0x80 boundary. [Below](#small-data-and-the-lit4-pool). |
| `0x4E1400`–`0x4E2680` | `0x3E1400` | `.sdata` | Initialized small globals: loaded and stored through `$gp`, holds pointers, integers, `-1` markers and float variables (`1.0f` 46 times). In link order. |
| `0x4E2680`–`0x4E3000` | — | `.sbss` (NOLOAD) | The last `$gp`-relative access is `0x4E2FDC`, then padding to a 0x80 boundary. (The one access at `0x4E8670` takes the address `_gp` itself.) In link order. |
| `0x4E3000`–`0x1ECEA00` | — | `.bss` (NOLOAD) | Reached with `lui`/`addiu` only, from `0x4E3000`. Mostly static pools. In link order, then the ee-gcc libraries' COMMON symbols from `0x1ECE340`. crt0 clears `0x4E2680`–`0x1ECEA00`. |

How `.data` and `.rodata` were told apart: every unit's `.data` piece comes before every unit's `.rodata` piece, so
code references form two separate, increasing streams. Only the `.rodata` stream is written by `__sinit` (const
objects with constructors), and it holds the D2 jump table and RenderWare's `$Id` strings, which CodeWarrior and GCC
both put in `.rodata`. The game's string literals are in `.rodata` too, which needs `-str readonly`
([compiler.md](compiler.md)).

## `.text` by library

Library code was built with ee-gcc and game code with CodeWarrior, and the two are told apart by codegen
fingerprints that hold across the whole binary:

- **CodeWarrior** starts every function on a 16-byte boundary (all 6,197 game functions) and doesn't emit
  branch-likely instructions (`beql`, `bnel`, `bgezl`…; only 2 game functions contain any, see below).
- **ee-gcc** aligns functions to 8 bytes, so about half of them start at an address ending in 8, and it uses
  branch-likely freely.
- Edges were then settled function by function: libraries never call game code, library data and strings sit
  inside the library's stretch of `.data`/`.rodata`, and functions listed in `.vtables` are CodeWarrior C++.

`python3 tools/xref.py compilers` prints the fingerprint regions. Units in order:

| VRAM | Unit | Category | Evidence |
|---|---|---|---|
| `0x100000` | `runtime/crt0` | runtime | `_start` (clears GPRs with `padduw`), `_exit`, syscall stubs, static-init caller |
| `0x100270` | `runtime/mwrt` | runtime | Metrowerks C++ runtime: `__construct_array`, `std::exception`/`bad_exception`. CodeWarrior-built, but with 128-bit `sq`/`lq` saves (a different build from the game's compiler). |
| `0x102380` | `sce/libgraph` | sce | `sceGs*` strings; called from RenderWare sky2 |
| `0x103BC8` | `sce/libdma` | sce | `libdma: sync timeout` |
| `0x104410` | `sce/libmpeg` | sce | `sceMpeg*` strings, macroblock decoder |
| `0x10B6C0` | `sce/libipu` | sce | IPU register access, called from libmpeg; its init uses the data after the `PsIIlibipu` tag |
| `0x10BA00` | `sce/libkernl` | sce | 138 syscall stubs, then libkernl (`## internel error in libkernl.a!`, TTY, TLB setup) |
| `0x117078` | `runtime/libm` | runtime | fdlibm double-precision math (constant tables of 1/3, …) |
| `0x11E3A0` | `sce/libcdvd` | sce | `Libcdvd bind err …` |
| `0x11FD00` | `runtime/libc` | runtime | newlib (`bug in vfprintf: bad base`, `ctype` table, `Infinity`) |
| `0x12BFA0` | `runtime/libgcc` | runtime | `__muldi3`, then the `__divdi3` family (each object carries its own 256-byte `__clz_tab`, at `0x4B3E80`, `0x4B3F80`, `0x4B4080`, `0x4B4180`), then fp-bit soft-float double routines |
| `0x12EB30` | `game/…` | game | `main` |
| `0x1D8260` | `rw/renderware` | rw | RenderWare 3.6: the first function uses `babinary.c`'s `.rodata`; 12 `$Id` strings run `babinary.c` … `p2heap.c`, then the PS2 sky2 driver |
| `0x211C30` | `game/…` | game | |
| `0x215660` | `sce/libpad2` | sce | `libpad2: buffer addr is not 64 byte align` |
| `0x215F18` | `sce/libdbc` | sce | `libdbc:` strings (`.rodata` from `0x4B9920`), `PsIIlibdbc` data; ends with a no-op `printf` |
| `0x216E40` | `game/…` | game | includes online code (`GtComm`, UPnP port mapping) |
| `0x2372F0` | `sce/libinsck` | sce | `PsIIlibinsck` data |
| `0x23A740` | `sce/libmrpc` | sce | first function using the data after `PsIIlibmrpc`; `sceSifMCallRpc` strings |
| `0x23AFD0` | `game/…` | game | |
| `0x241BC0` | `sce/libmc2` | sce | `libmc2:` strings, `Sony PS2 Memory Card Format`, `PsIIlibmc2` data |
| `0x2499D0` | `sce/netcnfif` | sce | `SceNetcnfifCallbackThread` (`.rodata` from `0x4BB580`, `.data` from `0x49BA00`) |
| `0x24ABB0` | `sce/libscf` | sce | reads `rom0:ROMVER`; uses the data after `PsIIlibscf` |
| `0x24B2A0` | `game/…` | game | includes RenderWare Audio, `0x290B10`–`0x2B52C0` (`rwa/`, see below) |
| `0x2C28C0` | `lg/lgcodec` | lg | Logitech headset audio: Speex (narrowband and sub-band CELP), G.723, µ-law (`lgCodecUlawEncode`) and liblgaud (`liblgaud version 1.10.001`); not split further |
| `0x2E2700` | `game/…` | game | |
| `0x304008` | `lg/lgkbm` | lg | Logitech USB keyboard/mouse (`LgKbM library version … May 18 2004`) |
| `0x306D90` | `game/…` | game | |
| `0x3D3158` | `sce/libnet` | sce | `insufficient buffer size in Libnet` |
| `0x3D46A0` | `game/…` | game | |
| `0x3FFE48` | `ea/dirtysock` | ea | EA DirtySock (`dirtyaddr`, `rpc-e:`, lobby API) |
| `0x41C900` | `game/…` | game | |
| `0x443860` | `lg/lgdev` | lg | Logitech wheels and force feedback (`liblgdev version 1.11.027, May 10 2004`) |
| `0x445300` | `game/…` | game | runs to the end of `.text` |

Two single functions inside game ranges (`0x2174A0`, `0x2FC530`) use branch-likely but sit between CodeWarrior
functions and are called from game code; they are probably game functions with inline asm.

One library was compiled with CodeWarrior inside the game ranges and so has no fingerprint: **RenderWare Audio**
(EE side), `0x290B10`–`0x2B52C0`, 701 functions (`rwa/`, its own progress category). It was found as a
*call-closed* region: none of its functions calls game code outside it, while 41 game functions call into it, and
it holds `RWA ERROR!`, `RwaStreamFormat…` and `RwaRPCTransfer` strings. The same test finds only two other closed
regions (`0x214610`, `0x35F0A0`, about 55 functions each), and game code calls those from 230 and 61 places, so they
are game utility modules.

## Game translation units

The game and RenderWare Audio ranges are split into **358 provisional translation units** (337 `game/unit_<VRAM>`
and 21 `rwa/unit_<VRAM>`; median 6 functions). The C units carved out of them (`d2/`, `d3/`) cut 9 in two, so
`b3.yaml` lists 367 game and RenderWare Audio pieces. The binary has no file names, so `tools/tusplit.py` infers
the boundaries and writes the `.text` block of `b3.yaml` (`python3 tools/tusplit.py --yaml`). It links functions
that must share a file and cuts where nothing links:

| Evidence | Why it holds |
|---|---|
| Shared string literals | File-local in CodeWarrior output; identical strings appear more than once. |
| Shared `.rodata`/`.data` items and `.sdata` float constants | Weaker: globals can be shared, so only links between functions at most 60 apart count. Float literals in `.lit4` are **not** evidence: the linker pools them across files ([below](#small-data-and-the-lit4-pool)). |
| `.rodata`, `.data`, `.sdata` and `.sbss` references out of link order | Those sections follow link order (items used only by nearby functions correlate with function order at 0.99, 0.99, 1.00 and 0.99), so an earlier function using a later address than a later function puts both in one file. `.bss` follows link order more loosely (0.80) and is not used for this. |
| Methods that appear in only one vtable | A class's own methods are defined in its file, and vtables are laid out in link order. |
| Calls to a helper used only nearby | The pattern of a file-local `static` function. |
| C++ static initializers | One per file and in link order through `.ctor`. 110 of the 158 locate reliably through the private data they share with game functions; consecutive ones must be in different files, which forces 26 cuts. |

Gaps that no link crosses become boundaries unless the piece after them carries no evidence of its own (no private
data, no `.sdata`/`.sbss`/`.bss` variable that only nearby functions use, no vtable entry); then boundaries that
contradict a shared string or an initializer are removed. Checks on the result: no string used by nearby functions
is split across units (0 of 88), no `.sdata` float constant is (0 of 25), and no initializer is (0 of 110).

**Provisional:** a unit may still hold several real files, or one file may be cut in two. Units will be merged or
split as decompiling uncovers static data and string order. Two units contain two initializer anchors and so must
hold at least two files each: `game/unit_0041D3E0` and `game/unit_0042C2A0`.

Before the `.lit4` pool was understood (the first D3 pass), shared float literals counted as same-file evidence
and the split had 377 units. Dropping that evidence and adding the small-data sections gives the 358 above.

## Data slices

`.data`, `.rodata`, `.sdata`, `.sbss` and `.bss` are cut into one slice per unit, named after the unit that owns
them (`assembly/asm/data/<unit>.data.s`, `.rodata.s`, `.sdata.s`, `.sbss.s`, `.bss.s`). `tools/dataslice.py` writes
that block of `b3.yaml` (`python3 tools/dataslice.py --yaml`):

- All five sections follow link order, so each is cut into consecutive slices in the units' `.text` order. A dynamic
  program picks the cuts that put the most code references inside their own unit's slice.
- Items no code references (RTTI, tables reached through pointers) go with the following unit when they start on
  an 8-byte boundary after the previous unit's last referenced item.
- Sony version tags (`PsIIlib…`) and RenderWare `$Id` strings start their library's data, so they are cut points
  even though no code references them.
- In `.data` and `.rodata` every cut is snapped to a 16-byte boundary at an item start: the generated assembly
  aligns items relative to the start of its own file and some need 16-byte alignment. Of the nearest boundaries
  before and after, the one that moves fewer referenced items across the cut wins. A few library slices therefore
  start one item away from their true start (libinsck's `.data` at `0x49B640` instead of its tag at `0x49B678`,
  libmc2's at `0x49B6D0` instead of its tag at `0x49B6D8`, libc's `.rodata` at `0x4B3560` instead of its `ctype` table at
  `0x4B3458`). `.sdata` slices only need 4-byte boundaries and `.sbss`/`.bss` slices none; `configure.py` gives
  each object the alignment of its start address.
- crt0's references don't count: it clears `.sbss`/`.bss` from their start address, which is not a variable of its
  own.
- C units' carved slices (the D2 jump table) are kept exactly so the C unit can replace them.

| Section | Slices | Code references landing in their own unit's slice |
|---|---|---|
| `.data` | 65 | 94.8% |
| `.rodata` | 229 | 96.3% |
| `.sdata` | 68 | 92.1% |
| `.sbss` | 75 | 83.2% |
| `.bss` | 87 | 23.3% |

Most of the rest are globals used by other files. `.bss` is dominated by them: a few early files (the one with
`main` among them) define state that code everywhere reads, so its slices are the coarsest.

## Small data and the `.lit4` pool

The `$gp` area starts at `_gp - 0x7FF0 = 0x4E0680` and holds four output sections, each starting on a 0x80
boundary like the other sections:

| VRAM | Section | Contents | Order |
|---|---|---|---|
| `0x4E0680`–`0x4E1400` | `.lit4` | float literals | first use, pooled across files: one piece, `lit4` |
| `0x4E1400`–`0x4E2680` | `.sdata` | initialized globals of 8 bytes or less | link order: per-unit slices |
| `0x4E2680`–`0x4E3000` | `.sbss` | zero-initialized globals of 8 bytes or less | link order: per-unit slices |
| `0x4E3000`–`0x1ECEA00` | `.bss` | larger zero-initialized data, then COMMON | link order, then COMMON: per-unit slices and `common` |

**`.lit4`.** A test compile shows that CodeWarrior puts every float constant it can't build with `lui` (`0.85f`,
`FLT_MAX`, `static const float` values too) in a `.lit4` section of its own and loads it with
`lwc1 $fN, 0($gp)` under an `R_MIPS_LITERAL` relocation; `1.0f` or `0.5f` are built with `lui`/`mtc1` instead.
The game's `.lit4` area holds 843 such words (570 distinct values) and is shared: RenderWare (built with GCC) and
game code load the same `0.05f` at `0x4E0688`, and `FLT_MAX` at `0x4E0C14` is loaded by 38 units. Inside one
function, literals not seen before take consecutive addresses in instruction order, so the linker evidently pooled
literals by value in order of first use while relocating, deduplicating only partly (`0.001f` appears ten times).
That order can't be reproduced from objects, so `.lit4` stays one shared piece and C units keep using it:
`tools/litfix.py` (run by the build after every compile, and by `funcmatch.py`) retargets each `R_MIPS_LITERAL`
relocation at the label the original instruction at the same address loads (`%gp_rel(D_004E0D80)`), turning it into
an ordinary `R_MIPS_GPREL16`. The object's own `.lit4` copies are then unreferenced and discarded.
`d3/func_002527F0` and `d3/func_003EA7E0` prove it: both load a pooled literal and are linked from C.

**`.sdata`/`.sbss`.** CodeWarrior emits each small variable in a section of its own (`.sdata` when initialized,
`.sbss` when not, even `int x = 0;`), and the linker keeps them in link order: items used only by nearby
functions correlate with function order at 1.00 (`.sdata`) and 0.99 (`.sbss`). `1.0f` appears 46 times in `.sdata` because those are float
variables, not literals.

**`.bss`.** Also in link order: the libraries' `.bss` comes first (MW runtime, libdma, libmpeg, libkernl, libcdvd),
and RenderWare's sits among the game units on either side of its code. Many items are globals
used far from their file, so the order is noisier (0.80). The last `0x6C0` bytes, from `0x1ECE340`, are COMMON
symbols of newlib (`0x1ECE340`, used by libc) and lgkbm, which the linker allocates after all sections; they form
the `common` piece.

## VU microcode

One splat piece per microprogram (`vu/rw_<VRAM>`, `vu/game_<VRAM>`, symbols `vu_rw_*`/`vu_game_*`). Each starts
with a DMA tag (`ret` or `end`) followed by VIF `MPG` uploads, and code passes its address to the DMA.

| VRAM | Owner | Referenced from |
|---|---|---|
| `0x469E00`, `0x46A000`, `0x46A270`, `0x46A3D0`, `0x46A560` | RenderWare sky2 | pipeline tables at `0x4B9474`–`0x4B9488` and `0x49AD80` |
| `0x46A590` … `0x4839E0` (22 programs) | game | one game function each |

## Data inside libkernl's `.data`

libkernl carries kernel patches as code stored in `.data` (linked to run at `0x8007xxxx` and installed through
syscalls `0x5A`/`0x5B`): about `0x484240`–`0x484500`, `0x4848C0`–`0x484F40` and `0x485080`–`0x485740`, with their
install tables at `0x485040` and `0x4857C8`. They stay data.

## Open items

- Refine the provisional game units and their data slices as decompiling uncovers files (above). Allowing
  8-byte-aligned `.data`/`.rodata` slice starts would need slices whose assembly contains no 16-byte `.align`.
- `.init`: the 158 static initializers are one piece (`sinit`). Each belongs to a file; 110 are located, the rest
  would need their units refined first.
- `lg/lgcodec` bundles Speex, G.723, µ-law and liblgaud, and the large libraries (libkernl, RenderWare, DirtySock)
  are one unit each; splitting them into their object files is D11 work.
- 116 symbols still resolve to absolute addresses (`build/undefined_syms_auto.txt`): hardware registers and masks
  (which should stay constants), `_end` (`0x1ECEA00`), and references into the middle of functions or strings. The
  last two must become real symbols before the shift build ([../../ROADMAP.md](../../ROADMAP.md#2-the-build-is-genuinely-relinkable))
  can pass.
