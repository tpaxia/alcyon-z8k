# Alcyon C Cross-Compiler for Z8002 (CP/M-8000)

A complete C cross-compilation toolchain targeting the Zilog Z8002
(non-segmented mode) and CP/M-8000, built from the original Digital Research
Alcyon C compiler.

Successfully compiles and runs C programs on CP/M-8000.

## Pipeline

```
cp68 source.c source.i          # C preprocessor
c068 source.i s.1 s.2 s.3       # C parser (icode output)
c1z8k s.1 s.2 source.s          # Z8002 code generator
asz8k -l source.s               # Z8000 assembler -> .obj
xcon -o source.out source.obj   # Convert to x.out format
ld8k -o program.z8k startup.out source.out [libs...]  # Linker
```

## Components

| Tool | Directory | Description | Origin |
|------|-----------|-------------|--------|
| cp68 | `cpp/` | C preprocessor | Alcyon (via th-otto/tos3x) |
| c068 | `parser/` | C parser, icode output | Alcyon (via th-otto/tos3x) |
| c1z8k | `cgen_z8k/` | Z8002 code generator | Retargeted from Alcyon 68K codegen |
| asz8k | `asz8k/` | Z8000 assembler (x.out object format) | CP/M-8000 source distribution |
| ld8k | `ld8k/` | Z8000 linker | CP/M-8000 source distribution |
| xcon | `ld8k/` | .obj to x.out converter | CP/M-8000 source distribution |
| xdump | `ld8k/` | x.out file dumper | CP/M-8000 source distribution |
| ar8k | `ld8k/` | x.out archive manager | CP/M-8000 source distribution |

Also included:
- `libcpm/` - CP/M C runtime library (stdio, malloc, string, math, etc.) with Z8K long arithmetic routines (lmul, ldiv, lrem) and startup code
- `cgen/` - Original 68000 code generator (c168) for reference
- `as/` - 68000 assembler (as68) for reference
- `link68/` - 68000 linker for reference
- `test_z8k/` - Code generator regression tests

## Building

```sh
make            # builds all tools
```

Or build individual tools:

```sh
make -C cpp         # cp68
make -C parser      # c068
make -C cgen_z8k    # c1z8k
make -C asz8k       # asz8k
make -C ld8k        # ld8k, xcon, xdump, ar8k
```

## Testing

```sh
bash test_z8k/run_tests.sh            # run all suites
bash test_z8k/run_tests.sh codegen    # codegen patterns only
bash test_z8k/run_tests.sh asm        # compile + assemble
bash test_z8k/run_tests.sh run        # end-to-end on Z8000 emulator
```

Three test suites:

- **codegen** — 24 tests: compile C to Z8002 assembly and check instruction patterns
- **asm** — reuses codegen sources: compile + assemble + convert to x.out
- **run** — compile, link, run on Z8000 emulator, check return value in R0

The `run` suite requires the `z8000_emu` submodule (`git submodule update --init`).

## Provenance

The code comes from three sources:

- **Alcyon C compiler** (cpp, parser, cgen, as, link68, optimize, util) -
  Copyright 1982-1983 Alcyon Corporation / Digital Research. Source obtained
  from [th-otto/tos3x](https://github.com/th-otto/tos3x), which includes
  bug fixes and portability patches for modern systems.

- **CP/M-8000 tools** (asz8k, ld8k, libcpm) - Copyright 1982-1983 Digital
  Research Inc. From the CP/M-8000 source distribution for Zilog Z8000.
  Modified for cross-compilation on POSIX systems (byte-swapping, fixed-width
  types, standard I/O).

- **Z8002 code generator** (cgen_z8k, long arithmetic library) - New work
  retargeting the Alcyon 68000 code generator to Z8002. See
  [cgen_z8k/README.md](cgen_z8k/README.md) for architecture details.

## Known Limitations

- Prologue/epilogue instructions are emitted by the parser in 68000 syntax;
  the code generator translates them to Z8002 but this is fragile.
- Long immediate operands (`ldl mem,#imm32`, `pushl @R15,#imm32`,
  `cpl mem,#imm32`) generate invalid Z8000 addressing modes.
- Float operations emit library calls but no Z8002 floating-point runtime
  exists yet.

## License

See [LICENSE](LICENSE) for details. CP/M-8000 components are under the DRDOS
Inc. permissive license (2022). New Z8002 work is under the MIT license.
