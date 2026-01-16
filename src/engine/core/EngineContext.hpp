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
    class AnimationLibrary;
    class AssetManifestLoader;
    class ArchetypeManager;
    class EntityFactory;
    class GuiService;

    struct DebugSettings {
        bool showPhysics = false;
        bool showHitboxes = false;
        bool pauseGame = false;
        bool showEditor = false;
    };

    /**
     * @brief A central DI container for all major engine services.
     */
    struct EngineContext {
        DebugSettings debugFlags;

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
        std::shared_ptr<AnimationLibrary> m_animationLibrary;
        std::shared_ptr<AssetManifestLoader> m_assetLoader;
        
        // --- Phase 6: Data-Driven Systems ---
        std::shared_ptr<ArchetypeManager> m_archetypeManager;
        std::shared_ptr<EntityFactory> m_entityFactory;

        // --- Phase 5C: Tooling ---
        std::shared_ptr<GuiService> m_guiService;

        // --- Global Settings ---
        static constexpr float MAP_SCALE = 2.0f; // Default map scale
        float mapScale = MAP_SCALE;
    };

} // namespace engine