#!/usr/bin/env bash
# Compare generated .4.quad text against HW5 reference output for IR-bootstrapped tests.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
TST="$(cd "$(dirname "$0")/.." && pwd)/test"
FDMJC="$(cd "$(dirname "$0")/.." && pwd)/build/tools/main/fdmjc"

if [ ! -x "$FDMJC" ]; then
    echo "Build fdmjc first: make build" >&2
    exit 1
fi

bash "$(dirname "$0")/setup_test.sh" >/dev/null

pass=0
fail=0
skip=0

check_quad() {
    local name="$1"
    local ref="$ROOT/HW5/test/${name}.4.quad"
    local got="$TST/${name}.4.quad"

    if [ ! -f "$ref" ]; then
        echo "SKIP $name (no HW5 reference .4.quad)"
        skip=$((skip + 1))
        return
    fi

    rm -f "$got" "$TST/${name}.4-xml.quad" "$TST/${name}.4-ssa.quad" \
          "$TST/${name}.4-ssa-withflow-xml.quad" "$TST/${name}.4-quadwithflow-xml.quad" \
          "$TST/${name}.reflow-"* "$TST/${name}.base"* "$TST/${name}.const"* \
          "$TST/${name}.loop"* "$TST/${name}.all"*

    # Generate quad only (skip codegen/regalloc for faster structural check).
    if ! "$FDMJC" "$TST" "$name" quad-only >/dev/null 2>&1; then
        echo "FAIL $name (compile error)"
        fail=$((fail + 1))
        return
    fi

    if [ ! -f "$got" ]; then
        echo "FAIL $name (missing $got)"
        fail=$((fail + 1))
        return
    fi

    if diff -q \
        <(python3 "$(dirname "$0")/normalize_quad.py" < <(tail -n +2 "$ref")) \
        <(python3 "$(dirname "$0")/normalize_quad.py" < <(tail -n +2 "$got")) \
        >/dev/null; then
        echo "PASS $name"
        pass=$((pass + 1))
    else
        echo "FAIL $name (differs from HW5 reference)"
        diff -u \
            <(python3 "$(dirname "$0")/normalize_quad.py" < <(tail -n +2 "$ref")) \
            <(python3 "$(dirname "$0")/normalize_quad.py" < <(tail -n +2 "$got")) \
            | head -40 || true
        fail=$((fail + 1))
    fi
}

for name in quadtest1 quadtest2 quadtest3 quadtest4 quadtest5 \
            quadtest6 quadtest7 quadtest8 quadtest9 \
            simplecall1 simplecall2; do
    check_quad "$name"
done

echo "----"
echo "quad verify: $pass passed, $fail failed, $skip skipped"
[ "$fail" -eq 0 ]
