#!/bin/bash
# Run test suite: compile → assemble → link → run on Z8000 emulator → check R0
# Each test_*.c is compiled, linked with crt0.s, run on the emulator,
# and the R0 return value is compared with the .expected file.

SUITEDIR="$(cd "$(dirname "$0")" && pwd)"
source "$SUITEDIR/../common.sh"

# Build emulator if needed
if [ ! -x "$EMU" ]; then
    echo "Building Z8000 emulator..."
    make -C "$CROSS/z8000_emu" 2>&1 | tail -1
    if [ ! -x "$EMU" ]; then
        echo "FAIL: could not build emulator"
        fail=1
        print_summary
        exit 1
    fi
fi

# Assemble and convert crt0 once
CRT0_S="$SUITEDIR/crt0.s"
CRT0_OBJ="$SUITEDIR/output/crt0.obj"
CRT0_OUT="$SUITEDIR/output/crt0.out"

assemble "$CRT0_S" "$CRT0_OBJ"
if [ ! -f "$CRT0_OBJ" ]; then
    echo "FAIL: crt0.s assembly failed"
    echo "$asm_errors"
    fail=1
    print_summary
    exit 1
fi

obj_to_xout "$CRT0_OBJ" "$CRT0_OUT" >/dev/null 2>&1
if [ ! -f "$CRT0_OUT" ]; then
    echo "FAIL: crt0 xcon failed"
    fail=1
    print_summary
    exit 1
fi

for src in "$SUITEDIR"/test_*.c; do
    name=$(basename "$src" .c)
    expected_file="$SUITEDIR/${name}.expected"

    if [ ! -f "$expected_file" ]; then
        echo "SKIP $name (no .expected file)"
        skip=$((skip+1))
        continue
    fi

    expected=$(cat "$expected_file" | tr -d '[:space:]')

    out_s="$SUITEDIR/output/${name}.s"
    out_obj="$SUITEDIR/output/${name}.obj"
    out_xout="$SUITEDIR/output/${name}.out"
    out_linked="$SUITEDIR/output/${name}.linked.out"
    out_bin="$SUITEDIR/output/${name}.bin"

    # Step 1: compile to assembly
    compile_to_asm "$src" "$out_s"
    if [ ! -f "$out_s" ]; then
        echo "FAIL $name (compile failed: $compile_errors)"
        fail=$((fail+1))
        continue
    fi

    # Step 2: assemble
    assemble "$out_s" "$out_obj"
    if [ ! -f "$out_obj" ]; then
        echo "FAIL $name (assemble failed)"
        fail=$((fail+1))
        continue
    fi

    # Step 3: convert to x.out
    obj_to_xout "$out_obj" "$out_xout" >/dev/null 2>&1
    if [ ! -f "$out_xout" ]; then
        echo "FAIL $name (xcon failed)"
        fail=$((fail+1))
        continue
    fi

    # Step 4: link with crt0
    link_errors=$(link_xout "$out_linked" "$CRT0_OUT" "$out_xout")
    if [ ! -f "$out_linked" ]; then
        echo "FAIL $name (link failed: $link_errors)"
        fail=$((fail+1))
        continue
    fi

    # Step 5: convert to flat binary
    conv_errors=$(xout_to_bin "$out_linked" "$out_bin")
    if [ ! -f "$out_bin" ]; then
        echo "FAIL $name (xout2bin failed: $conv_errors)"
        fail=$((fail+1))
        continue
    fi

    # Step 6: run on emulator
    run_emu "$out_bin"
    emu_rc=$?

    if [ -z "$emu_r0" ]; then
        echo "FAIL $name (emulator: could not extract R0)"
        echo "$emu_output" > "$SUITEDIR/output/${name}.emu_log"
        fail=$((fail+1))
        continue
    fi

    # Step 7: compare R0 with expected
    if [ "$emu_r0" = "$expected" ]; then
        echo "PASS $name (R0=$emu_r0)"
        pass=$((pass+1))
    else
        echo "FAIL $name (expected=$expected, got R0=$emu_r0)"
        echo "$emu_output" > "$SUITEDIR/output/${name}.emu_log"
        fail=$((fail+1))
    fi
done

echo ""
echo "run: $pass passed, $fail failed, $skip skipped"
print_summary
