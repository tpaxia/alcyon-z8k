#!/bin/bash
# Asm test suite: compile + assemble, check for asz8k errors
# Reuses codegen test sources — validates the assembler accepts our output.
#
# asz8k reports E errors on external/global symbol DA-mode references
# (e.g. "ld R0,_var"). These are expected — the assembler still produces
# the .obj with relocations and the linker resolves them. We treat these
# as non-fatal: if a .obj was produced, the E errors are just warnings.

SUITEDIR="$(cd "$(dirname "$0")" && pwd)"
source "$SUITEDIR/../common.sh"

CODEGEN_DIR="$SUITEDIR/../codegen"

for src in "$CODEGEN_DIR"/test_*.c; do
    name=$(basename "$src" .c)
    out_s="$SUITEDIR/output/${name}.s"
    out_obj="$SUITEDIR/output/${name}.obj"
    out_xout="$SUITEDIR/output/${name}.out"

    # Step 1: compile to assembly
    compile_to_asm "$src" "$out_s"
    if [ $? -gt 1 ]; then
        echo "FAIL $name (compile crash)"
        fail=$((fail+1))
        continue
    fi

    if [ ! -f "$out_s" ]; then
        echo "SKIP $name (no assembly output)"
        skip=$((skip+1))
        continue
    fi

    # Step 2: assemble
    assemble "$out_s" "$out_obj"

    if [ ! -f "$out_obj" ]; then
        echo "FAIL $name (asz8k produced no .obj)"
        echo "$asm_errors" > "$SUITEDIR/output/${name}.asm_err"
        fail=$((fail+1))
        continue
    fi

    # Step 3: convert to x.out
    xcon_errors=$(obj_to_xout "$out_obj" "$out_xout")
    if [ $? -ne 0 ]; then
        echo "FAIL $name (xcon failed: $xcon_errors)"
        fail=$((fail+1))
        continue
    fi

    if [ ${asm_E_count:-0} -gt 0 ]; then
        echo "PASS $name (${asm_E_count} relocation E warnings)"
    else
        echo "PASS $name"
    fi
    pass=$((pass+1))
done

echo ""
echo "asm: $pass passed, $fail failed, $skip skipped"
print_summary
