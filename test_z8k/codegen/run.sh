#!/bin/bash
# Codegen test suite: compile C to Z8002 assembly and diff against expected output
# Usage: ./run.sh [test_name.c]

SUITEDIR="$(cd "$(dirname "$0")" && pwd)"
source "$SUITEDIR/../common.sh"

run_one_test() {
    local base="$1"
    local name=$(basename "$base" .c)
    local tmpdir
    tmpdir=$(mktemp -d)

    # Compile to assembly
    compile_to_asm "$base" "$SUITEDIR/output/${name}.s" "$tmpdir"
    local rc=$?
    local errors="$compile_errors"

    # Check for "no code table" errors
    local no_code=$(echo "$errors" | grep -c "no code table")

    # Verdict
    if [ $rc -gt 1 ]; then
        echo "FAIL $name (exit code $rc — crash)"
        fail=$((fail+1))
    elif [ "$no_code" -gt 0 ]; then
        echo "SKIP $name ($no_code 'no code table' errors)"
        skip=$((skip+1))
    elif [ ! -f "$SUITEDIR/expected/${name}.s" ]; then
        echo "SKIP $name (no expected output)"
        skip=$((skip+1))
    else
        local d
        d=$(diff "$SUITEDIR/expected/${name}.s" "$SUITEDIR/output/${name}.s")
        if [ -z "$d" ]; then
            echo "PASS $name"
            pass=$((pass+1))
        else
            echo "FAIL $name"
            echo "$d" | head -20
            fail=$((fail+1))
        fi
    fi

    # Keep errors for inspection
    [ -n "$errors" ] && echo "$errors" > "$SUITEDIR/output/${name}.err" 2>/dev/null

    rm -rf "$tmpdir"
}

if [ -n "$1" ]; then
    run_one_test "$SUITEDIR/$1"
else
    for t in "$SUITEDIR"/test_*.c; do
        run_one_test "$t"
    done
fi

echo ""
echo "codegen: $pass passed, $fail failed, $skip skipped"
print_summary
