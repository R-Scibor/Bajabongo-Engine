#pragma once

#include <memory>
#include <entt/fwd.hpp>
#include <box2d/id.h>

namespace engine
{
    class ILoggerManager;
    class ILogger;

    class PhysicsBodyCreationSystem {
    public:
        PhysicsBodyCreationSystem(entt::registry& registry, b2WorldId worldId, ILoggerManager& logManager);
        void update();

    private:
        entt::registry& m_registry;
        b2WorldId m_worldId;
        std::shared_ptr<ILogger> m_logger;
    };

} // namespace engine