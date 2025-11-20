# Rendering and Animation Systems

The Bajabongo-Engine employs a decoupled rendering architecture where game logic is separated from the actual drawing implementation. This system is built around an Entity Component System (ECS) pattern, leveraging `entt` for component management.

## Core Architecture

The rendering pipeline is designed to be flexible and data-driven. The core components are:

*   **Components:** Data structures attached to entities (e.g., `RenderableComponent`, `AnimationComponent`).
*   **Systems:** Logic that processes entities based on their components (e.g., `RenderSystem`, `AnimationSystem`).
*   **Resources:** Shared data definitions (e.g., `SpriteDesc`, `AnimationClip`).
*   **Abstraction:** The `IRenderer` interface abstracts the underlying graphics library (SFML), allowing for potential future replacements or mocking for tests.

## Rendering Components

To render an entity, it typically needs at least two components:

### 1. TransformComponent
Defines the entity's spatial properties in the world.
*   **position:** The `x, y` coordinates in world space.

### 2. RenderableComponent
Defines how the entity should look.
*   **spriteId:** A string identifier linking to a registered sprite definition.
*   **layer:** An integer for Z-sorting (higher values draw on top).
*   **color:** A tint color (e.g., for damage effects or environmental lighting).

## Sprite System

Sprites are not stored directly in components. Instead, components reference sprites by a string ID. The `SpriteManager` acts as a central registry.

### SpriteDesc
Describes a static image within a texture atlas.
*   **textureId:** The texture file identifier.
*   **uvRect:** The region of the texture to render (`sf::IntRect`).
*   **origin:** The pivot point of the sprite (default is top-left `0,0`).

### SpriteManager
Holds a map of `std::string` -> `SpriteDesc`.
*   **Usage:** `renderSystem.cpp` queries this manager to find the UV coordinates and texture for a given `spriteId`.

## Animation System

Animations are handled by swapping the `spriteId` in the `RenderableComponent` over time.

### AnimationClip
Defines a single animation sequence.
*   **name:** Unique identifier.
*   **spriteIds:** A list of sprite IDs to play in sequence.
*   **frameDuration:** Time in seconds for each frame.
*   **loop:** Boolean indicating if the animation repeats.

### AnimationComponent
Maintains the runtime state of an animation on an entity.
*   **currentClipId:** ID of the playing clip.
*   **timer:** Accumulates delta time.
*   **currentFrameIndex:** Tracks the current frame in the sequence.
*   **isPlaying / isFinished:** Control flags.

### AnimationSystem
The `AnimationSystem::update(deltaTime)` method:
1.  Iterates over all entities with both `AnimationComponent` and `RenderableComponent`.
2.  Updates the internal timer.
3.  Advances the frame index based on `frameDuration`.
4.  Updates the `RenderableComponent.spriteId` to match the current frame of the active `AnimationClip`.

This design ensures that the `RenderSystem` doesn't need to know about animations; it simply draws whatever sprite ID is currently assigned.

## Render Loop (`RenderSystem`)

The `RenderSystem::update()` method performs the following steps:

1.  **Cull & Collect:** Gathers all entities with `TransformComponent` and `RenderableComponent`.
2.  **Sort:** Sorts entities first by `layer`, then by Y-position (for top-down 2D depth sorting).
3.  **Draw:** Iterates through the sorted list, retrieving sprite data from `SpriteManager` and issuing draw calls to the `IRenderer`.

### Debug Drawing
The `RenderSystem` also supports a debug mode (toggleable via `setDebugDraw`). This mode visualizes:
*   Physics bodies (colliders)
*   Shapes (Circles, Rectangles) based on the underlying Box2D definitions.

## Interfaces

### IRenderer
The engine is agnostic to the specific rendering backend. The `IRenderer` interface defines primitives:
*   `drawSprite(...)`
*   `drawCircle(...)`
*   `drawRect(...)`
*   `clear(...)`, `beginFrame()`, `endFrame()`

The concrete implementation `SFMLRenderer` translates these calls into SFML commands.