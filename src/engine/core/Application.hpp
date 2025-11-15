#pragma once

#include <memory>
#include <entt/fwd.hpp> // Forward declaration dla registry
#include <box2d/id.h>   // MUST include: b2WorldId cannot be forward-declared (C++ struct with layout)

#include "engine/core/EngineContext.hpp"
#include "engine/physics/PhysicsBodyCreationSystem.hpp"
#include "engine/physics/PhysicsSyncSystem.hpp"
#include "engine/rendering/RenderSystem.hpp"

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
        Application(const EngineContext& context);

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
         * @param deltaTime The time elapsed since the last frame, in seconds. (NO LONGER USED)
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
         * Cleans up the associated Box2D body from the physics world.
         * @param entity The entity whose PhysicsBodyComponent was destroyed.
		 */
        void onPhysicsBodyDestroyed(entt::registry& registry, entt::entity entity);

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

        // Physics Systems
        PhysicsBodyCreationSystem m_physicsBodyCreationSystem;
        PhysicsSyncSystem m_physicsSyncSystem;

        // Rendering System
        RenderSystem              m_renderSystem;

        // Fixed-step game loop
        const float m_physicsTimeStep = 1.0f / 60.0f;
        float m_accumulator = 0.0f;
    };
}
