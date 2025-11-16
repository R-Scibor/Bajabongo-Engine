#pragma once

#include <memory>
#include <entt/fwd.hpp> // Forward declaration for registry
#include <box2d/id.h>   // MUST include: b2WorldId cannot be forward-declared

#include "engine/core/EngineContext.hpp"
#include "engine/physics/PhysicsBodyCreationSystem.hpp"
#include "engine/physics/PhysicsSyncSystem.hpp"
#include "engine/rendering/RenderSystem.hpp"
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
        explicit Application(EngineContext& context);

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

        /**
         * @brief Hook called when a PhysicsBodyComponent is destroyed.
         *
         * Cleans up the associated Box2D body from the physics world.
         * @param entity The entity whose PhysicsBodyComponent was destroyed.
         */
        void onPhysicsBodyDestroyed(entt::registry& registry, entt::entity entity);

        // Core context reference
        EngineContext& m_context;

        // Core services (shared from EngineContext)
        std::shared_ptr<IWindow>        m_window;
        std::shared_ptr<IRenderer>      m_renderer;
        std::shared_ptr<ILoggerManager> m_logManager;
        std::shared_ptr<IInputManager>  m_inputManager;

        // State Management (owned by Application)
        StateManager m_stateManager;

        // ECS and Physics
        std::shared_ptr<entt::registry> m_registry;
        b2WorldId                       m_physicsWorld; // Box2D 3.0: lightweight opaque handle

        // The application's own logger instance
        std::shared_ptr<ILogger>        m_logger;

        // Physics and rendering systems are now managed within the active game state.

        // Fixed-step game loop
        const float m_physicsTimeStep = 1.0f / 60.0f;
        float       m_accumulator = 0.0f;
    };

} // namespace engine
