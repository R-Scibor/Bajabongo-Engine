# Bajabongo-Engine

A 2D game engine built in C++20.

## Project Status

*   **Current Milestone:** A "Walking Skeleton" architecture is complete. The engine can successfully initialize, run the main loop, and render a basic shape to the screen.
*   **Technology Stack:** C++20, SFML 3.0.2 (Development Branch)

## Core Architecture

The engine is built upon a foundation of modern C++ and SOLID design principles, emphasizing flexibility and separation of concerns.

### Key Achievements

1.  **State Management:**
    *   A stack-based `StateManager` now controls the application flow, with support for deferred transitions (push, pop, swap). This decouples the engine from game-specific logic. See the [State Management Documentation](docs/state_management.md) for more details.

2.  **Main Application Loop:**
    *   The core engine (`Application`) now delegates all input, update, and rendering logic to the active game state(s), acting as a simple driver for the `StateManager`.

3.  **Dependency Injection (DI) and Abstraction:**
    *   The `Application` class remains completely decoupled from any specific rendering or windowing library.
    *   It depends exclusively on abstract interfaces (`IWindow&`, `IRenderer&`, etc.), which are bundled into a central `EngineContext` struct.

4.  **Module Separation (Engine vs. Game):**
    *   The `game` project acts as the "Composition Root." It creates all concrete services and registers all game states.
    *   The `engine` library remains pure, with no knowledge of game-specific logic, ensuring it is reusable and self-contained.

### Architectural Adaptation

During initial development, the architecture was adapted to align with the design of the SFML library. The `SFMLRenderer` class now implements both the `IWindow` and `IRenderer` interfaces, managing a single `sf::RenderWindow` object. This change resolved a critical stability issue while fully preserving the core principles of abstraction and dependency injection.

## Next Steps

With the core architectural patterns (DI, ECS, State Management) in place, the next phase will focus on building out game features within this structure:

1.  Expanding the `GameplayState` to include more complex game logic.
2.  Creating more concrete states, such as a `PauseState` or `OptionsState`.
3.  Developing more sophisticated rendering techniques within the existing `RenderSystem`.

## Documentation

*   [State Management System](docs/state_management.md)
*   [Logging System Architecture](docs/logging.md)
*   [Core Math Library](docs/math.md)
*   [Physics System Architecture](docs/physics.md)