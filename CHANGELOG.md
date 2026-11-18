# Changelog

All notable changes are listed here. The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/), and this project is not yet versioned for a public release.

## Unreleased

Work landed on the local `build/cmake` and `cleanup/code-quality` branches. Nothing in this list has been published as a GitHub release.

### Added

- A CMake build that mirrors the Visual Studio 2022 solution: `engine` static library, `game` executable, and presets for Windows (`windows-vs2022`, optional Ninja) and Linux (Ninja Debug/Release, plus sanitizer and coverage trees).
- vcpkg manifest mode with a pinned builtin baseline (Box2D 3.2 and the rest of the dependency set are reproducible).
- Opt-in flags: `BAJA_BUILD_TESTS`, `BAJA_STRICT_WARNINGS`, `BAJA_SANITIZE`, `BAJA_COVERAGE`, `BAJA_PCH`, and `BAJA_BUILD_BENCH`.
- Catch2 and CTest suite (98 cases): pure logic, headless ECS systems, EntityFactory and MapLoader, shipped hideout/hospital integration, JSON robustness, and a fixed-dt headless simulation.
- Dev launch flag `game --map hideout` or `game --map hospital` to skip splash and menu.
- Local CI workflow (Linux Debug, ASan/UBSan, clang-tidy, Windows VS 2022 Debug plus CTest). The file is in the tree; it is unpublished until the branch is pushed.
- Public docs: README architecture diagram, `docs/ARCHITECTURE.md`, `docs/BUILDING.md`, `docs/TECH_DEBT.md`, `CONTRIBUTING.md`, MIT `LICENSE`, and `assets/CREDITS.md`.
- `.clang-format`, `.clang-tidy`, engine/game include decoupling check, and a `baja_header_check` target that compiles each `src/engine` header on its own.

### Changed

- Box2D call sites updated for the 3.2 C API.
- imgui-sfml is built from a vcpkg overlay so Dear ImGui 1.93 font atlas and texture updates work; HUD/editor fonts load after Init.
- Fallback texture and font paths are supplied by the game through `EngineContext::AssetPathConfig`.
- Visibility tags stay in the engine; game-specific editor inspectors and empty animation-set fallbacks are registered from the game.
- Shared helpers for angle wrapping (`engine::wrapAngle`, `engine::Pi`) and testable damage, visibility, and detection math.
- Logger method names use camelCase (`getLogger`, `loadConfiguration`, `setLogger`).
- Linux warning set is `-Wall -Wextra -Wshadow -Wconversion` (MSVC `/W4` when `BAJA_STRICT_WARNINGS` is on in CMake).

### Fixed

- Unfinished Warehouse map dropped from selection; stale `map.json` removed.
- A missing scav-laugh sound entry was removed from `resources.json`.
- CMake copies assets and sets the working directory so `../../assets` and `config/logging.ini` resolve next to the executable.
- `-Wextra` findings and constructor parameter shadowing / conversion warnings in project code.
- `MapLoader` and `AssetManifestLoader` reject corrupted JSON without throwing; `EntityFactory` no longer aborts when a child archetype is missing.

### Security

- No credential or token changes. Asset media under `assets/textures`, `assets/sounds`, and `assets/music` remains gitignored; see `assets/CREDITS.md` before any public pack.
