#pragma once

#include <memory>
#include <entt/fwd.hpp> // Forward declaration dla registry
#include <box2d/id.h>   // MUST include: b2WorldId cannot be forward-declared (C++ struct with layout)

namespace engine {
    class IWindow;
    class IRenderer;
    class IInputManager;
    class ILoggerManager;
    class ILogger;

    /**
     * @brief The main Application class.
     * Orchestrates the main game loop and owns references to core engine services.
     * All dependencies are injected via constructor (Dependency Injection).
     */
    class Application {
    public:
        /**
         * @brief Constructs the Application, injecting core service dependencies.
         * @param window Reference to IWindow implementation.
         * @param renderer Reference to IRenderer implementation.
         * @param logManager Reference to ILoggerManager implementation.
         * @param inputManager Reference to IInputManager implementation.
         * @param registry Reference to EnTT registry (ECS core).
         * @param physicsWorld Box2D world ID (Box2D 3.0 uses opaque handles).
         */
        Application(
            IWindow& window,
            IRenderer& renderer,
            ILoggerManager& logManager,
            IInputManager& inputManager,
            entt::registry& registry,
            b2WorldId physicsWorld
        );

        ~Application();

        /**
         * @brief Starts and runs the main game loop.
         */
        void run();

    private:
        /**
         * @brief Processes all pending inputs for the current frame.
         */
        void processInput();

        /**
         * @brief Updates the game state for the current frame.
         * @param deltaTime The time elapsed since the last frame, in seconds.
         */
        void update(float deltaTime);

        /**
         * @brief Renders the game state for the current frame.
         */
        void render();

        // References to abstract interfaces for core services
        IWindow& m_window;
        IRenderer& m_renderer;
        ILoggerManager& m_logManager;
        IInputManager& m_inputManager;

        // ECS and Physics (injected dependencies)
        entt::registry& m_registry;
        b2WorldId m_physicsWorld; // Box2D 3.0: lightweight opaque handle (pass by value)

        // The application's own logger instance
        std::shared_ptr<ILogger> m_logger;
    };
}
