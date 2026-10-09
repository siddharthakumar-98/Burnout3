# Burnout3_decomp

**A matching decompilation of Burnout 3: Takedown (PS2, NTSC-U, SLUS-21050).**

The goal is to reconstruct **Burnout 3's game code** as C/C++ that CodeWarrior PS2 3.0.1 build 119 compiles to the
original bytes. The complete `SLUS_210.50` still builds byte-identically, with vendor libraries preserved in assembly.
The game is C++ on top of RenderWare 3.6, Sony libsce, EA DirtySock and Logitech device libraries. Use
[existing source and decompilation references](docs/library-references.md) to recover their interfaces and guide
useful matches; full vendor-library reconstruction is outside the game-source completion gate. Game assets are
never in this repo. The rebuilt ELF runs in PCSX2 with your own disc providing them.

> **Status:** D4 (core infrastructure) done with a tail of 6; D5 (main loop/game flow) done under the roadmap's
> tail policy with [10 unmatched functions](docs/d5.md#recorded-tail). D6 vehicle physics is [in progress](docs/d6.md). The compiler is identified (CodeWarrior 3.0.1 build 119,
> `-O4 -str readonly -Cpp_exceptions off`), and 94 functions in 36 C/C++ files compile to the original bytes and are
> linked in place of their assembly. The binary is mapped: libraries, VU microcode and every section are fenced off,
> the game progress category contains 400 units including C carves and static initializers. Units have assembly
> data slices; text carves can share their original unit's data. The current report has **141 / 5,654 game functions**
> matched (2.49%), covering **21,304 / 2,674,804 code bytes** (0.80%). [PROGRESS.md](PROGRESS.md) and the visual map
> track only game code; preserved dependencies are outside their denominator. Burnout 2's
> DWARF-named decomp has counterparts for Burnout 3's core code ([docs/burnout2.md](docs/burnout2.md)); the pool,
> file system and load queue now carry Criterion's names (`GtLList`, `CGTFileSystem`, `CAsyncLoadManager`). The full build still
> reproduces the original SHA-1; boot/race behavior was verified in the earlier PCSX2 checks. See [../ROADMAP.md](../ROADMAP.md) for
> milestones and [docs/layout.md](docs/layout.md) for the memory layout.

## Supported build

| Game | Region | Serial | ELF SHA-1 |
|---|---|---|---|
| Burnout 3: Takedown | NTSC-U | SLUS-21050 (`VER 1.00`) | `332be40d6081b8b5055a6ea01194ad6ff662a863` |

## Requirements

- Docker (the build runs in a linux/amd64 image, through Rosetta on Apple Silicon)
- Python 3, for the optional native venv with splat/spimdisasm
- Rust, to run the ISO extractor in `../Burnout3_rust`
- Your own dump of the game
- The CodeWarrior PS2 compiler, **3.0.1 build 119**, which you supply in `compilers/3.0.1b119-040914/` (never
  committed). It is the decomp.me build `3.0.1b119-040914`. [docs/compiler.md](docs/compiler.md) explains why this build was
  chosen.

## Building

1. Extract the ELF from your disc into `orig/`:

   ```bash
   cd ../Burnout3_rust && cargo run --release -p iso_extract -- extract ~/Desktop/ps2_games/"Burnout 3 - Takedown (USA).iso" ../Burnout3_decomp/orig --only SLUS_210
   ```

2. Build the image once:

   ```bash
   docker build --platform linux/amd64 -t b3-build -f docker/Dockerfile .
   ```

3. Configure. This checks your ELF's hash, splits it with splat into `assembly/asm/` and writes `build.ninja`:

   ```bash
   tools/dock python3 configure.py
   ```

4. Build:

   ```bash
   tools/dock ninja
   ```

The last step prints `build/SLUS_210.50: 332be40d… OK` when the output matches. Use `configure.py --no-split` to
regenerate `build.ninja` without re-running splat.

C units need the compiler in `compilers/3.0.1b119-040914/` (see Requirements). `tools/dock ninja` compiles each
`c_cpp/src/**/*.c` listed in `C_UNITS` in `configure.py`, and links it in place of its assembly once it's marked as
matching. To compare a C unit against the original, run `tools/dock objdiff-cli report generate -o build/report.json`,
or open `tools/bin/objdiff` in this folder.

To boot the rebuilt game, start PCSX2 with your ISO inserted and the rebuilt ELF swapped in:

```bash
/Applications/PCSX2-v2.4.0.app/Contents/MacOS/PCSX2 -elf ~/Desktop/Burnout3/Burnout3_decomp/build/SLUS_210.50 -- ~/Desktop/ps2_games/"Burnout 3 - Takedown (USA).iso"
```

