# Input Management System

## 1. Overview

The Input Management system is designed to decouple the core game engine from any specific windowing or input library (such as SFML). It achieves this by following the **Dependency Inversion Principle**, one of the core tenets of SOLID design.

Instead of the high-level `Application` depending directly on low-level input libraries, both depend on a shared abstraction (`IInputManager`). This makes the engine more modular, easier to test, and allows for future platform changes (e.g., switching to SDL or a custom input library) without modifying the core engine code.

## 2. Architecture

The system is split into two main areas: **Core Abstractions** and **Concrete Implementations**.

### Core Abstractions (`src/engine/core/`)

These files define the "contract" for how input works within the Bajabongo Engine. They are completely platform-agnostic.

*   **[`IInputManager.hpp`](../src/engine/core/IInputManager.hpp:0)**: This is the central interface for the system. It defines a set of pure virtual functions that any concrete input manager must implement.
    *   `isKeyPressed(engine::KeyCode key) const`: Checks if a specific key is currently held down.
    *   `getMousePosition() const`: Returns the current mouse cursor position as an `engine::Vector2i`.

*   **[`input/KeyCode.hpp`](../src/engine/core/input/KeyCode.hpp:0)**: This file defines the `engine::KeyCode` enum. This is our own internal, abstract representation of all possible keyboard keys. Using our own enum ensures that the core engine never needs to know about the key codes of a specific library like `sf::Keyboard::Key`.

### Concrete Implementation (`src/engine/input/`)

This is the platform-specific code that implements the `IInputManager` interface. It acts as an **Adapter**, translating the engine's abstract calls into concrete calls for a specific library.

*   **[`SFMLInputManager.hpp`](../src/engine/input/SFMLInputManager.hpp:0) / [`SFMLInputManager.cpp`](../src/engine/input/SFMLInputManager.cpp:0)**: This is the implementation for SFML.
    *   It inherits from `engine::IInputManager`.
    *   The `.cpp` file is the **only place in the engine** that should include SFML's input headers (`<SFML/Window/Keyboard.hpp>`, `<SFML/Window/Mouse.hpp>`).
    *   It contains a mapping function (`toSFMLKey`) that translates an `engine::KeyCode` to the corresponding `sf::Keyboard::Key`.
    *   The implemented methods call the underlying SFML functions (e.g., `sf::Keyboard::isKeyPressed`).

## 3. Usage (Dependency Injection)

To use the input system, a concrete implementation must be created at the "composition root" of the application (typically `main()`) and injected into the classes that need it.

**Example from [`game.cpp`](../src/game/game.cpp:0):**

```cpp
#include "engine/core/Application.hpp"
#include "engine/rendering/SFMLRenderer.hpp"
#include "engine/input/SFMLInputManager.hpp" // 1. Include the concrete manager
#include "engine/logging/SpdlogManager.hpp"

int main() {
    // ... other initializations
    
    SFMLRenderer renderer;
    engine::SFMLInputManager inputManager; // 2. Create an instance of the concrete manager

    renderer.create("Game Window", 1280, 720);

    // 3. Inject the manager into the Application via its constructor
    Application app(renderer, renderer, logManager, inputManager);

    app.run();

    return 0;
}
```

Any system inside the `Application` can now access the input manager through its member variable (`m_inputManager`) and check for input in a platform-agnostic way.

**Example from [`Application.cpp`](../src/engine/core/Application.cpp:0):**

```cpp
void Application::processInput() {
    m_window.pollEvents();

    // Check for input using the abstract interface and our engine's key codes
    if (m_inputManager.isKeyPressed(engine::KeyCode::W))
    {
        m_logger->info("W key is pressed!");
    }
}