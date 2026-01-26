#pragma once

#include <memory>
#include <entt/fwd.hpp>

namespace engine
{
    struct EngineContext;
    class ILogger;

    /**
     * @class PhysicsSyncSystem
     * @brief Synchronizes Box2D physics simulation with engine TransformComponents.
     *
     * This system iterates over all entities that have both a PhysicsBodyComponent
     * and a TransformComponent. It updates the TransformComponent (position and rotation)
     * to match the current state of the Box2D body after the physics step.
     */
    class PhysicsSyncSystem {
    public:
        /**
         * @brief Constructs a new PhysicsSyncSystem.
         *
         * @param context The engine context containing the registry.
         */
        PhysicsSyncSystem(const EngineContext& context);

        /**
         * @brief Updates the system, syncing physics state to transform components.
         *
         * Retrieves the position and rotation from the Box2D body and applies it
         * to the entity's TransformComponent.
         */
        void update();

    private:
        /** @brief Reference to the EnTT registry. */
        entt::registry& m_registry;
        
        /** @brief Logger instance. */
        std::shared_ptr<ILogger> m_logger;
    };

} // namespace engine