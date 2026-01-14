#pragma once

namespace engine
{
    // Box2D uses 32-bit integers for filtering
    // Each category must be a power of 2
    enum PhysicsCategory : unsigned int
    {
        None        = 0,
        Default     = 1 << 0,
        Player      = 1 << 1,
        Enemy       = 1 << 2,
        Wall        = 1 << 3,
        Projectile  = 1 << 4,
        LowObstacle = 1 << 5, // Fences, tables (blocks movement, allows shooting)
        Sensor      = 1 << 6,
        Hurtbox     = 1 << 7,
        VisibilityBlocker = 1 << 8,
        All         = 0xFFFFFFFF
    };
}
