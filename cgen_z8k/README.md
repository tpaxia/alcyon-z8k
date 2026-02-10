# c1z8k - Alcyon C Code Generator for Zilog Z8002

This is a retarget of the Alcyon C code generator (`c168`) from Motorola 68000
to Zilog Z8002 (non-segmented mode). It is part of the Alcyon C compiler
pipeline originally used by Digital Research for CP/M-68K and Atari TOS
development.

The original 68000 code generator source is from
[th-otto/tos3x](https://github.com/th-otto/tos3x) (`alcyon/cgen/`).

## Architecture

The Alcyon code generator uses a table-driven skeleton expansion system.
The architecture cleanly separates target-independent and target-dependent code:

**Target-independent** (unchanged from 68000):
- `codegen.c` - Main code generation driver, tree walking
- `smatch.c` - Skeleton matching and macro expansion engine
- `canon.c` - Tree canonicalization
- `sucomp.c` - Sethi-Ullman complexity computation
- `tabl.c` - Table utilities
- `main.c` - Icode reader, program entry (program name changed)
- `interf.c` - Data directive output (section directives adjusted)
- `putexpr.c` - Debug expression printer

**Target-dependent** (rewritten for Z8002):
- `cgen.h` - Machine model: register definitions, output macros
- `icode.h` - `PTRSIZE=2` (Z8002 non-segmented = 16-bit pointers)
- `optab.c` - Z8002 mnemonics, operator dispatch tables, branch tables
- `util.c` - Addressing mode output, type suffixes, register formatting
- `cskel.h` - Skeleton macro byte definitions (added PSHL, CRPAIR)
- `cskels.c` - All 42 skeleton groups converted from 68000 to Z8002

## Z8002 vs 68000 Key Differences

| Aspect | 68000 | Z8002 |
|--------|-------|-------|
| Registers | D0-D7 + A0-A7 (split) | R0-R15 (uniform) |
| Pointer size | 32-bit | 16-bit (non-segmented) |
| Operand order | `op src,dst` | `op dst,src` |
| Size encoding | Suffix `.b`/`.w`/`.l` | Mnemonic `ldb`/`ld`/`ldl` |
| Register pairs | D0 is always 32-bit | RR0 = {R0(high), R1(low)} |
| Push/Pop | `move Rn,-(sp)` | `push @R15,Rn` |
| Frame pointer | A6 via `link`/`unlk` | R14 by convention |
| Sign extend | `ext.w` / `ext.l` | `extsb` / `exts` |
| Load address | `lea addr,An` | `lda Rn,addr` |
| Compare | `cmp src,dst` | `cp dst,src` |
| Branch | `beq`/`bgt`/`bra` | `jr eq,`/`jr gt,`/`jp t,` |

## Register Pair Handling

The most significant difference is register pair semantics. On the 68000,
data registers are uniformly 32-bit — a word load into D0 affects only the
lower 16 bits, and D0 always refers to the same physical register.

On Z8002, word registers (R0) and long register pairs (RR0) have different
names and semantics:
- `RR0` = {R0 (high 16 bits), R1 (low 16 bits)}
- `mult RR0,src` multiplies R1 by src, result in RR0
- `div RR0,src` divides RR0 by src, quotient in R1, remainder in R0

This required:
- New `CRPAIR` macro (138) to always output `RR%d` for mult/div instructions
- New `PSHL` macro (137) for explicit long pushes
- Fixed `OUTEXT`/`OUTUEXT` macros to copy value to odd register before extending
- Fixed `MODSWAP` to extract quotient for DIV (move R1->R0) vs no-op for MOD
- Added LONG->INT truncation in `outextend()` to extract low word (R1->R0)
- Replaced inline type-conversion skeletons with generic RIGHT+EXRL path

## Building

```
make -C cgen_z8k
```

Produces `c1z8k`, the Z8002 code generator. Used in the pipeline:

```
cp68 source.c → c068 → c1z8k → [assembler]
```

## Testing

```
cd test_z8k && bash run_tests.sh
```

Runs 11 progressive test programs checking for correct Z8002 assembly output
and absence of 68000-isms. Tests cover: empty functions, return values,
assignments, arithmetic, branches, function calls, multiply/divide, shifts,
compound assignment, post-increment, and type conversions.

## Skeleton Groups

All 42 skeleton groups from the 68000 code generator have been converted:

| Index | Name | Purpose |
|-------|------|---------|
| 1 | fe_eqop | Compound assign for effect (+=, -=, etc.) |
| 2 | fe_assign | Assignment for effect |
| 3 | fe_eqshft | Compound shift for effect |
| 4 | fe_eqxor | Compound XOR for effect |
| 5 | fe_eqaddr | Compound address op for effect |
| 6 | fc_fix | Condition code fixup |
| 7 | fc_rel | Relational compare and branch |
| 8 | fc_btst | Bit test |
| 9 | fs_op | Binary op to stack |
| 10 | fs_itl | Int to long on stack |
| 11 | fs_ld | Load to stack |
| 12 | fr_add | ADD/SUB/AND/OR to register |
| 13 | fr_mult | Multiply to register |
| 14 | fr_div | Divide to register |
| 15 | fr_shft | Shift to register |
| 16 | fr_xor | XOR to register |
| 17 | fr_neg | Negate/complement to register |
| 18 | fr_eqop | Compound assign to register |
| 19 | fr_postop | Post-increment/decrement |
| 20 | fr_assign | Assignment to register |
| 21-24 | fr_eq* | Compound mult/div/shift/xor |
| 25 | fr_call | Function call |
| 26-27 | fr_itl/lti | Int<->long conversion |
| 28 | fr_ld | Load to register |
| 29-31 | fr_eq*/fe_eq* | Compound address/not ops |
| 32-33 | fr_docast/fs_docast | Type cast |
| 34-37 | fr_ftol/ltof/ftoi/itof | Float conversions (library calls) |
| 38-40 | fe_eq* | Compound mult/div/mod for effect |
| 41 | fr_tochar | Truncate to char |
| 42 | fr_ldiv | Long divide |

## Runtime Library (`libcpm/`)

The code generator emits `call lmul`, `call ldiv`, `call lrem` for 32-bit
long integer operations via the OPCALL mechanism in `smatch.c`. These library
routines use Z8000 hardware instructions:

| Routine | Instruction | Operation |
|---------|-------------|-----------|
| `lmul`  | `MULTL RQ0, RR4` | 32×32 → 64-bit multiply, returns lower 32 bits in RR0 |
| `ldiv`  | `DIVL RQ0, RR4`  | 64÷32 → 32-bit quotient in RR0, remainder stored in `_ldivr` |
| `lrem`  | `DIVL RQ0, RR4`  | 64÷32 → 32-bit remainder directly in RR0 |

Source files: `libcpm/lmul.S`, `libcpm/ldiv.S`, `libcpm/lrem.S`

The Z8000 hardware multiply/divide makes these trivial (5-10 instructions each)
compared to the original 68000 versions which used manual 16×16 multiply loops.

Calling convention (matches code generator output):
- Arguments: two 32-bit longs on stack (SP+2 and SP+6, after return address)
- Return value: RR0 (R0=high word, R1=low word)
- `ldiv` also stores remainder in `_ldivr` (BSS) for `lrem` compatibility

All three routines have been tested on CP/M-8000 (Z8002 emulator) with
`LONGTEST.Z8K` — both direct hardware instruction tests and library call tests pass.

## Full Pipeline

```
cp68 source.c source.i          # C preprocessor
c068 source.i s.1 s.2 s.3       # Parser (icode output)
c1z8k s.1 s.2 source.s          # Code generator (this tool)
asz8k -l source.s                # Z8000 assembler → .obj
xcon -o source.out source.obj    # Convert to x.out format
ld8k -o program.z8k startup.out source.out [libs...]  # Linker
```

Successfully compiled and run on CP/M-8000: "Hello from Alcyon C on Z8002!"

## Known Limitations

- Prologue/epilogue (`link`, `unlk`, `rts`, `bra`) are emitted as literal
  assembly by the parser, not the code generator. These still use 68000
  mnemonics and need post-processing or parser modification.
- Some inline type-conversion skeletons for unsigned char operations
  (`ctasg0a`, `ctasg0b`) may produce incorrect register names when the
  parent node type differs from the operand type. These cases use the
  generic RIGHT+EXRL fallback path instead.
- Long immediate codegen bugs: `ldl mem,#imm32`, `pushl @R15,#imm32`,
  `cpl mem,#imm32` generate invalid Z8000 addressing modes. These
  prevent C programs from using long constants directly; hand-written
  assembly works around this.
- Float operations generate library calls which would need a Z8002
  floating-point runtime library.
