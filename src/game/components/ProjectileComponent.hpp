#pragma once

namespace game
{
    struct ProjectileComponent
    {
        float damage = 10.0f;
        // Lifetime is handled by LifetimeComponent
    };
}