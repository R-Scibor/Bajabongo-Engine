# Entity Component System (ECS)

The Bajabongo-Engine uses [EnTT](https://github.com/skypjack/entt) as its ECS framework. This document outlines our specific architectural patterns and systems built on top of EnTT.

## Archetypes

We use a data-driven approach to define game entities. Archetypes are JSON definitions that specify which components an entity should have and their initial values.

**File:** `assets/data/archetypes.json`

### Structure

```json
{
  "soldier": {
    "Renderable": { "spriteId": "soldier_idle", "layer": 1 },
    "Physics": { "type": "dynamic", "size": [32, 64] },
    "Transform": { "rotation": 0.0 }
  }
}
```

### Composite Archetypes (Hierarchy)

Archetypes can define child entities, creating a hierarchy tree. The engine recursively spawns and attaches these children.

```json
{
  "soldier_with_gun": {
    "Renderable": { "spriteId": "soldier_idle" },
    "children": [
      {
        "archetype": "rifle",
        "offset": [10, 5],
        "rotation": 0
      }
    ]
  }
}
```

When `soldier_with_gun` is spawned:
1.  The **Soldier** entity is created.
2.  The **Rifle** entity is created recursively.
3.  The Rifle is automatically attached to the Soldier via `ParentComponent`.
4.  The Soldier receives a `ChildComponent` tracking the Rifle.

## Hierarchy System

The **Hierarchy System** manages the parent-child relationships between entities. It ensures child entities move and rotate with their parents and handles cascading destruction.

**Location:** `src/engine/ecs/HierarchySystem.hpp`

### Components

*   **`ParentComponent`**: Attached to the **Child**. Stores the parent ID and local offsets (position/rotation).
*   **`ChildComponent`**: Attached to the **Parent**. Stores a list of child entity IDs.

### Transform Synchronization

The system runs every frame (after Physics/Animation, before Rendering) to update child transforms:

`ChildWorldPos = ParentWorldPos + Rotate(LocalOffset * ParentScale, ParentRotation)`
`ChildWorldRot = ParentRotation + LocalRotation`

### Cascading Destruction

When a parent entity is destroyed, the engine automatically destroys all its children to prevent memory leaks and "orphaned" objects (e.g., floating guns).

This is handled via an event hook in the `GameplayState`:
```cpp
registry.on_destroy<engine::ChildComponent>()
        .connect<&engine::HierarchySystem::onParentDestroyed>(&m_hierarchySystem);
```

## Systems

(List of other major ECS systems can go here as we document them)
*   **PhysicsSystem**: Syncs Box2D bodies with TransformComponents.
*   **RenderSystem**: Draws entities with RenderableComponents.