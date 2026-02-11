#!/bin/bash
# Shared tool paths and helper functions for Z8002 test suites

TESTDIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CROSS="$TESTDIR/.."

# Tool paths
CP68="$CROSS/cpp/cp68"
C068="$CROSS/parser/c068"
C1Z8K="$CROSS/cgen_z8k/c1z8k"
ASZ8K="$CROSS/asz8k/asz8k"
ASZ8K_PD="$CROSS/asz8k/asz8k.pd"
XCON="$CROSS/ld8k/xcon"
LD8K="$CROSS/ld8k/ld8k"
XOUT2BIN="$TESTDIR/tools/xout2bin.py"
EMU="$CROSS/z8000_emu/build/z8000emu"

# Counters
pass=0
fail=0
skip=0

# compile_to_asm <source.c> <output.s> [<tmpdir>]
#   Runs cp68 -> c068 -> c1z8k, producing assembly output.
#   Returns 0 on success, 1 on failure.
#   On failure, error message is in $compile_errors.
compile_to_asm() {
    local src="$1"
    local out_s="$2"
    local tmpdir="${3:-$(mktemp -d)}"
    local need_cleanup=false
    [ -z "$3" ] && need_cleanup=true

    "$CP68" "$src" "$tmpdir/test.i" 2>/dev/null
    if [ $? -ne 0 ]; then
        compile_errors="preprocessor failed"
        $need_cleanup && rm -rf "$tmpdir"
        return 1
    fi

    "$C068" "$tmpdir/test.i" "$tmpdir/test.1" "$tmpdir/test.2" "$tmpdir/test.3" 2>/dev/null
    if [ $? -ne 0 ]; then
        compile_errors="parser failed"
        $need_cleanup && rm -rf "$tmpdir"
        return 1
    fi

    compile_errors=$("$C1Z8K" "$tmpdir/test.1" "$tmpdir/test.2" "$tmpdir/test.s" 2>&1)
    local rc=$?

    if [ -f "$tmpdir/test.s" ]; then
        cp "$tmpdir/test.s" "$out_s"
    fi

    $need_cleanup && rm -rf "$tmpdir"
    return $rc
}

# assemble <source.s> <output.obj>
#   Runs asz8k in a temp directory with a short filename (14-char limit workaround).
#   Returns 0 on success.
#   Sets asm_errors to stderr/stdout, asm_E_count to total E errors,
#   and asm_E_common to count of E errors on .common symbol references.
assemble() {
    local src="$1"
    local out_obj="$2"
    local asmtmp
    asmtmp=$(mktemp -d)

    # asz8k needs its predefined symbols file in CWD
    cp "$ASZ8K_PD" "$asmtmp/asz8k.pd"
    cp "$src" "$asmtmp/t.s"

    asm_errors=$( cd "$asmtmp" && "$ASZ8K" -l t.s 2>&1 )
    local rc=$?

    # Count E errors
    asm_E_count=$(echo "$asm_errors" | grep -c '^E ')
    # Count E errors that reference .common symbols (expected, not fatal)
    asm_E_common=$(echo "$asm_errors" | grep '^E ' | grep -c '\.common')

    if [ -f "$asmtmp/t.obj" ]; then
        cp "$asmtmp/t.obj" "$out_obj"
    fi

    rm -rf "$asmtmp"
    return $rc
}

# obj_to_xout <input.obj> <output.out>
#   Runs xcon to convert .obj to x.out format.
obj_to_xout() {
    local obj="$1"
    local out="$2"
    "$XCON" -o "$out" "$obj" 2>&1
}

# link_xout <output.out> <input1.out> [<input2.out> ...]
#   Runs ld8k to link x.out files.
link_xout() {
    local out="$1"
    shift
    "$LD8K" -o "$out" "$@" 2>&1
}

# xout_to_bin <input.out> <output.bin>
#   Converts x.out to flat binary using xout2bin.py.
xout_to_bin() {
    local xout="$1"
    local bin="$2"
    python3 "$XOUT2BIN" "$xout" "$bin" 2>&1
}

# run_emu <binary> [extra_args...]
#   Runs the Z8000 emulator, returns R0 value (decimal) in $emu_r0.
#   Sets emu_output to full emulator output.
#   Returns emulator exit code.
run_emu() {
    local bin="$1"
    shift
    emu_output=$("$EMU" --cycles 100000 "$@" "$bin" 2>&1)
    local rc=$?

    # Extract R0 from register dump: "R0 =XXXX"
    local r0_hex
    r0_hex=$(echo "$emu_output" | grep 'R0 =' | head -1 | sed 's/.*R0 =\([0-9A-Fa-f]*\).*/\1/')
    if [ -n "$r0_hex" ]; then
        emu_r0=$((16#$r0_hex))
    else
        emu_r0=""
    fi

    return $rc
}

# print_summary
#   Outputs summary line in machine-parseable format.
print_summary() {
    echo "SUMMARY pass=$pass fail=$fail skip=$skip"
}