Use `build/SLUS_210.50` (no extension). `build/SLUS_210.50.elf` is an unfinished intermediate file. Starting an ELF
from PCSX2's menu boots without a disc, and the game stalls on a black screen because it loads its modules from the
disc at startup.

## Layout

The decomp has two sides plus shared files at the top level.

| Path | What it is | Committed |
|---|---|---|
| **`assembly/`** | The assembly side ([README](assembly/README.md)) | |
| `assembly/splat/b3.yaml` | Split of the load segment into code, data, VU microcode and carved-out units | yes |
| `assembly/include/` | Assembler macros | yes |
| `assembly/asm/`, `assembly/assets/` | splat output, regenerated from your ELF | **no** |
| **`c_cpp/`** | The C/C++ side ([README](c_cpp/README.md)) | |
| `c_cpp/src/` | Decompiled C/C++, each file replacing the assembly unit with the same path | yes |
| `c_cpp/include/` | Shared headers | yes |
| **Shared** | | |
| `configure.py` | One combined build: splits, assembles, compiles, links, checks the SHA-1. Lists C units and compiler flags. | yes |
| `config/symbol_addrs.txt` | Symbol names | yes |
| `config/reloc_addrs.txt` | Relocation overrides (offsets the disassembler mistook for labels) | yes |
| `config/linker_extra.ld` | Extra linker script, including the offsets used by `reloc_addrs.txt` | yes |
| `tools/elf.py` | Extracts the load segment and rebuilds the exact ELF container | yes |
| `tools/funcmatch.py` | Compares C/C++ functions with the original: one function under chosen flags or compiler, every function in a file (`--all`), or one function across source variants (`--variants DIR`) | yes |
| `tools/litfix.py` | Points a compiled object's float literals at the original's pooled `.lit4` entries (run by the build) | yes |
| `tools/xref.py`, `tools/tusplit.py`, `tools/dataslice.py` | Map the binary: cross-references, translation-unit boundaries, per-unit data slices | yes |
| `tools/ghidra_sync.py` | Keeps the Ghidra project (via ghidra-mcp) in step with the repo's functions, names and units | yes |
| `tools/ghidra_read.py` | Read-only Ghidra queries (decompilation, callers/callees, refs, xrefs, memory) for matcher agents | yes |
| `tools/vdbhash.py` | `Data/vdb.xml` format and key hash; `check` resolves the code's key names against your own copy | yes |
| `tools/progress.py`, `tools/progress_map.py` | Write `PROGRESS.md`, the progress map `progress_map.svg` and its badge data `progress.json` from objdiff's report | yes |
| `tools/dock` | Runs a command in the build container | yes |
| `docker/Dockerfile` | Build image: binutils-mips-linux-gnu, wibo, objdiff-cli, splat | yes |
| `docs/` | Memory layout, compiler identification, reverse-engineering notes | yes |
| `orig/` | Your ELF and its raw load segment | **no** |
| `compilers/` | CodeWarrior binaries you supply | **no** |
| `build/` | Build output | **no** |
| `tools/bin/` | Downloaded helper tools (objdiff GUI, m2c) | **no** |

## Tooling for decompiling

- **objdiff:** run `tools/bin/objdiff` (macOS GUI) in this directory. It reads `objdiff.json`.
- **Ghidra 12.1.4** with ghidra-emotionengine-reloaded, installed via Homebrew (`ghidraRun`).
- **m2c:** `tools/bin/m2c/m2c.py` for first-draft C from a function's asm.
- **funcmatch:** `tools/dock python3 tools/funcmatch.py <function> <file.c> [-f=FLAGS ...] [-c compilers/<version>]`
  compiles one function, compares it with the original through objdiff, and shows an instruction diff when it
  doesn't match.

Run `tools/dock python3 tools/progress.py` after a successful build to regenerate the game-only progress table,
map and badge. The complete objdiff report retains every library category for diagnostics. Vendor-source reuse
does not add game progress or permit a nonmatching implementation to replace preserved assembly. Keep native
assembly and VU microcode as assembly, and leave the Rust rewrite deferred while this PS2 milestone is active.

## Legal

This directory publishes decompiled source, headers, symbol names and build configs only. It never includes the game
executable, disassembly, assets or compilers. You need your own legally obtained copy of the game. Burnout is a
trademark of Electronic Arts, and RenderWare is a trademark of Criterion Software. This is an unofficial fan project
with no affiliation to either.
