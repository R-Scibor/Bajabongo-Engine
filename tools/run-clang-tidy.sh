#!/usr/bin/env bash
# Run project clang-tidy over src/ using compile_commands.json from a CMake
# build tree. Adds the GCC builtin include dir so clang-tidy (Clang) can
# find stddef.h when the compile database was produced by GCC.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD="${1:-$ROOT/build/linux-ninja-debug}"
GCC_INCLUDE="$(g++ -print-file-name=include)"
if [[ ! -f "$GCC_INCLUDE/stddef.h" ]]; then
  GCC_INCLUDE="/usr/lib/gcc/x86_64-linux-gnu/$(g++ -dumpversion)/include"
fi
if [[ ! -f "$BUILD/compile_commands.json" ]]; then
  echo "No compile_commands.json in $BUILD" >&2
  exit 2
fi
cd "$ROOT"
mapfile -t FILES < <(find src -name '*.cpp' | sort)
clang-tidy -p "$BUILD" --quiet --use-color=false \
  --extra-arg="-isystem${GCC_INCLUDE}" \
  "${FILES[@]}"
