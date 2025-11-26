#pragma once

#include <entt/fwd.hpp>
#include <memory>

namespace engine
{
    class ILogger;
    class EngineContext;

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

    private:
        entt::registry& m_registry;
        std::shared_ptr<ILogger> m_logger;
    };
}