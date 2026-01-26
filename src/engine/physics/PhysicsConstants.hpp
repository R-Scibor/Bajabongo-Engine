#pragma once

namespace engine
{
    /**
     * @enum PhysicsCategory
     * @brief Defines physics collision categories for filtering.
     *
     * These categories are used with Box2D's filtering system to determine which
     * objects should collide with each other. They are bit flags, so multiple
     * categories can be combined.
     */
    enum PhysicsCategory : unsigned int
    {
        None        = 0,        /**< No category */
        Default     = 1 << 0,   /**< Default category for objects */
        Player      = 1 << 1,   /**< Player character */
        Enemy       = 1 << 2,   /**< Enemy characters */
        Wall        = 1 << 3,   /**< Solid walls and static obstacles */
        Projectile  = 1 << 4,   /**< Projectiles like bullets or grenades */
        LowObstacle = 1 << 5,   /**< Low obstacles (fences, tables) that block movement but may allow shooting over */
        Sensor      = 1 << 6,   /**< Trigger areas that detect presence but don't physically block */
        Hurtbox     = 1 << 7,   /**< Vulnerable areas on characters */
        VisibilityBlocker = 1 << 8, /**< Objects that block line of sight (used by fog/visibility system) */
        All         = 0xFFFFFFFF /**< Matches all categories */
    };
}
