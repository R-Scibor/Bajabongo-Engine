#pragma once

#include <entt/fwd.hpp>
#include <box2d/id.h>

namespace engine
{
    class PhysicsBodyCreationSystem {
    public:
        PhysicsBodyCreationSystem(entt::registry& registry, b2WorldId worldId);
        void update();

    private:
        entt::registry& m_registry;
        b2WorldId m_worldId;
    };

} // namespace engine