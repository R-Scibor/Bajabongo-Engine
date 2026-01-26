#pragma once

#include <entt/fwd.hpp>

namespace engine {
    struct EngineContext;

    /**
     * @class LifetimeSystem
     * @brief Manages the lifespan of temporary entities.
     *
     * This system decrements the lifetime counter of entities with a LifetimeComponent
     * and automatically destroys them when their lifetime expires (reaches zero or less).
     * Useful for projectiles, particle effects, and temporary markers.
     */
    class LifetimeSystem {
    public:
        /**
         * @brief Constructs a new LifetimeSystem.
         *
         * @param context The engine context containing the registry.
         */
        explicit LifetimeSystem(EngineContext& context);

        /**
         * @brief Updates the lifetime of entities.
         *
         * @param fixedDeltaTime The fixed time step for physics/logic updates.
         */
        void update(float fixedDeltaTime);

    private:
        /** @brief Reference to the engine context. */
        EngineContext& m_context;
    };
}