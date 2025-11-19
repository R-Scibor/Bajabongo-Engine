#pragma once

#include <memory>
#include <entt/fwd.hpp>
#include <box2d/id.h>

namespace engine {
    class IWindow;
    class IRenderer;
    class ILoggerManager;
    class IInputManager;
    class StateManager;
    class IResourceManager;
    class SpriteManager;

    /**
     * @brief A central DI container for all major engine services.
     */
    struct EngineContext {
        // --- Core Services ---
        std::shared_ptr<IWindow> m_window;
        std::shared_ptr<IRenderer> m_renderer;
        std::shared_ptr<ILoggerManager> m_logManager;
        std::shared_ptr<IInputManager> m_inputManager;

        // --- ECS and Physics ---
        std::shared_ptr<entt::registry> m_registry;
        b2WorldId m_physicsWorld;

        // --- Event Bus ---
        std::shared_ptr<entt::dispatcher> m_dispatcher;

        // --- State Manager ---
        StateManager* m_stateManager = nullptr;

        // --- Phase 5A Services ---
        std::shared_ptr<IResourceManager> m_resourceManager;
        std::shared_ptr<SpriteManager> m_spriteManager;
    };

} // namespace engine