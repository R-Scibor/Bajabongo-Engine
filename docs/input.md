# Input Management System

## 1. Overview

The Input Management system is designed to decouple the core game engine from any specific windowing or input library (such as SFML). It achieves this by following the **Dependency Inversion Principle**, one of the core tenets of SOLID design.

Instead of the high-level `Application` depending directly on low-level input libraries, both depend on a shared abstraction (`IInputManager`). This makes the engine more modular, easier to test, and allows for future platform changes (e.g., switching to SDL or a custom input library) without modifying the core engine code.

## 2. Architecture

The system is split into two main areas: **Core Abstractions** and **Concrete Implementations**.

### Core Abstractions (`src/engine/core/`)

These files define the "contract" for how input works within the Bajabongo Engine. Key and mouse codes are engine types. The one SFML type in the interface is `sf::Event`, which is forward-declared and passed to `processEvent`.

*   **[`IInputManager.hpp`](../src/engine/core/IInputManager.hpp)**: This is the central interface for the system. It defines a set of pure virtual functions that any concrete input manager must implement.
    *   `processEvent(const sf::Event& event)`: Feeds a window event to the manager (used for the mouse wheel).
    *   `update()`: Per-frame hook.
    *   `isKeyPressed(engine::KeyCode key) const`: Checks if a specific key is currently held down.
    *   `isMouseButtonPressed(engine::MouseCode button) const`: Checks if a mouse button is currently held down.
    *   `getMousePosition() const`: Returns the current mouse cursor position, relative to the window, as an `engine::Vector2i`.
    *   `consumeMouseScrollDelta()`: Returns the accumulated vertical wheel delta and resets it to zero.

*   **[`input/KeyCode.hpp`](../src/engine/core/input/KeyCode.hpp)** and **[`input/MouseCode.hpp`](../src/engine/core/input/MouseCode.hpp)**: These files define the `engine::KeyCode` and `engine::MouseCode` enums. They are our own internal, abstract representation of keyboard keys and mouse buttons. Using our own enums ensures that the core engine never needs to know about the key codes of a specific library like `sf::Keyboard::Key`.

### Concrete Implementation (`src/engine/input/`)

This is the platform-specific code that implements the `IInputManager` interface. It acts as an **Adapter**, translating the engine's abstract calls into concrete calls for a specific library.

*   **[`SFMLInputManager.hpp`](../src/engine/input/SFMLInputManager.hpp) / [`SFMLInputManager.cpp`](../src/engine/input/SFMLInputManager.cpp)**: This is the implementation for SFML.
    *   It inherits from `engine::IInputManager`.
    *   Its constructor takes an `ILoggerManager&` (to create its own "Input" logger) and the `sf::RenderWindow&` used for window-relative mouse positions.
    *   The `.cpp` file is the **only place in the engine** that includes SFML's keyboard and mouse headers (`<SFML/Window/Keyboard.hpp>`, `<SFML/Window/Mouse.hpp>`).
    *   It contains mapping functions (`toSFMLKey`, `toSFMLMouse`) that translate `engine::KeyCode` and `engine::MouseCode` to the corresponding SFML values.
    *   Keys and buttons are polled through SFML. `processEvent` only accumulates the vertical mouse wheel delta.

## 3. Usage (Dependency Injection)

To use the input system, a concrete implementation is created at the "composition root" of the application (`main()` in `game.cpp`) and injected through `EngineContext`.

**Simplified from [`game.cpp`](../src/game/game.cpp):**

```cpp
#include "engine/core/Application.hpp"
#include "engine/rendering/SFMLRenderer.hpp"
#include "engine/input/SFMLInputManager.hpp" // 1. Include the concrete manager
#include "engine/logging/SpdlogManager.hpp"

int main(int argc, char* argv[]) {
    auto logManager = std::make_shared<engine::SpdlogManager>("config/logging.ini");

    auto renderer = std::make_shared<engine::SFMLRenderer>();
    renderer->create("Game Window", 1920, 1080, true);

    // 2. Create the manager, injecting the log manager and the native window
    auto inputManager = std::make_shared<engine::SFMLInputManager>(*logManager, renderer->getNativeRenderWindow());

    // 3. Store it in the shared context
    auto context = std::make_shared<engine::EngineContext>();
    context->m_logManager = logManager;
    context->m_renderer = renderer;
    context->m_window = renderer;
    context->m_inputManager = inputManager;
    // ... registry, dispatcher, physics world, resource managers

    auto stateManager = std::make_unique<engine::StateManager>(*context);
    // ... register states and push the first one

    engine::Application app(context, std::move(stateManager));
    app.run();
    return 0;
}
```

States and systems read the manager from `EngineContext::m_inputManager` and check input in a platform-agnostic way. `Application` forwards each polled window event to `processEvent`.
