#pragma once

#include <memory>
#include <entt/fwd.hpp>
#include <box2d/id.h>

// Forward declarations of all engine services

namespace engine {
    class IWindow;
    class IRenderer;
    class ILoggerManager;
    class IInputManager;
    class StateManager; // Fwd-decl of the new manager

    /**
     * @brief A central DI container for all major engine services.
     *
     * This object is created in the
     * "composition root" (main.cpp) and passed by reference to all
     * systems and states that require access to core services.
     */
    struct EngineContext {
        // --- Core Services (from Phase 1-3) ---
        std::shared_ptr<IWindow> m_window;
        std::shared_ptr<IRenderer> m_renderer;
        std::shared_ptr<ILoggerManager> m_logManager;
        std::shared_ptr<IInputManager> m_inputManager;

        // --- ECS and Physics (from Phase 3) ---
        std::shared_ptr<entt::registry> m_registry;
        b2WorldId m_physicsWorld;

        // --- New Phase 4 Services ---

        /**
         * @brief The central event bus for asynchronous communication.
         *
         */
        std::shared_ptr<entt::dispatcher> m_dispatcher;

        /**
         * @brief A non-owning pointer to the StateManager.
         *
         * The Application owns the StateManager, but states need
         * this pointer to request transitions.
         *
         */
        StateManager* m_stateManager = nullptr;
    };

} // namespace engine