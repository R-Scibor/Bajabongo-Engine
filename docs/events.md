# Event System

## 1. Overview

The engine utilizes the **EnTT** library's `dispatcher` for its event system. This provides a type-safe, decoupled way for systems to communicate with each other without direct dependencies.

## 2. Physics Events

`PhysicsEventSystem` reads Box2D contact and sensor events after each world step and enqueues them on the dispatcher. Queued events are delivered when `dispatcher->update()` runs. These events are defined in [`src/engine/events/PhysicsEvents.hpp`](../src/engine/events/PhysicsEvents.hpp).

### Event Types

| Event | Description | Members |
| :--- | :--- | :--- |
| **`PhysicsContactBeginEvent`** | Fired when two solid shapes begin touching. | `entityA`, `entityB` |
| **`PhysicsContactEndEvent`** | Fired when two solid shapes stop touching. | `entityA`, `entityB` |
| **`PhysicsSensorBeginEvent`** | Fired when a shape enters a sensor trigger. | `sensorEntity`, `visitorEntity`, `isVisitorSensor` |
| **`PhysicsSensorEndEvent`** | Fired when a shape exits a sensor trigger. | `sensorEntity`, `visitorEntity` |

### Usage Example

To listen for events, connect a listener to the specific event sink on the dispatcher:

```cpp
// Connect listener
context.m_dispatcher->sink<engine::PhysicsSensorBeginEvent>()
    .connect<&GameplayState::onSensorBegin>(this);

// Handler function
void GameplayState::onSensorBegin(const engine::PhysicsSensorBeginEvent& event) {
    // Logic here
}

// Disconnect when done
context.m_dispatcher->sink<engine::PhysicsSensorBeginEvent>().disconnect(this);
```

## 3. State Events

State management events are used to control the flow of game states (push, pop, swap, clear). These are defined in [`src/engine/events/StateEvents.hpp`](../src/engine/events/StateEvents.hpp).

- `RequestStatePushEvent`
- `RequestStatePopEvent`
- `RequestStateSwapEvent`
- `RequestStateClearEvent`

See [State Management](state_management.md) for more details.

## 4. Audio Events

Sound and music requests are defined in [`src/engine/events/AudioEvents.hpp`](../src/engine/events/AudioEvents.hpp): `PlaySoundEvent`, `StopSoundEvent`, `PlayMusicEvent`, `StopMusicEvent`, `SetVolumeEvent`, and `MusicFinishedEvent`. See [Audio](audio.md) for details.
