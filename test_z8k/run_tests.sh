#!/bin/bash
# Run Z8002 test suites
# Usage: ./run_tests.sh [suite ...]
#   No args → run all suites: codegen asm run
#   ./run_tests.sh codegen       → run only codegen
#   ./run_tests.sh codegen run   → run codegen and run

TESTDIR="$(cd "$(dirname "$0")" && pwd)"

ALL_SUITES="codegen asm run"

if [ $# -gt 0 ]; then
    suites="$*"
else
    suites="$ALL_SUITES"
fi

total_pass=0
total_fail=0
total_skip=0

for suite in $suites; do
    suite_script="$TESTDIR/$suite/run.sh"
    if [ ! -f "$suite_script" ]; then
        echo "ERROR: unknown suite '$suite' (no $suite/run.sh)"
        total_fail=$((total_fail+1))
        continue
    fi

    echo "=== $suite ==="
    output=$(bash "$suite_script" 2>&1)
    echo "$output"

    # Parse SUMMARY line: "SUMMARY pass=N fail=N skip=N"
    summary=$(echo "$output" | grep '^SUMMARY ')
    if [ -n "$summary" ]; then
        p=$(echo "$summary" | sed 's/.*pass=\([0-9]*\).*/\1/')
        f=$(echo "$summary" | sed 's/.*fail=\([0-9]*\).*/\1/')
        s=$(echo "$summary" | sed 's/.*skip=\([0-9]*\).*/\1/')
        total_pass=$((total_pass + p))
        total_fail=$((total_fail + f))
        total_skip=$((total_skip + s))
    fi
    echo ""
done

echo "=== TOTAL ==="
echo "Results: $total_pass passed, $total_fail failed, $total_skip skipped"
[ $total_fail -gt 0 ] && exit 1
exit 0
