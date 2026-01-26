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
     * * Orchestrates the main game loop (Input -> Update -> Render) and owns references to core engine services.
     * It implements a **Fixed Timestep** game loop to ensure physics consistency regardless of framerate.
     *
     * All dependencies are injected via constructor (Dependency Injection) through the EngineContext.
     */
    class Application {
    public:
        /**
         * @brief Constructs the Application, injecting core service dependencies.
         *
         * All dependencies are supplied through EngineContext, which is created
         * in the composition root (game module) and passed by reference.
         *
         * @param context Shared pointer to the EngineContext containing core systems (Window, Logger, etc.).
         * @param stateManager Unique pointer to the StateManager responsible for game states.
         */
        Application(
            std::shared_ptr<EngineContext> context,
            std::unique_ptr<StateManager> stateManager
        );

        /// @brief Destructor. Shuts down subsystems like GUI and releases resources.
        ~Application();

        /**
         * @brief Starts and runs the main game loop.
         * * This method blocks until the window is closed or the state stack becomes empty.
         * It manages the "Fixed Timestep" logic (processing physics in fixed chunks
         * while rendering as fast as possible).
         */
        void run();

    private:
        /**
         * @brief Processes all pending inputs for the current frame.
         * * Polls window events (SFML), updates the InputManager, and feeds events to the GUI and StateManager.
         */
        void processInput();

        /**
         * @brief Updates the game state for the current frame (Variable Timestep).
         * * Used for non-physics logic, event dispatching, and audio updates.
         */
        void update();

        /**
         * @brief Updates the physics simulation with a fixed time step.
         * * This ensures deterministic physics behavior. Called multiple times per frame if necessary.
         */
        void fixedUpdate();

        /**
         * @brief Renders the game state for the current frame.
         * * Clears the screen, delegates rendering to the active state, draws the GUI, and swaps buffers.
         */
        void render();

        // --- Dependencies ---

        /// @brief Core context holding references to global systems (owned shared pointer).
        std::shared_ptr<EngineContext> m_context;

        /// @brief State Management system (owned exclusively by Application).
        std::unique_ptr<StateManager> m_stateManager;

        /// @brief The application's own logger instance for "Core" messages.
        std::shared_ptr<ILogger> m_logger;

        // --- Game Loop Timing ---

        /// @brief The target duration for a single physics step (1/60th of a second).
        const float m_fixedTimestep = 1.0f / 60.0f;
        
        /// @brief Accumulator for elapsed time, used to run fixed updates in chunks.
        float       m_accumulator = 0.0f;

    };

} // namespace engine