# Bajabongo-Engine

A 2D game engine built in C++20.

## Project Status

*   **Current Milestone:** A "Walking Skeleton" architecture is complete. The engine can successfully initialize, run the main loop, and render a basic shape to the screen.
*   **Technology Stack:** C++20, SFML 3.0.2 (Development Branch)

## Core Architecture

The engine is built upon a foundation of modern C++ and SOLID design principles, emphasizing flexibility and separation of concerns.

### Key Achievements

1.  **Main Application Loop:**
    *   The core engine (`Application`) is fully operational, managing the game loop, calculating `deltaTime`, and orchestrating `processInput()`, `update()`, and `render()` calls in the correct sequence.

2.  **Dependency Injection (DI) and Abstraction:**
    *   The `Application` class is completely decoupled from any specific rendering or windowing library.
    *   It depends exclusively on abstract interfaces (`IWindow&` and `IRenderer&`), allowing different backends to be injected without changing the engine's core code.

3.  **Module Separation (Engine vs. Game):**
    *   The `game` project acts as the "Composition Root." It is the only part of the solution aware of concrete implementations like `SFMLRenderer`.
    *   The `engine` library remains pure, with no knowledge of game-specific logic, ensuring it is reusable and self-contained.

### Architectural Adaptation

During initial development, the architecture was adapted to align with the design of the SFML library. The `SFMLRenderer` class now implements both the `IWindow` and `IRenderer` interfaces, managing a single `sf::RenderWindow` object. This change resolved a critical stability issue while fully preserving the core principles of abstraction and dependency injection.

## Next Steps

The current codebase provides a solid and stable foundation for future development. The next phase will focus on:

1.  Integrating the **EnTT** library for an Entity-Component-System (ECS) architecture.
2.  Developing a data-driven **RenderSystem**.
3.  Replacing the temporary `drawShape()` method with a proper rendering pipeline based on entities and components (e.g., `TransformComponent`, `RenderableComponent`).

## Documentation

*   [Logging System Architecture](docs/logging.md)
*   [Core Math Library](docs/math.md)
*   [Physics System Architecture](docs/physics.md)