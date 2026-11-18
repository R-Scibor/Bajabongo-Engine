# Contributing

Thanks for looking at Bajabongo Engine. This file is a short guide to local style and tests.

## Language and layout

- C++20, CMake 3.21 or newer.
- 4-space indent, no tabs, attached braces, `Type&` / `Type*` on the left, roughly 120 columns.
- Format C++ with clang-format using the repo `.clang-format`. A typical version is clang-format 19:

```bash
clang-format -i path/to/file.cpp
```

- Keep engine and game decoupled: sources under `src/engine` must not include `game/` headers. The build checks this automatically.
- Prefer small, focused changes. Do not change gameplay behavior unless the change is intentional, testable, and described in the commit message.
- Keep new documentation in plain ASCII punctuation (no en or em dashes).

## Tests

Logic changes should come with Catch2 tests under `tests/`. The suite is headless (no window). Linux presets enable `BAJA_BUILD_TESTS`. Run:

```bash
cmake --build --preset linux-ninja-debug
ctest --test-dir build/linux-ninja-debug --output-on-failure
```

On Linux, also run the sanitizer tree when you touch parsers, physics, or ECS:

```bash
cmake --preset linux-ninja-debug-sanitize
cmake --build --preset linux-ninja-debug-sanitize
ASAN_OPTIONS=detect_leaks=1:abort_on_error=1 ctest --test-dir build/linux-ninja-debug-sanitize --output-on-failure
```

See [Building](docs/BUILDING.md) for Windows, coverage, and the optional `BAJA_BUILD_BENCH` target.

## Commits

Write short imperative commit messages (for example `test: cover MapLoader without a window`). Do not commit build trees, `imgui.ini`, secrets, or other paths listed in `.gitignore`.
