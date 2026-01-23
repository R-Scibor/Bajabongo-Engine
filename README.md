# Bajabongo Engine

A modular, data-driven 2D game engine built with **C++20**, **SFML 3**, **EnTT**, and **Box2D 3.0**.

The project is structured into two distinct parts: the **Core Engine** (`src/engine`) which provides reusable systems and abstractions, and the **Game Module** (`src/game`) which implements specific gameplay logic, acting as the composition root.

## 🚀 Key Features

### Core Technology
*   **Modern C++20 Codebase**: Utilizes latest standard features for performance and safety.
*   **Entity Component System (ECS)**: Powered by [EnTT](https://github.com/skypjack/entt) for high-performance entity management.
*   **Physics Engine**: Integrated **Box2D 3.0** (C API) for reliable 2D physics simulation.
*   **Rendering**: **SFML 3.0.2** backend with support for sprite batching, animations, and camera control.
*   **Input System**: Abstracted input handling supporting keyboard and mouse.
*   **Logging**: Structured, multi-sink logging using [spdlog](https://github.com/gabime/spdlog).

### Engine Systems
*   **State Management**: Stack-based Finite State Machine (Push, Pop, Swap) handling game flow (Splash -> Menu -> Gameplay).
*   **Asset Pipeline**: JSON-based asset manifests with hot-reloading capabilities using `tools/AssetBaker.py`.
*   **Event System**: Decoupled event dispatching for physics collisions, audio triggers, and state changes.
*   **Animation**: Component-based animation system supporting multiple clips and state-based transitions.

### Gameplay Features
*   **AI Behavior**: Robust AI system including:
    *   Enemy detection and state transitions (Idle, Alert, Combat).
    *   Behavioral archetypes: Patrol, Rush, Sniper, Turret.
    *   Pathfinding and obstacle avoidance.
*   **Combat System**:
    *   Weapon inventory and switching mechanics.
    *   Projectile ballistics and hit detection.
    *   Damage handling and visual feedback (Hit Flash).
*   **Visibility & Fog**: Dynamic visibility system ("Fog of War") based on player line-of-sight.
*   **Interactive Maps**: JSON-based map loading with defined spawn points, resources, and collision geometry.

## 📂 Project Structure

```
c:/repos/Bajabongo-Engine/
├── assets/                 # Game assets (sprites, fonts, data, shaders)
├── docs/                   # Detailed technical documentation
├── src/
│   ├── engine/             # Core Engine Library (Reusable)
│   │   ├── core/           # Application loop, DI, StateManager
│   │   ├── ecs/            # Entity Factory, Archetypes, Systems
│   │   ├── physics/        # Box2D integration
│   │   ├── rendering/      # SFML wrappers, Animation
│   │   └── ...
│   └── game/               # Game Logic (Implementation)
│       ├── ai/             # Enemy AI logic
│       ├── components/     # Game-specific components
│       ├── states/         # Game states (MainMenu, Gameplay)
│       ├── systems/        # Gameplay systems (Damage, Weapon, Fog)
│       └── game.cpp        # Composition Root (Main Entry)
└── tools/                  # Python tools for asset management
```

## 🛠️ Getting Started

### Prerequisites
*   **Visual Studio 2022** (or compatible C++20 compiler)
*   **vcpkg** package manager
*   **Python 3.x** (for asset tools)

### Building the Project
1.  Ensure **vcpkg** is installed and integrated with Visual Studio (`vcpkg integrate install`).
2.  Open `Bajabongo-Engine.sln` in Visual Studio.
3.  Select the **Debug** or **Release** configuration (x64 recommended).
4.  Build the solution (F7). Visual Studio should automatically invoke vcpkg to install missing dependencies (SFML, EnTT, Box2D, Spdlog).
5.  Run the `game` project.

## 📚 Documentation

Detailed documentation for specific subsystems can be found in the `docs/` directory:

*   [**Asset Pipeline**](docs/asset_pipeline.md): How to add and manage assets.
*   [**ECS Architecture**](docs/ecs.md): Entity-Component-System patterns and usage.
*   [**State Management**](docs/state_management.md): How the game flow is structured.
*   [**Rendering & Animation**](docs/rendering_animation.md): Sprite and animation details.
*   [**Physics System**](docs/physics.md): Box2D integration and collision handling.
*   [**Input System**](docs/input.md): Handling user input.
*   [**Event System**](docs/events.md): Event dispatching architecture.
*   [**Audio System**](docs/audio.md): Audio playback and management.
*   [**Logging**](docs/logging.md): Debugging and logging practices.
*   [**Math Library**](docs/math.md): Vector and geometry utilities.

## 🏗️ Architecture Overview

The engine follows **SOLID** principles, heavily utilizing **Dependency Injection (DI)**.
*   **EngineContext**: A central container holding references to core services (Renderer, Input, Registry, PhysicsWorld).
*   **Application Class**: Drives the main loop but delegates logic to the active `GameState`.
*   **Separation of Concerns**: The `engine` project knows nothing about `game` logic. The `game` project ties everything together.
