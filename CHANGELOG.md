# Changelog

All notable changes are listed here. The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/), and this project is not yet versioned for a public release.

## Unreleased

Work landed on the local `build/cmake` and `cleanup/code-quality` branches. Nothing in this list has been published as a GitHub release.

### Added

- A CMake build that mirrors the Visual Studio 2022 solution: `engine` static library, `game` executable, and presets for Windows (`windows-vs2022`, optional Ninja) and Linux (Ninja Debug/Release, plus sanitizer and coverage trees).
- vcpkg manifest mode with a pinned builtin baseline (Box2D 3.2 and the rest of the dependency set are reproducible).
- Opt-in flags: `BAJA_BUILD_TESTS`, `BAJA_STRICT_WARNINGS`, `BAJA_SANITIZE`, `BAJA_COVERAGE`, `BAJA_PCH`, and `BAJA_BUILD_BENCH`.
- Catch2 and CTest suite (131 cases): pure logic, headless ECS systems, EntityFactory and MapLoader, shipped hideout/hospital integration, JSON robustness, a fixed-dt headless simulation, `engine::Random` seed/range tests, spawn-relative patrol checks, feet-collider slide helpers, and a hospital enemy-AI regression (seed 1, 1800 frames).
- `baja_enemy_ai_diag` headless tool (map assets, seed, seconds) that prints stuck-check, teleport, and displacement totals for hideout and hospital.
- A short enemy-AI section in `docs/ARCHITECTURE.md` (states, per-spawn waypoints, unstuck) and how to run `baja_enemy_ai_diag`.
- `HeadlessGameplayLoop` now runs state transitions, movement, and stuck recovery (same order as gameplay, before the Box2D step).
- All loggers also write to `logs/game.log` next to the executable (truncated per run, warnings flushed at once) so a crash leaves a log on disk.
- Dev launch flags: `game --map hideout` or `game --map hospital` to skip splash and menu, and `game --seed <n>` to seed the shared RNG.
- Repo hygiene: `.editorconfig`, `.gitattributes` (no bulk renormalize of existing files), GitHub issue and pull request templates, and this changelog.
- Local CI workflow (Linux Debug, ASan/UBSan, clang-tidy, Windows VS 2022 Debug plus CTest). The file is in the tree; it is unpublished until the branch is pushed.
- Public docs: README architecture diagram, `docs/ARCHITECTURE.md`, `docs/BUILDING.md`, `docs/TECH_DEBT.md`, `CONTRIBUTING.md`, MIT `LICENSE`, and `assets/CREDITS.md`.
- `.clang-format`, `.clang-tidy`, engine/game include decoupling check, and a `baja_header_check` target that compiles each `src/engine` header on its own.

### Changed

- Box2D call sites updated for the 3.2 C API.
- imgui-sfml is built from a vcpkg overlay so Dear ImGui 1.93 font atlas and texture updates work; HUD/editor fonts load after Init.
- Fallback texture and font paths are supplied by the game through `EngineContext::AssetPathConfig`.
- Visibility tags stay in the engine; game-specific editor inspectors and empty animation-set fallbacks are registered from the game.
- Shared helpers for angle wrapping (`engine::wrapAngle`, `engine::Pi`), a shared `engine::Random` (`std::mt19937` on `EngineContext`), and testable damage, visibility, and detection math.
- Logger method names use camelCase (`getLogger`, `loadConfiguration`, `setLogger`).
- Linux warning set is `-Wall -Wextra -Wshadow -Wconversion` (MSVC `/W4` when `BAJA_STRICT_WARNINGS` is on in CMake).

### Fixed

- Unfinished Warehouse map dropped from selection; stale `map.json` removed.
- A missing scav-laugh sound entry was removed from `resources.json`.
- CMake copies assets and sets the working directory so `../../assets` and `config/logging.ini` resolve next to the executable.
- `-Wextra` findings and constructor parameter shadowing / conversion warnings in project code.
- `MapLoader` and `AssetManifestLoader` reject corrupted JSON without throwing; `EntityFactory` no longer aborts when a child archetype is missing.
- Gameplay code no longer uses `srand` / `rand()` or per-system `mt19937` engines; all rolls go through `engine::Random`.
- The stock `Doxyfile` and machine-specific `.vscode/c_cpp_properties.json` are no longer tracked (left on disk and gitignored). `Doxyfile.engine` / `Doxyfile.game` were not present and were not changed.
- Patrol waypoints from an archetype are treated as offsets from each spawn and scaled by `mapScale`, or as a wander box around the spawn when the list is empty. Absolute waypoint lists on the map object (including a Tiled `waypoints` property) still override the archetype.
- Enemy wall-slide rays start at the solid feet fixture, not the sprite origin, with a lookahead of at least 48 units. `lastValidPosition` is cached only after the body actually moved. Stuck recovery tries an open-side impulse before teleporting, rejects wall overlaps, and shoves when the player can see the enemy instead of skipping recovery.
- After physics bodies exist, per-spawn patrol waypoints are shape-cast (feet collider) against Collision and HalfCollision. A blocked candidate is shortened to a point before the hit, or replaced by a random clear direction; if nothing works, the enemy patrols in place.
- If a patrolling enemy is stuck for two checks, it skips to the next waypoint instead of waiting for a teleport.
- Leaving a gameplay state (hideout to hospital via the mission table) no longer leaves a dangling `PickupSystem` listener that crashed the game on the first bullet hit.
- Shots go where they are aimed: aiming and projectile spawn share one muzzle pivot per shooter (`Muzzle` component, `-96` for the feet-origin player, default `-35` for center-origin enemies). Player shots no longer fly above the cursor and enemy bullets no longer spawn above their heads. Recoil and bloom now apply after a shot, so a single tap is not kicked upward.
- If a chasing, approaching, or rushing enemy is stuck for two checks, it sidesteps toward the most open direction for a short time instead of walking into the wall. The headless harness can place the player behind a nearby wall (`baja_enemy_ai_diag ... chase`).

### Security

- No credential or token changes. Asset media under `assets/textures`, `assets/sounds`, and `assets/music` remains gitignored; see `assets/CREDITS.md` before any public pack.
