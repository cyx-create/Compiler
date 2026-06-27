#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
TST="$(cd "$(dirname "$0")/.." && pwd)/test"
mkdir -p "$TST"

copy_fmj() {
    local src_dir="$1"
    local name="$2"
    cp "$ROOT/${src_dir}/test/${name}.fmj" "$TST/${name}.fmj" 2>/dev/null || true
}

copy_backend_bootstrap() {
    local name="$1"
    cp "$ROOT/HW12/test/${name}.fmj" "$TST/${name}.fmj" 2>/dev/null || true
    cp "$ROOT/HW12/test/${name}.5-xml.asm" "$TST/${name}.5-xml.asm" 2>/dev/null || true
    cp "$ROOT/HW11/test/${name}.4-ssa-withflow-xml.quad" "$TST/${name}.4-ssa-withflow-xml.quad" 2>/dev/null || true
    cp "$ROOT/HW11/test/${name}.4-ssa.quad" "$TST/${name}.4-ssa.quad" 2>/dev/null || true
}

echo "Installing HW3 IR tests (.fmj only — parsed by PARSING frontend)..."
for name in irtest1 irtest2 irtest3 irtest4 irtest5 irtest6 irtest7 irtest8 irtest9 \
            irtest10 irtest11 irtest12 irtest13 irtest14 irtest15 irtest16 irtest17 \
            irtest18 irtest19 irtest20 irtest21 irtest22; do
    copy_fmj HW3 "$name"
done

echo "Installing HW5 quad tests (.fmj only)..."
for name in quadtest1 quadtest2 quadtest3 quadtest4 quadtest5 \
            quadtest6 quadtest7 quadtest8 quadtest9 \
            simplecall1 simplecall2; do
    copy_fmj HW5 "$name"
done

echo "Installing HW12 backend bootstrap tests..."
for name in insttest0 insttest1 insttest2 insttest3 insttest4 \
            fibonacci bubblesort \
            optloopivtest1 optloopivtest2 optloopivtest3 \
            optloopivtest4 optloopivtest5 optloopivtest6; do
    copy_backend_bootstrap "$name"
done

echo "Test fixtures installed in $TST ($(ls -1 "$TST"/*.fmj 2>/dev/null | wc -l) .fmj files)"
