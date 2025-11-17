#pragma once

#include <memory>
#include <entt/fwd.hpp> // Forward declaration for registry
#include <box2d/id.h>   // MUST include: b2WorldId cannot be forward-declared

#include "engine/core/EngineContext.hpp"
#include "engine/core/StateManager.hpp"

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
         *
         * All dependencies are supplied through EngineContext, which is created
         * in the composition root (game module) and passed by reference.
         */
        Application(
            std::shared_ptr<EngineContext> context,
            std::unique_ptr<StateManager> stateManager
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
         *
         * Currently used for non-physics game logic.
         */
        void update();

        /**
         * @brief Updates the physics simulation with a fixed time step.
         */
        void fixedUpdate();

        /**
         * @brief Renders the game state for the current frame.
         */
        void render();

        // Core context (owned)
        std::shared_ptr<EngineContext> m_context;

        // State Management (owned by Application)
        std::unique_ptr<StateManager> m_stateManager;

        // The application's own logger instance
        std::shared_ptr<ILogger> m_logger;

        // Fixed-step game loop
        const float m_fixedTimestep = 1.0f / 60.0f;
        float       m_accumulator = 0.0f;

        // Physics and rendering systems are now managed within the active game state.

    };

} // namespace engine
