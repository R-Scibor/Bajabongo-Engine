#pragma once

#include "engine/core/EngineContext.hpp"
#include "engine/ecs/ArchetypeManager.hpp"
#include "engine/core/math/MathAliases.hpp"
#include <entt/entt.hpp>
#include <nlohmann/json.hpp>
#include <string>
#include <memory>
#include <functional>
#include <unordered_map>

namespace engine {

    class ILogger;

    /**
     * @class EntityFactory
     * @brief Responsible for creating entities from archetypes.
     *
     * The EntityFactory uses the ArchetypeManager to retrieve entity definitions
     * and constructs entities by adding the specified components. It supports
     * both core engine components and game-specific components via a registration system.
     */
    class EntityFactory {
    public:
        /** @brief Function signature for component loader callbacks. */
        using ComponentLoaderFn = std::function<void(entt::registry&, entt::entity, const nlohmann::json&)>;

        /**
         * @brief Constructs a new EntityFactory.
         *
         * @param context Reference to the engine context.
         * @param archetypeManager Shared pointer to the archetype manager.
         */
        EntityFactory(EngineContext& context, std::shared_ptr<ArchetypeManager> archetypeManager);

        /**
         * @brief Spawns a new entity based on an archetype ID.
         *
         * @param archetypeId The unique identifier of the archetype to instantiate.
         * @param position The initial world position for the new entity.
         * @return entt::entity The handle to the created entity, or entt::null if creation failed.
         */
        entt::entity spawn(const std::string& archetypeId, const Vector2f& position);

        /**
         * @brief Registers a custom loader function for a component type.
         *
         * @param componentName The name of the component as it appears in the JSON archetype definition.
         * @param loader The function to call when this component is encountered in an archetype.
         */
        void registerComponentLoader(const std::string& componentName, ComponentLoaderFn loader);

    private:
        /** @brief Reference to the engine context. */
        EngineContext& m_context;
        
        /** @brief Shared pointer to the archetype manager. */
        std::shared_ptr<ArchetypeManager> m_archetypeManager;
        
        /** @brief Logger instance for the EntityFactory. */
        std::shared_ptr<ILogger> m_logger;

        /** @brief Map of registered component loaders keyed by component name. */
        std::unordered_map<std::string, ComponentLoaderFn> m_componentRegistry;

        /**
         * @brief Registers the default set of engine component loaders.
         *
         * This includes loaders for Transform, Renderable, Physics, etc.
         */
        void registerDefaultLoaders();
    };

} // namespace engine