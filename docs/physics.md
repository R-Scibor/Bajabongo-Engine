# Physics System Architecture

## 1. Overview

The physics system in the Bajabongo Engine is responsible for simulating realistic physical interactions between game objects. It is built upon the powerful **Box2D 3.0** library and is tightly integrated with our **Entity-Component-System (ECS)** architecture, powered by the **EnTT** library.

The design emphasizes a clean, data-driven approach where the state of the physics world is a direct result of the components attached to entities.

## 2. Architecture & Design

The system is centered around the new handle-based API of **Box2D 3.0**, which offers improved performance and a safer, more modern interface compared to older pointer-based versions.

### Core Systems

- **[`PhysicsBodyCreationSystem`](../src/engine/physics/PhysicsBodyCreationSystem.hpp:0)**: This system is responsible for creating physics bodies in the Box2D world. It queries for entities that have a `PendingPhysicsBodyComponent` and creates a corresponding `b2Body` in the physics world. After creation, it adds a `PhysicsBodyComponent` to the entity and removes the `PendingPhysicsBodyComponent`.

- **[`PhysicsSyncSystem`](../src/engine/physics/PhysicsSyncSystem.hpp:0)**: This system synchronizes the state of the `TransformComponent` with the state of the physics body in the Box2D world. It iterates over all entities that have both a `TransformComponent` and a `PhysicsBodyComponent`, and updates the entity's position and rotation to match the physics simulation.

- **[`PhysicsEventSystem`](../src/engine/physics/PhysicsEventSystem.hpp:0)**: This system bridges the gap between Box2D's C-API event polling and the engine's C++ ECS event architecture. It polls Box2D for contact and sensor events each frame and dispatches them as EnTT signals.

### Key Components for Entities

- **[`TransformComponent`](../src/engine/components/TransformComponent.hpp:0)**: Defines the position, rotation, and scale of an entity in the game world.
- **[`PendingPhysicsBodyComponent`](../src/engine/components/PendingPhysicsBodyComponent.hpp:0)**: A component that holds the definition for a physics body to be created. The `PhysicsBodyCreationSystem` uses this data to construct a `b2Body`.
- **[`PhysicsBodyComponent`](../src/engine/components/PhysicsBodyComponent.hpp:0)**: This component is added to an entity after its physics body has been created. It stores the `b2BodyId` handle, linking the entity to its representation in the physics world.

## 3. The Box2D 3.0 API

A significant aspect of our physics integration is the strict adherence to the **Box2D 3.0 API**. This version marks a major shift from the traditional pointer-based design (`b2Body*`) to a handle-based system (`b2BodyId`).

### Why Box2D 3.0?

- **Safety**: Handles are safer than raw pointers. They prevent common issues like dangling pointers and make the lifetime management of physics objects more robust.
- **Performance**: The new API is designed for better performance, especially in scenarios with many objects.
- **Modern C++**: The API aligns better with modern C++ practices.

All interactions with Box2D are done through functions that accept these ID handles, such as `b2CreateBody()`, `b2Body_SetTransform()`, etc.

## 4. Physics Events

The engine supports two types of physics events, which are dispatched via the EnTT dispatcher:

1.  **Contact Events**: Fired when two solid bodies collide.
    *   `PhysicsContactBeginEvent`
    *   `PhysicsContactEndEvent`

2.  **Sensor Events**: Fired when a body enters or exits a sensor (trigger) volume.
    *   `PhysicsSensorBeginEvent`
    *   `PhysicsSensorEndEvent`

**Note on Sensors in Box2D 3.0:** To ensure reliable sensor detection, `enableSensorEvents` is enabled for all shapes in `PhysicsBodyCreationSystem`. This ensures that dynamic bodies (visitors) are correctly detected when they enter sensor volumes.

### Example: Listening for Sensor Events

```cpp
// in GameplayState::onEnter or similar
context.m_dispatcher->sink<engine::PhysicsSensorBeginEvent>().connect<&GameplayState::onSensorBegin>(this);

// Callback method
void GameplayState::onSensorBegin(const engine::PhysicsSensorBeginEvent& event) {
    m_logger->info("Entity {} entered Sensor {}", 
        entt::to_integral(event.visitorEntity), 
        entt::to_integral(event.sensorEntity));
}
```

## 5. How to Create a Physics-Enabled Entity

Creating a physics-enabled entity is a simple, data-driven process:

1.  Create an entity in the `entt::registry`.
2.  Add a `TransformComponent` to define its initial position.
3.  Add a `PendingPhysicsBodyComponent` with the desired physics properties (e.g., body type, shape, density).

The engine systems will automatically handle the rest.

### Example

```cpp
// In your game logic, where you have access to the entt::registry
entt::registry registry;

// 1. Create an entity
auto entity = registry.create();

// 2. Add a TransformComponent
auto& transform = registry.emplace<engine::TransformComponent>(entity);
transform.position = {100.0f, 50.0f};
transform.rotation = 0.0f;

// 3. Add a PendingPhysicsBodyComponent to trigger creation
auto& pendingBody = registry.emplace<engine::PendingPhysicsBodyComponent>(entity);
pendingBody.position = {100.0f, 50.0f};
pendingBody.size = {10.0f, 10.0f};
pendingBody.isStatic = false;
pendingBody.density = 1.0f;
pendingBody.isSensor = false; // Set to true for triggers

// The PhysicsBodyCreationSystem will now automatically create a Box2D body
// for this entity on its next update. The PhysicsSyncSystem will then keep
// the TransformComponent updated with the physics simulation.
```

This approach keeps the game logic clean and decoupled from the low-level details of the physics engine.