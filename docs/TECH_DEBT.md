# Technical debt (source markers)

Inventory of `TODO`, `FIXME`, and `HACK` comments in `src/` as of 18 Nov 2026 (Europe/Warsaw). Public information only. A broader, older backlog also lives in [BACKLOG.md](BACKLOG.md).

No `FIXME` comments were found in `src/`.

## Game

### `src/game/states/GameplayState.cpp`

- **HACK, empty animation set resolver.** When an archetype name contains `"enemy"`, the hook still returns `"player"`. This is the historic fallback used when a set id is empty. Changing it would alter enemy visuals unless every enemy clip is verified. Left as-is.

- **HACK, `GameplayState::onExit`.** `registry.clear()` tears down the scene and fires `on_destroy` hooks (including physics cleanup). Global entities are lost. A tagged or filtered clear would need a policy for those entities. Left as-is.

### `src/game/systems/WeaponSystem.cpp`

- **TODO, reload sound.** A comment marks where a reload sound could be started after `startReload`. Audio paths are on hold and were not changed.

- **Muzzle height (resolved).** Spawn and aim share `muzzlePivot()`; the height is per shooter (`Muzzle` component in `archetypes.json`, default `-35`). Sprites still do not rotate the barrel, so steep up/down shots start from the pivot rather than the drawn gun.

## Engine

### `src/engine/rendering/IRenderer.hpp`

- **HACK note on `drawSpriteDirect`.** The comment describes a raw sprite pass-through for debugging or legacy integration, so the renderer does not have to look up a full renderable component. The method is still part of the public interface. The note was left as documentation, not removed.

## Fixes in this pass

None of these markers were both trivial and safe to change without a behavior or audio risk.

## Follow-ups

- The empty animation-set `"player"` fallback and `GameplayState::onExit` `registry.clear()` are unchanged (see above). A filtered teardown was not applied in this pass.
- Enemy AI has no pathfinding. Chase, patrol, rush, and retreat move in a straight line with a short wall-slide ray. Stuck recovery is reactive (impulse, teleport, shove, or kill). A headless diagnosis of wall-stuck behavior is in the cleanup log; no gameplay change was made in this pass.
