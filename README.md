# Bajabongo Engine

A modular, data-driven 2D game engine built with **C++20**, **SFML 3.1**, **EnTT**, and **Box2D 3.2**.

The project is structured into two distinct parts: the **Core Engine** (`src/engine`) which provides reusable systems and abstractions, and the **Game Module** (`src/game`) which implements specific gameplay logic, acting as the composition root.

## 🚀 Key Features

### Core Technology
*   **Modern C++20 Codebase**: Utilizes latest standard features for performance and safety.
*   **Entity Component System (ECS)**: Powered by [EnTT](https://github.com/skypjack/entt) for high-performance entity management.
*   **Physics Engine**: Integrated **Box2D 3.2** (C API) for reliable 2D physics simulation.
*   **Rendering**: **SFML 3.1** backend with support for sprite batching, animations, and camera control.
*   **Input System**: Abstracted input handling supporting keyboard and mouse.
*   **Logging**: Structured, multi-sink logging using [spdlog](https://github.com/gabime/spdlog).

### Engine Systems
*   **State Management**: Stack-based Finite State Machine (Push, Pop, Swap) handling game flow (Splash -> Menu -> Gameplay).
*   **Asset Pipeline**: JSON-based asset manifests with hot-reloading capabilities using `tools/AssetBaker.py`.
*   **Event System**: Decoupled event dispatching for physics collisions, audio triggers, and state changes.
*   **Animation**: Component-based animation system supporting multiple clips and state-based transitions.

### Gameplay Features
*   **AI Behavior**: Robust AI system including:
    *   Enemy detection and state transitions (Idle, Alert, Combat).
    *   Behavioral archetypes: Patrol, Rush, Sniper, Turret.
    *   Pathfinding and obstacle avoidance.
*   **Combat System**:
    *   Weapon inventory and switching mechanics.
    *   Projectile ballistics and hit detection.
    *   Damage handling and visual feedback (Hit Flash).
*   **Visibility & Fog**: Dynamic visibility system ("Fog of War") based on player line-of-sight.
*   **Interactive Maps**: JSON-based map loading with defined spawn points, resources, and collision geometry.

## 📂 Project Structure

```
Bajabongo-Engine/
├── assets/                 # Game assets (sprites, fonts, data, shaders)
├── cmake/                  # Compiler options, runtime layout, decoupling check
├── docs/                   # Detailed technical documentation
├── src/
│   ├── engine/             # Core Engine Library (Reusable)
│   │   ├── core/           # Application loop, DI, StateManager
│   │   ├── ecs/            # Entity Factory, Archetypes, Systems
│   │   ├── physics/        # Box2D integration
│   │   ├── rendering/      # SFML wrappers, Animation
│   │   └── ...
│   └── game/               # Game Logic (Implementation)
│       ├── ai/             # Enemy AI logic
│       ├── components/     # Game-specific components
│       ├── states/         # Game states (MainMenu, Gameplay)
│       ├── systems/        # Gameplay systems (Damage, Weapon, Fog)
│       └── game.cpp        # Composition Root (Main Entry)
├── tests/                  # Catch2 unit tests (no window required)
├── tools/                  # Python tools for asset management
├── CMakeLists.txt
└── CMakePresets.json
```

## 🛠️ Getting Started

### Prerequisites
*   **CMake 3.21** or newer
*   A **C++20** compiler: **Visual Studio 2022** on Windows, or a recent GCC or Clang on Linux
*   **vcpkg** in manifest mode (`vcpkg.json` at the repo root). Use Visual Studio's bundled vcpkg, or a standalone clone. In both cases set `VCPKG_ROOT` (the CMake presets load `$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake`).
*   **Ninja** for the Linux presets (and the optional Windows Ninja presets)
*   **Python 3.x** (for asset tools)

Dependencies are resolved from `vcpkg.json` (SFML, EnTT, Box2D, spdlog, nlohmann-json, Dear ImGui, imgui-sfml, Catch2).

### Windows (CMake + Visual Studio 2022)

1.  Install Visual Studio 2022 with the "Desktop development with C++" workload.
2.  Provide vcpkg and set `VCPKG_ROOT`:
    *   **VS-bundled vcpkg**: install the Visual Studio "C++ vcpkg package manager" component, then point `VCPKG_ROOT` at that copy (commonly `C:\Program Files\Microsoft Visual Studio\2022\<Edition>\VC\vcpkg`).
    *   **Standalone vcpkg**: clone [vcpkg](https://github.com/microsoft/vcpkg) and set `VCPKG_ROOT` to that directory.
3.  From the repo root, in a Developer Command Prompt or any shell where `cmake` and `VCPKG_ROOT` are available:

```bat
cmake --preset windows-vs2022
cmake --build --preset windows-vs2022-debug
```

Use `windows-vs2022-release` for a Release build. The executable is written to `build/windows-vs2022/out/game/Debug/game.exe` (or `Release`). Run it from that folder so the copied `../../assets` and `config/logging.ini` paths resolve.

Optional single-config Ninja presets, if Ninja is on PATH: `windows-ninja-debug` and `windows-ninja-release`.

### Visual Studio solution (alternative)

The `Bajabongo-Engine.sln` / `.vcxproj` files remain a supported Windows/MSVC workflow:

1.  Ensure **vcpkg** is installed and integrated with Visual Studio (`vcpkg integrate install`).
2.  Open `Bajabongo-Engine.sln` in Visual Studio.
3.  Select the **Debug** or **Release** configuration (x64 recommended).
4.  Build the solution (F7). Visual Studio should automatically invoke vcpkg to install missing dependencies (SFML, EnTT, Box2D, Spdlog).
5.  Run the `game` project. The Visual Studio debugger defaults the working directory to `src/game`, which is how the classic relative asset paths resolve in this workflow.

### Linux (CMake + Ninja)

Install Ninja, CMake 3.21+, a C++20 toolchain, and typical X11 / OpenGL development packages so the SFML vcpkg port can compile. Then:

```bash
export VCPKG_ROOT=/path/to/vcpkg
cmake --preset linux-ninja-debug
cmake --build --preset linux-ninja-debug
```

The Linux presets also turn on `BAJA_STRICT_WARNINGS` (`-Wall -Wextra`) and `BAJA_BUILD_TESTS`. Use `linux-ninja-release` for a Release build.

The executable is written to `build/linux-ninja-debug/out/game/Debug/game` (or the Release equivalent). Run it from that folder:

```bash
cd build/linux-ninja-debug/out/game/Debug
./game
```

### Testing

Unit tests use [Catch2](https://github.com/catchorg/Catch2) and CTest. They cover pure logic (angle wrapping, vectors, AABB helpers, point-in-polygon, JSON parsing and serialization, logger.ini parsing, ammo/reload, health, and the state stack) and do not open a window. The suite currently has 46 cases.

`BAJA_BUILD_TESTS` defaults to **OFF**. The Linux presets turn it **ON**. On Windows, pass the option when configuring:

```bat
cmake --preset windows-vs2022 -DBAJA_BUILD_TESTS=ON
cmake --build --preset windows-vs2022-debug
ctest --test-dir build/windows-vs2022 -C Debug --output-on-failure
```

On Linux, after a `linux-ninja-debug` (or release) build:

```bash
ctest --test-dir build/linux-ninja-debug --output-on-failure
```

For a local clang-tidy pass over `src/` (same checks as `.clang-tidy`), configure a build so `compile_commands.json` exists, then run:

```bash
tools/run-clang-tidy.sh build/linux-ninja-debug
```

The script adds the GCC builtin include path so Clang can find `stddef.h` when the compile database was produced by GCC, and it exits with a non-zero status if a project warning is reported.

### Sanitizers

On Linux, AddressSanitizer and UndefinedBehaviorSanitizer are opt-in via `BAJA_SANITIZE` (GCC/Clang only). The flags apply to engine, game, and tests, not to vcpkg dependencies. A separate build tree keeps the regular Debug build clean:

```bash
export VCPKG_ROOT=/path/to/vcpkg
cmake --preset linux-ninja-debug-sanitize
cmake --build --preset linux-ninja-debug-sanitize
ASAN_OPTIONS=halt_on_error=1:abort_on_error=1:detect_leaks=1 UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 ctest --test-dir build/linux-ninja-debug-sanitize --output-on-failure
```

The sanitized executable is written to `build/linux-ninja-debug-sanitize/out/game/Debug/game`. For a gameplay smoke, run it with `--map hideout` or `--map hospital`. Leak detection is useful for CTest (clean process exit). A smoke that stops the game with SIGTERM is not a clean shutdown, so leave `detect_leaks=0` for those runs.

The local CI workflow in `.github/workflows/ci.yml` includes Linux jobs for the regular Debug build, the sanitizer preset, and clang-tidy, plus a Windows VS 2022 Debug + CTest job. The workflow file is in the tree; it is not published until the branch is pushed.

### Dev map flag

Launching `game` with no arguments starts the normal flow (splash, then menu). For local smoke tests you can skip splash and menu and load a map directly:

```text
game --map hideout
game --map hospital
```

`--map=hideout` and `--map=hospital` are accepted as well. `game --help` (or `-h`) prints usage. Any other map name is rejected. This flag is a development aid; default behavior is unchanged when it is omitted.

## Controls

Derived from the current input handling in the game module.

**Splash**
*   Enter or Space: continue

**Main menu**
*   W / Up Arrow, S / Down Arrow: move the selection
*   Enter or Space: confirm
*   Escape: quit
*   Left mouse button: click a menu entry

**Map selection**
*   Left mouse button: choose a map marker
*   Escape: return to Hideout gameplay

**Gameplay**
*   W A S D: move
*   Left Shift: run
*   Mouse: face the cursor
*   Right mouse button: aim
*   Left mouse button: fire (while aiming)
*   R: reload
*   1-9: switch weapons
*   H: use a medkit (requires a medkit, and current HP below max)
*   E: interact (for example the Mission Table, which opens map selection)
*   Escape: return to the previous state
*   F1: toggle the in-game editor

## 📚 Documentation

Detailed documentation for specific subsystems can be found in the `docs/` directory:

*   [**Asset Pipeline**](docs/asset_pipeline.md): How to add and manage assets.
*   [**ECS Architecture**](docs/ecs.md): Entity-Component-System patterns and usage.
*   [**State Management**](docs/state_management.md): How the game flow is structured.
*   [**Rendering & Animation**](docs/rendering_animation.md): Sprite and animation details.
*   [**Physics System**](docs/physics.md): Box2D integration and collision handling.
*   [**Input System**](docs/input.md): Handling user input.
*   [**Event System**](docs/events.md): Event dispatching architecture.
*   [**Audio System**](docs/audio.md): Audio playback and management.
*   [**Logging**](docs/logging.md): Debugging and logging practices.
*   [**Math Library**](docs/math.md): Vector and geometry utilities.

## 🏗️ Architecture Overview

The engine follows **SOLID** principles, heavily utilizing **Dependency Injection (DI)**.
*   **EngineContext**: A central container holding references to core services (Renderer, Input, Registry, PhysicsWorld).
*   **Application Class**: Drives the main loop but delegates logic to the active `GameState`.
*   **Separation of Concerns**: The `engine` project knows nothing about `game` logic. The `game` project ties everything together. Engine code must not include headers from `src/game`; CMake enforces this on every build via `cmake/CheckEngineDecoupling.cmake`. Game systems talk to the engine through interfaces, hooks, and configuration rather than reaching into engine internals that would create a reverse dependency.
*   **Asset paths**: Fallback texture and font locations live in `EngineContext::AssetPathConfig`. The composition root (`src/game/game.cpp`) fills those paths in; the engine does not hardcode game assets such as `eg.png` or `default.ttf`.

## Screenshots and GIFs

> **PLACEHOLDER (Rafał will provide):** screenshots and short GIFs of the game belong in this section. Add images here when they are ready. Do not leave marketing shots out of this heading; this block is only a stand-in.

_No screenshots or GIFs have been added yet._

## License

Source code and documentation are released under the [MIT License](LICENSE).

Files under `assets/` have their own terms. See [assets/CREDITS.md](assets/CREDITS.md).
