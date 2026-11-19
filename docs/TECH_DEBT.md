# Technical debt

This page lists the `TODO`, `FIXME`, and `HACK` comments in `src/` and the known limitations of the engine and the game. It was last checked against the code on 19 Nov 2026.

## Source markers

There are no `FIXME` comments in `src/`.

### `src/game/states/GameplayState.cpp`

- **HACK, empty animation set resolver.** When an entity has no animation set id and its archetype name contains `"enemy"`, the resolver returns `"player"`, so the enemy uses the player clips. Changing it would alter enemy visuals unless every enemy clip is verified first.
- **HACK, `GameplayState::onExit`.** `registry.clear()` tears down the whole scene and fires the `on_destroy` hooks (including physics cleanup). Any entity meant to outlive the state is lost. A filtered clear would need a policy for such entities.

### `src/game/systems/WeaponSystem.cpp`

- **TODO, reload sound.** A comment marks where a reload sound could start after `startReload`. No reload sound is played yet.

### `src/engine/rendering/IRenderer.hpp`

- **HACK note on `drawSpriteDirect`.** The comment describes a raw sprite pass-through for debugging or legacy code, which skips the renderable component lookup. The method is still part of the interface but nothing in `src/` calls it.

## Known limitations and roadmap

### Rendering

- No frustum culling or spatial partitioning. `RenderSystem` collects and sorts every entity with `TransformComponent` and `RenderableComponent` each frame. This is fine for the current maps; larger maps would need a view-rect test or a spatial grid. (`src/engine/rendering/RenderSystem.cpp`)
- Some engine interfaces still use SFML types: `IGameState::handleEvent` and `IInputManager::processEvent` take `sf::Event`, `RenderSystem::update` takes `sf::RenderTarget`, and `RenderableComponent::color` is `sf::Color`. An engine-level input event type would remove the first two. (`src/engine/core/IGameState.hpp`, `src/engine/core/IInputManager.hpp`)

### Animation

- Clip lookup is based on string naming. `AnimationStateMachineSystem` builds clip ids from the animation set id plus fixed suffixes (`_idle`, `_walk_anim`, `_aim`, `_dead`, `_heal`) and strips `_suffix` parts of the set id as a fallback. A data-driven state-to-clip map would be less fragile. (`src/engine/systems/AnimationStateMachineSystem.cpp`)
- Every state needs a clip. If a clip is missing, `AnimationSystem` logs a warning once and leaves the sprite on its last frame. `tools/AssetBaker.py` works around this by generating a 1-frame clip for each static sprite; an engine fallback to the static sprite would be cleaner. (`src/engine/rendering/AnimationSystem.cpp`)
- Facing is a horizontal flip only. There are no directional sprite sets. (`src/engine/systems/AnimationStateMachineSystem.cpp`)

### ECS and hierarchy

- `HierarchySystem` updates children in registry order. In a chain deeper than one level, a child can be processed before its parent and lag one frame. There are no dirty flags, so every child is recomputed each tick. (`src/engine/ecs/HierarchySystem.cpp`)
- Children can only be destroyed together with their parent. There is no detach operation that keeps the world transform (for example, dropping a held item). (`src/engine/ecs/HierarchySystem.cpp`)

### Scenes and persistence

- No save or load. Nothing in the registry is serialized, so entities spawned at runtime (for example from the editor's Asset Browser) are gone when the state exits. A save system would also need a post-load step to recreate Box2D bodies, since `b2BodyId` handles cannot be stored. (`src/game/states/GameplayState.cpp`)
- No state carries over between maps. Because of the `registry.clear()` above, player health and inventory are rebuilt from the archetype on every map load.
- Asset and config paths are relative to the working directory (`../../assets/...`, `config/logging.ini`), so the game must be started from its output folder. (`src/game/game.cpp`, `cmake/CopyRuntimeAssets.cmake`)

### Maps

- `MapLoader` reads only image layers and the object layers named `Collision`, `HalfCollision`, and `Entities`. Tile layers, tilesets, and other layer names are ignored. (`src/engine/assets/MapLoader.cpp`)
- Collision objects are read as rectangles (`x`, `y`, `width`, `height`, `rotation`). Polygon and ellipse objects are not supported, and each rectangle is a separate box. (`src/engine/assets/MapLoader.cpp`)

### Gameplay

- Enemy AI has no pathfinding. Chase, patrol, rush, and retreat steer in a straight line with a short wall-slide ray. Stuck recovery is reactive (sidestep, impulse, teleport, shove, or as a last resort kill). (`src/game/ai/`)
- Projectiles start from a per-shooter muzzle pivot (`Muzzle` in `archetypes.json`, default height `-35`). Sprites do not rotate the gun, so steep up or down shots start from the pivot rather than the drawn barrel. (`src/game/components/MuzzleComponent.hpp`)
