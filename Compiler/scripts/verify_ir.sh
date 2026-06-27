#!/usr/bin/env bash
# Compile all IR-only tests (HW3 bootstrap AST) through frontend → IR.
set -euo pipefail
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

for f in "$TST"/irtest*.fmj; do
    [ -f "$f" ] || continue
    name="$(basename "$f" .fmj)"
    ast="$TST/${name}.2-semant.ast"
    if [ ! -f "$ast" ]; then
        echo "SKIP $name (no .2-semant.ast)"
        skip=$((skip + 1))
        continue
    fi
    rm -f "$TST/${name}.3.irp" "$TST/${name}.3-canon.irp" "$TST/${name}.4.quad" \
          "$TST/${name}.4-xml.quad"
    if "$FDMJC" "$TST" "$name" quad-only >/dev/null 2>&1 && [ -f "$TST/${name}.4.quad" ]; then
        echo "PASS $name"
        pass=$((pass + 1))
    else
        echo "FAIL $name"
        fail=$((fail + 1))
    fi
done

echo "----"
echo "IR/quad smoke: $pass passed, $fail failed, $skip skipped"
[ "$fail" -eq 0 ]
