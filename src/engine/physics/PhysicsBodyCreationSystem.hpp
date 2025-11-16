#pragma once

#include <memory>
#include <entt/fwd.hpp>
#include <box2d/id.h>

namespace engine
{
    struct EngineContext;
    class ILogger;

    class PhysicsBodyCreationSystem {
    public:
        PhysicsBodyCreationSystem(const EngineContext& context);
        void update();

    private:
        entt::registry& m_registry;
        b2WorldId m_worldId;
        std::shared_ptr<ILogger> m_logger;
    };

} // namespace engine