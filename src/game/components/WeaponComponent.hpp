#pragma once

namespace game
{
    struct WeaponComponent
    {
        float cooldownTimer = 0.0f;
        float fireRate = 0.2f;      // Time between shots in seconds
        float projectileSpeed = 300.0f;
        float projectileLifetime = 2.0f;
        float damage = 10.0f;
    };
}