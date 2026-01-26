#pragma once

#include <entt/fwd.hpp>
#include <memory>

namespace engine
{
    class ILogger;
    struct EngineContext;

    /**
     * @class HierarchySystem
     * @brief System responsible for updating child transforms based on their parents.
     *
     * This system maintains the scene graph hierarchy by iterating over all entities
     * with a ParentComponent and updating their world TransformComponent. The child's
     * position and rotation are calculated relative to the parent's world transform
     * using the local offsets defined in the ParentComponent.
     *
     * @note This system should run after physics and animation updates, but before rendering,
     * to ensure that all attached entities are correctly positioned for the frame.
     */
    class HierarchySystem
    {
    public:
        /**
         * @brief Constructs a new HierarchySystem.
         *
         * @param context The engine context containing the registry and other subsystems.
         */
        explicit HierarchySystem(const EngineContext& context);

        /**
         * @brief Updates the hierarchy system.
         *
         * Iterates through all entities with ParentComponent and updates their
         * TransformComponent to reflect the parent's transform and local offsets.
         */
        void update();

        /**
         * @brief Callback for when an entity with a ChildComponent is destroyed.
         *
         * This function handles the cascading destruction of child entities when
         * their parent is destroyed.
         *
         * @param registry Reference to the EnTT registry.
         * @param entity The entity handle of the parent being destroyed.
         */
        void onParentDestroyed(entt::registry& registry, entt::entity entity);

    private:
        /** @brief Reference to the EnTT registry. */
        entt::registry& m_registry;
        
        /** @brief Logger instance for the HierarchySystem. */
        std::shared_ptr<ILogger> m_logger;
    };
}