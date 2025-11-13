#pragma once

#include <memory>
#include <entt/fwd.hpp>

namespace engine
{
    class ILoggerManager;
    class ILogger;

    class PhysicsSyncSystem {
    public:
        PhysicsSyncSystem(entt::registry& registry, ILoggerManager& logManager);
        void update();

    private:
        entt::registry& m_registry;
        std::shared_ptr<ILogger> m_logger;
    };

} // namespace engine