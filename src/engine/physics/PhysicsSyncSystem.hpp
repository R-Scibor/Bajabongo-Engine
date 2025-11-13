#pragma once

#include <entt/fwd.hpp>

namespace engine
{

    class PhysicsSyncSystem {
    public:
        PhysicsSyncSystem(entt::registry& registry);
        void update();

    private:
        entt::registry& m_registry;
    };

} // namespace engine