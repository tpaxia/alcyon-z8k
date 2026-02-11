#!/bin/bash
# Codegen test suite: compile C to Z8002 assembly and check patterns
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

    # Check expected patterns if pattern file exists
    local pattern_ok=true
    if [ -f "$SUITEDIR/expected/${name}.patterns" ]; then
        while IFS= read -r pattern; do
            [[ "$pattern" =~ ^#.*$ || -z "$pattern" ]] && continue
            if ! grep -qE "$pattern" "$SUITEDIR/output/${name}.s" 2>/dev/null; then
                echo "  MISSING pattern: $pattern"
                pattern_ok=false
            fi
        done < "$SUITEDIR/expected/${name}.patterns"
    fi

    # Check forbidden patterns (68000-isms in code-generator output)
    local no_68k=true
    if [ -f "$SUITEDIR/output/${name}.s" ]; then
        local cgen_lines=$(sed -n '/^\*line/,/^[^*]/p' "$SUITEDIR/output/${name}.s" 2>/dev/null | grep -v '^\*line')
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
