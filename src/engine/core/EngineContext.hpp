#pragma once

#include <memory>
#include <entt/fwd.hpp>
#include <box2d/id.h>
#include <string>
#include <vector>
#include <optional>
#include "engine/core/math/Random.hpp"

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
    class AudioSystem;

    /**
     * @brief Runtime toggleable debug flags.
     * * Controlled typically via the ImGui editor or config files.
     */
    struct DebugSettings {
        bool showPhysics = false;  ///< Draw physics colliders/shapes.
        bool showHitboxes = false; ///< Draw logic hitboxes (interaction zones).
        bool pauseGame = false;    ///< Pauses the update loop (but usually not rendering).
        bool showEditor = false;   ///< Toggles the main editor/inspector overlay.
    };

    /**
     * @brief A central Dependency Injection (DI) container for all major engine services.
     * * This struct acts as a Service Locator passed to Systems and States.
     * It ensures that core subsystems are accessible without using global singletons.
     * Most members are `std::shared_ptr` to ensure shared ownership and proper lifecycle management.
     */
    struct EngineContext {

        /**
         * @brief Configuration data for loading a specific game level/mission.
         * * Used to pass parameters from a MenuState to a GameplayState.
         */
        struct MissionConfig {
            std::string mapFile;     ///< Path to the level data file (e.g., "assets/data/warehouse.json").
            std::string displayName; ///< Human-readable name of the level (e.g., "Warehouse").
            // Add more if needed: difficulty, enemy count, etc.
        };

        /**
         * @brief Holds the configuration for the mission currently being loaded or played.
         * * Set this before pushing the GameplayState onto the stack.
         */
        std::optional<MissionConfig> currentMission;

        /// @brief Current state of debug visualization options.
        DebugSettings debugFlags;

        /**
         * @brief Game-provided asset locations used by engine services.
         *
         * The engine does not hardcode game asset paths. The composition root
         * fills this in before constructing Application (and before any texture
         * or font load). Empty values mean "no file fallback".
         */
        struct AssetPathConfig {
            /// Placeholder texture if a requested file cannot be loaded.
            /// If this is empty or also fails to load, a magenta square is generated.
            std::string missingTexturePath;
            /// Font files tried in order for the HUD (GuiService) and debug text
            /// (SFMLRenderer).
            std::vector<std::string> fontPaths;
        };
        AssetPathConfig assetPaths;

        // --- Core Services ---
        std::shared_ptr<IWindow> m_window;             ///< Window management interface.
        std::shared_ptr<IRenderer> m_renderer;         ///< Low-level rendering interface.
        std::shared_ptr<ILoggerManager> m_logManager;  ///< Central logging hub.
        std::shared_ptr<IInputManager> m_inputManager; ///< Input polling and state tracking.

        // --- ECS and Physics ---

        /**
         * @brief The EnTT registry.
         * * Stores all entities and their components. This is the heart of the ECS.
         */
        std::shared_ptr<entt::registry> m_registry;

        /// @brief Handle to the Box2D physics world simulation.
        b2WorldId m_physicsWorld;

        // --- Event Bus ---
        /// @brief Event dispatcher for cross-system communication.
        std::shared_ptr<entt::dispatcher> m_dispatcher;

        // --- State Manager ---
        /**
         * @brief Raw pointer to the StateManager.
         * * Kept as a raw pointer to avoid circular dependency issues (Application owns StateManager).
         */
        StateManager* m_stateManager = nullptr;

        // --- Asset Management (Phase 5A) ---
        std::shared_ptr<IResourceManager> m_resourceManager;  ///< Low-level resource cache (textures, fonts).
        std::shared_ptr<SpriteManager> m_spriteManager;       ///< High-level sprite logic.
        std::shared_ptr<AnimationLibrary> m_animationLibrary; ///< definitions of animations.
        std::shared_ptr<AssetManifestLoader> m_assetLoader;   ///< Loader for batch asset files.

        // --- Data-Driven Systems (Phase 6) ---
        std::shared_ptr<ArchetypeManager> m_archetypeManager; ///< Blueprint manager for entities.
        std::shared_ptr<EntityFactory> m_entityFactory;       ///< Factory for spawning entities from archetypes.

        // --- Tooling (Phase 5C) ---
        std::shared_ptr<GuiService> m_guiService; ///< Immediate Mode GUI (ImGui) service.

        // --- Audio (Phase 4) ---
        std::shared_ptr<AudioSystem> m_audioSystem; ///< Audio playback and management.

        // --- Randomness ---
        /// Shared generator. Seeded from random_device unless the composition root calls seed().
        Random m_random;

        // --- Global Settings ---
        static constexpr float MAP_SCALE = 2.0f; ///< Default scaling factor for map visuals.
        float mapScale = MAP_SCALE;              ///< Current scaling factor.
    };

} // namespace engine
