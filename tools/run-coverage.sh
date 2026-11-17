#!/usr/bin/env bash
# Build (or reuse) a BAJA_COVERAGE tree, run CTest, and write a src/ coverage report.
#
# Usage:
#   tools/run-coverage.sh
#   tools/run-coverage.sh build/linux-ninja-debug-coverage
#
# Configure first if the tree does not exist:
#   cmake --preset linux-ninja-debug-coverage
#   cmake --build --preset linux-ninja-debug-coverage
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD="${1:-$ROOT/build/linux-ninja-debug-coverage}"

if [[ ! -d "$BUILD" ]]; then
    echo "Build directory not found: $BUILD" >&2
    echo "Configure and build with BAJA_COVERAGE=ON first (preset linux-ninja-debug-coverage)." >&2
    exit 1
fi

if ! command -v ctest >/dev/null 2>&1; then
    echo "ctest is not on PATH." >&2
    exit 1
fi

if ! command -v gcov >/dev/null 2>&1; then
    echo "gcov is not on PATH (needed for the report)." >&2
    exit 1
fi

echo "Running CTest in $BUILD"
ctest --test-dir "$BUILD" --output-on-failure

OUT="$BUILD/coverage"
mkdir -p "$OUT"
python3 "$ROOT/tools/coverage_report.py" --root "$ROOT" --build "$BUILD" --out "$OUT"
echo "Wrote $OUT/summary.txt"
