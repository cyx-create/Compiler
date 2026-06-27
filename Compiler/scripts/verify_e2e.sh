#!/usr/bin/env bash
# Full end-to-end verification wrapper.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

if [ ! -x build/tools/main/fdmjc ]; then
  echo "Build fdmjc first: make build" >&2
  exit 2
fi

bash scripts/setup_test.sh >/dev/null

PYTHON="${PYTHON:-python3}"
EXTRA=()
case "${1:-}" in
  --quick)
    EXTRA=(--quick --skip-structural)
    shift
    ;;
  --full)
    EXTRA=(--with-opt-consistency --with-stress --modes base,const,loop1,loop2,allloop,allopt)
    shift
    ;;
esac

exec "$PYTHON" scripts/verify_e2e.py --compile-missing "${EXTRA[@]}" "$@"
