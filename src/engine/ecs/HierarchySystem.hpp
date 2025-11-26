#pragma once

#include <entt/fwd.hpp>
#include <memory>

namespace engine
{
    class ILogger;
    struct EngineContext;

    /**
     * @brief System responsible for updating child transforms based on their parents.
     *
     * This system iterates over all entities with a ParentComponent and updates their
     * World TransformComponent to match the parent's World Transform + Local Offsets.
     * 
     * Runs after Physics/Animation, but before Rendering.
     */
    class HierarchySystem
    {
    public:
        explicit HierarchySystem(const EngineContext& context);

        void update();

        /**
         * @brief Callback for when an entity with a ChildComponent is destroyed.
         *        Recursively destroys all children.
         */
        void onParentDestroyed(entt::registry& registry, entt::entity entity);

    private:
        entt::registry& m_registry;
        std::shared_ptr<ILogger> m_logger;
    };
}