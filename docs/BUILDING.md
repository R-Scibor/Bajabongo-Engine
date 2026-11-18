# Building

This project uses CMake 3.21 or newer, C++20, and vcpkg in manifest mode. Presets live in `CMakePresets.json`. Dependencies are declared in `vcpkg.json` (SFML 3, EnTT, Box2D 3.2, spdlog, nlohmann-json, Dear ImGui, imgui-sfml, Catch2) and pinned by the builtin baseline in that file.

Set `VCPKG_ROOT` to a vcpkg checkout (or the Visual Studio bundled copy). Every CMake preset loads `$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake` and turns on `VCPKG_MANIFEST_MODE`.

## Windows (Visual Studio 2022 + CMake)

1. Install Visual Studio 2022 with the "Desktop development with C++" workload.
2. Provide vcpkg and set `VCPKG_ROOT`:
   - VS-bundled vcpkg: install the "C++ vcpkg package manager" component, then point `VCPKG_ROOT` at that copy (commonly `C:\Program Files\Microsoft Visual Studio\2022\<Edition>\VC\vcpkg`).
   - Standalone vcpkg: clone [vcpkg](https://github.com/microsoft/vcpkg) and set `VCPKG_ROOT` to that directory.
3. From the repo root, in a Developer Command Prompt (or any shell with `cmake` and `VCPKG_ROOT`):

```bat
cmake --preset windows-vs2022
cmake --build --preset windows-vs2022-debug
```

Use `windows-vs2022-release` for a Release build. The executable is written to `build/windows-vs2022/out/game/Debug/game.exe` (or `Release`). Run it from that folder so the copied `../../assets` and `config/logging.ini` paths resolve.

Optional single-config Ninja presets, if Ninja is on PATH: `windows-ninja-debug` and `windows-ninja-release`.

To build tests on Windows:

```bat
cmake --preset windows-vs2022 -DBAJA_BUILD_TESTS=ON
cmake --build --preset windows-vs2022-debug
ctest --test-dir build/windows-vs2022 -C Debug --output-on-failure
```

### Visual Studio solution (alternative)

`Bajabongo-Engine.sln` remains a supported MSVC workflow. Integrate vcpkg with Visual Studio (`vcpkg integrate install`), open the solution, pick Debug or Release (x64 recommended), and build. The debugger working directory defaults to `src/game`, which is how the classic relative asset paths resolve in this workflow. Keep the `.vcxproj` source lists in sync if you add or rename files.

## Linux (CMake + Ninja)

Install Ninja, CMake 3.21 or newer, a C++20 toolchain (GCC or Clang), and typical X11 / OpenGL development packages so the SFML vcpkg port can compile. Then:

```bash
export VCPKG_ROOT=/path/to/vcpkg
cmake --preset linux-ninja-debug
cmake --build --preset linux-ninja-debug
cd build/linux-ninja-debug/out/game/Debug
./game
```

Linux presets turn on `BAJA_STRICT_WARNINGS` (`-Wall -Wextra -Wshadow -Wconversion`) and `BAJA_BUILD_TESTS`. Use `linux-ninja-release` for a Release build.

```bash
ctest --test-dir build/linux-ninja-debug --output-on-failure
```

### Sanitizers

`linux-ninja-debug-sanitize` is a separate tree with `BAJA_SANITIZE` (ASan + UBSan on engine, game, and tests; not on vcpkg deps):

```bash
cmake --preset linux-ninja-debug-sanitize
cmake --build --preset linux-ninja-debug-sanitize
ASAN_OPTIONS=halt_on_error=1:abort_on_error=1:detect_leaks=1 \
UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 \
ctest --test-dir build/linux-ninja-debug-sanitize --output-on-failure
```

For a gameplay smoke, run `build/linux-ninja-debug-sanitize/out/game/Debug/game --map hideout` (or `hospital`). Leave `detect_leaks=0` if you stop the process with SIGTERM.

### Coverage

`linux-ninja-debug-coverage` is a separate tree with `BAJA_COVERAGE` (gcov). Do not combine it with `BAJA_SANITIZE`.

```bash
cmake --preset linux-ninja-debug-coverage
cmake --build --preset linux-ninja-debug-coverage
tools/run-coverage.sh
```

The default report covers `src/` files exercised by the test binary. See `build/linux-ninja-debug-coverage/coverage/summary.txt`.

### Optional benchmark

With tests enabled:

```bash
cmake --preset linux-ninja-debug -DBAJA_BUILD_BENCH=ON
cmake --build --preset linux-ninja-debug --target baja_bench
./build/linux-ninja-debug/tests/baja_bench
```

`baja_bench` runs a fixed-dt headless hideout simulation and prints elapsed time, ms/frame, and fps to stdout. It is off by default and is not part of CTest.

Other useful flags: `BAJA_PCH=ON` precompiles `src/engine/pch.h` (off by default). `BAJA_STRICT_WARNINGS` is already on in the Linux presets.

Dev launches: `game --map hideout` or `game --map hospital` skip splash and menu.

## vcpkg notes

- Manifest mode is required. The builtin baseline in `vcpkg.json` pins port versions.
- `vcpkg-configuration.json` adds `./cmake/vcpkg-overlays` as overlay-ports. The tree currently overlays `imgui-sfml` so it builds against Dear ImGui 1.93 (texture and font atlas patches).
- First configure can take a while while vcpkg builds SFML and friends. Later configures reuse `vcpkg_installed` in the build tree.

## Troubleshooting

### `VCPKG_ROOT` or toolchain missing

If CMake prints a notice about no `CMAKE_TOOLCHAIN_FILE`, the presets were not used or `VCPKG_ROOT` is unset. Export or set the variable and configure with a preset. System packages are only a fallback and often lack SFML 3, Box2D 3, or imgui-sfml.

### imgui-sfml overlay and reconfigure

If imgui-sfml fails to compile (font atlas, `ImGuiBackendFlags_RendererHasTextures`, or similar ImGui 1.9x API errors), the overlay port in `cmake/vcpkg-overlays/imgui-sfml` is probably not the copy vcpkg built.

1. Confirm `vcpkg-configuration.json` lists `./cmake/vcpkg-overlays` under `overlay-ports`.
2. Reconfigure from a clean build tree, or remove the already-installed imgui-sfml package from that tree (`vcpkg_installed`) and run CMake again so the overlay port is rebuilt. Changing only the overlay files does not rebuild a package that is already installed.
3. Do not mix an old `vcpkg_installed` from a tree that was configured before the overlay existed.

### Assets or fonts not found

Run the executable from `out/game/Debug` (or `Release`), not from the build root. Relative paths such as `../../assets` and `config/logging.ini` are resolved from that working directory. In the `.sln` workflow, the working directory is `src/game`.

### Coverage or sanitizer flags in the wrong tree

Use the dedicated presets. They write to `build/linux-ninja-debug-sanitize` and `build/linux-ninja-debug-coverage`. Do not turn both `BAJA_SANITIZE` and `BAJA_COVERAGE` on in one tree. If `BAJA_PCH` was left ON in a cache and you want it off, pass `-DBAJA_PCH=OFF` when reconfiguring; the preset does not force it off.

### clang-tidy cannot find `stddef.h`

After a GCC-generated `compile_commands.json`, run `tools/run-clang-tidy.sh <build-dir>`. The script adds the GCC builtin include path.

### Linux SFML port fails to configure

Install X11, OpenGL, and related development packages, then reconfigure. The SFML vcpkg port needs those headers on Linux.
