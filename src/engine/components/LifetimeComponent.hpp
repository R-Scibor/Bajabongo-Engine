#pragma once

namespace engine
{
    /**
     * @brief Component that tracks the finite lifespan of an entity.
     * * Useful for temporary entities like projectiles, particle effects, or temporary power-ups.
     * The `LifetimeSystem` typically decrements this value and destroys the entity when it relies <= 0.
     */
    struct LifetimeComponent
    {
        /// @brief Remaining time before destruction, in seconds.
        float lifetime; 
    };
}