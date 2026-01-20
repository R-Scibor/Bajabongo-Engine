#pragma once

#include <entt/entity/entity.hpp>

namespace game
{
    struct ProjectileComponent
    {
        float damage = 10.0f;
        entt::entity owner = entt::null;
        // Lifetime is handled by LifetimeComponent
    };
}