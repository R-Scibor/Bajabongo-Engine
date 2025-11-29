#pragma once

namespace engine
{
    /**
     * @brief Component that tracks the remaining lifetime of an entity.
     * When lifetime reaches 0, the entity should be destroyed by LifetimeSystem.
     */
    struct LifetimeComponent
    {
        float lifetime; // Remaining time in seconds
    };
}