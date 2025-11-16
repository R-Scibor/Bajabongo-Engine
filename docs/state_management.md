# State Management System

The engine features a robust, stack-based state management system designed to separate distinct parts of the game (like menus, gameplay, and pause screens) into self-contained units. This promotes clean architecture and makes it easy to manage the application's flow.

## Core Concepts

### The State Stack
The `StateManager` maintains a stack of game states. This stack-based approach allows for easy layering of states. For example, a `PauseState` can be pushed on top of a `GameplayState` without destroying the gameplay's underlying state.

- The state at the top of the stack is considered the "active" state. It is the only state that receives input events and logical updates.
- All states on the stack are rendered, from the bottom up. This allows an overlay state (like a pause menu) to be rendered on top of the state it's covering (like the main gameplay).

### Deferred Transitions
A key feature of the `StateManager` is its **deferred transition system**. When a state change is requested (e.g., pushing a new state or popping the current one), the action is not performed immediately. Instead, it is added to a queue of pending actions.

These actions are only processed at a specific, safe point in the main game loop. This design prevents a wide range of bugs, such as a state being destroyed while its `update` method is still executing.

---

## The `IGameState` Interface

All game states must inherit from the `engine::IGameState` abstract class and implement its five pure virtual methods. This interface defines the complete lifecycle of a state.

```cpp
// src/engine/core/IGameState.hpp

class IGameState {
public:
    virtual ~IGameState() = default;

    // Called once when the state is pushed onto the stack.
    virtual void onEnter(EngineContext& context) = 0;

    // Called once when the state is popped from the stack.
    virtual void onExit(EngineContext& context) = 0;

    // Handles SFML events for the active (top) state only.
    virtual void handleEvent(EngineContext& context, const sf::Event& event) = 0;

    // Updates game logic with a fixed time step for the active state.
    virtual void update(EngineContext& context, float fixedDeltaTime) = 0;

    // Renders the state. Called for all states on the stack.
    virtual void render(EngineContext& context) = 0;
};
```

- **`onEnter()`**: Use this for initialization. Set up UI, create entities, start music, etc.
- **`onExit()`**: Use this for cleanup. Destroy entities, save progress, stop music, etc.
- **`handleEvent()`**: Process user input. Only the top-most state receives these events.
- **`update()`**: Run game logic, physics, and other fixed-step updates. Only the top-most state is updated.
- **`render()`**: Draw the state's visuals. All states on the stack are rendered in order.

---

## The `StateManager` API

The `StateManager` is the central hub for controlling the application flow. It is accessible via the `EngineContext`.

### Registering a State
Before a state can be used, it must be registered with the `StateManager`'s factory. This is typically done once at startup in the composition root (`game.cpp`).

```cpp
// In game.cpp
context.m_stateManager->registerState<game::MainMenuState>("MainMenu");
context.m_stateManager->registerState<game::GameplayState>("Gameplay");
```
The template argument is the state's class name, and the string parameter is a unique identifier used to request the state.

### Requesting Transitions
There are two primary ways to request a state change:

**1. Direct API Call (Recommended for most UI/game logic)**
The `StateManager` provides a direct, clear API for requesting transitions.

```cpp
// Swap the current state with a new one
context.m_stateManager->requestSwap("Gameplay");

// Push a new state on top of the current one
context.m_stateManager->requestPush("PauseMenu");

// Pop the current state off the stack
context.m_stateManager->requestPop();

// Pop all states from the stack
context.m_stateManager->requestClear();
```

**2. Event Dispatcher (For decoupled systems)**
For more complex scenarios where a system should not have a direct dependency on the `StateManager`, you can enqueue an event.

```cpp
// Enqueue an event to be processed later in the frame
context.m_dispatcher->enqueue<engine::RequestStateSwapEvent>({ "Gameplay" });
```
The `StateManager` automatically listens for these events and adds the corresponding action to its pending queue.

---

## Tutorial: How to Create a `PauseState`

Here is a step-by-step guide to creating a new state.

**1. Create the Header File (`PauseState.hpp`)**
```cpp
// src/game/states/PauseState.hpp
#pragma once
#include "engine/core/IGameState.hpp"

namespace game {
    class PauseState : public engine::IGameState {
    public:
        void onEnter(engine::EngineContext& context) override;
        void onExit(engine::EngineContext& context) override;
        void handleEvent(engine::EngineContext& context, const sf::Event& event) override;
        void update(engine::EngineContext& context, float fixedDeltaTime) override;
        void render(engine::EngineContext& context) override;
    };
}
```

**2. Create the Source File (`PauseState.cpp`)**
```cpp
// src/game/states/PauseState.cpp
#include "PauseState.hpp"
#include "engine/core/EngineContext.hpp"
#include "engine/core/StateManager.hpp"
#include "engine/core/ILogger.hpp"
#include <SFML/Window/Event.hpp>

namespace game {
    void PauseState::onEnter(engine::EngineContext& context) {
        context.m_logManager->GetLogger("Game")->info("Entering PauseState.");
    }

    void PauseState::onExit(engine::EngineContext& context) {
        context.m_logManager->GetLogger("Game")->info("Exiting PauseState.");
    }

    void PauseState::handleEvent(engine::EngineContext& context, const sf::Event& event) {
        // When Escape is pressed again, pop this state to resume the game
        if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
            if (keyPressed->code == sf::Keyboard::Key::Escape) {
                context.m_stateManager->requestPop();
            }
        }
    }

    void PauseState::update(engine::EngineContext&, float) {
        // The pause menu is active, so no game logic should run.
    }

    void PauseState::render(engine::EngineContext& context) {
        // Draw "PAUSED" text or a menu overlay here.
        // The GameplayState beneath will still be rendered first.
    }
}
```

**3. Register the New State**
In `src/game/game.cpp`, add the registration call.
```cpp
// In game.cpp, during initialization
context.m_stateManager->registerState<game::PauseState>("Pause");
```

**4. Trigger the Transition**
In `GameplayState::handleEvent`, listen for the Escape key to push the `PauseState`.
```cpp
// In GameplayState.cpp
void GameplayState::handleEvent(engine::EngineContext& context, const sf::Event& event) {
    if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
        if (keyPressed->code == sf::Keyboard::Key::Escape) {
            context.m_stateManager->requestPush("Pause");
        }
    }
}
```

**5. Add to Project Files**
Finally, add `PauseState.hpp` and `PauseState.cpp` to the `game.vcxproj` file so they are included in the build.