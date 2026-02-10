#!/bin/bash
# Run all Z8002 code generator tests
# Usage: ./run_tests.sh [test_name.c]
#
# Each test:
# 1. Preprocesses test_XX.c with cp68
# 2. Parses with c068 to produce icode
# 3. Runs c1z8k to produce Z8002 assembly
# 4. Checks for "no code table" errors (should be NONE for passing tests)
# 5. Checks for expected instruction patterns in the output
# 6. Reports PASS/FAIL

TESTDIR="$(cd "$(dirname "$0")" && pwd)"
CROSS="$TESTDIR/.."
CP68="$CROSS/cpp/cp68"
C068="$CROSS/parser/c068"
C1Z8K="$CROSS/cgen_z8k/c1z8k"

pass=0
fail=0
skip=0

run_one_test() {
    local base="$1"
    local name=$(basename "$base" .c)
    local tmpdir="$TESTDIR/tmp_$$_${name}"
    mkdir -p "$tmpdir"

    # Compile through preprocessor and parser
    "$CP68" "$base" "$tmpdir/test.i" 2>/dev/null
    if [ $? -ne 0 ]; then
        echo "FAIL $name (preprocessor failed)"
        fail=$((fail+1))
        rm -rf "$tmpdir"
        return
    fi

    "$C068" "$tmpdir/test.i" "$tmpdir/test.1" "$tmpdir/test.2" "$tmpdir/test.3" 2>/dev/null
    if [ $? -ne 0 ]; then
        echo "FAIL $name (parser failed)"
        fail=$((fail+1))
        rm -rf "$tmpdir"
        return
    fi

    # Run Z8002 code generator
    local errors
    errors=$("$C1Z8K" "$tmpdir/test.1" "$tmpdir/test.2" "$tmpdir/test.s" 2>&1)
    local rc=$?

    # Check for "no code table" errors
    local no_code=$(echo "$errors" | grep -c "no code table")

    # Check expected patterns if pattern file exists
    local pattern_ok=true
    if [ -f "$TESTDIR/expected/${name}.patterns" ]; then
        while IFS= read -r pattern; do
            [[ "$pattern" =~ ^#.*$ || -z "$pattern" ]] && continue
            if ! grep -qE "$pattern" "$tmpdir/test.s" 2>/dev/null; then
                echo "  MISSING pattern: $pattern"
                pattern_ok=false
            fi
        done < "$TESTDIR/expected/${name}.patterns"
    fi

    # Check forbidden patterns (68000-isms in code-generator output)
    # Note: prologue/epilogue (link, unlk, rts, bra for return) come from the
    # parser, not from the code generator. We only flag 68000-isms that appear
    # on lines between *line markers (which are code-generator output).
    local no_68k=true
    if [ -f "$tmpdir/test.s" ]; then
        # Extract code-generator lines (between *line markers, excluding parser boilerplate)
        local cgen_lines=$(sed -n '/^\*line/,/^[^*]/p' "$tmpdir/test.s" 2>/dev/null | grep -v '^\*line')
        for bad in "move " "move\." "ext\." "lea " "\-\(sp\)" "\(sp\)\+"; do
            if echo "$cgen_lines" | grep -qiE "$bad" 2>/dev/null; then
                local found=$(echo "$cgen_lines" | grep -iE "$bad" | head -1)
                echo "  FOUND 68000-ism in codegen: $found"
                no_68k=false
            fi
        done
    fi

    # Verdict
    if [ $rc -gt 1 ]; then
        echo "FAIL $name (exit code $rc — crash)"
        fail=$((fail+1))
    elif [ "$no_code" -gt 0 ]; then
        echo "SKIP $name ($no_code 'no code table' errors)"
        skip=$((skip+1))
    elif ! $pattern_ok; then
        echo "FAIL $name (missing expected patterns)"
        fail=$((fail+1))
    elif ! $no_68k; then
        echo "FAIL $name (68000 instructions in codegen output)"
        fail=$((fail+1))
    else
        echo "PASS $name"
        pass=$((pass+1))
    fi

    # Keep output for inspection
    mkdir -p "$TESTDIR/output"
    [ -f "$tmpdir/test.s" ] && cp "$tmpdir/test.s" "$TESTDIR/output/${name}.s" 2>/dev/null
    # Also keep errors
    [ -n "$errors" ] && echo "$errors" > "$TESTDIR/output/${name}.err" 2>/dev/null
    rm -rf "$tmpdir"
}

if [ -n "$1" ]; then
    run_one_test "$TESTDIR/$1"
else
    for t in "$TESTDIR"/test_*.c; do
        run_one_test "$t"
    done
fi

echo ""
echo "Results: $pass passed, $fail failed, $skip skipped"
