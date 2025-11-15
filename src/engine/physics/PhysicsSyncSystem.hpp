#pragma once

#include <memory>
#include <entt/fwd.hpp>

namespace engine
{
    struct EngineContext;
    class ILogger;

    class PhysicsSyncSystem {
    public:
        PhysicsSyncSystem(const EngineContext& context);
        void update();

    private:
        entt::registry& m_registry;
        std::shared_ptr<ILogger> m_logger;
    };

} // namespace engine