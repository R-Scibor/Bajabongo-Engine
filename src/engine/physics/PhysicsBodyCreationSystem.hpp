#pragma once

#include <memory>
#include <entt/fwd.hpp>
#include <box2d/id.h>

namespace engine
{
    struct EngineContext;
    class ILogger;

    /**
     * @class PhysicsBodyCreationSystem
     * @brief System responsible for creating Box2D physics bodies.
     *
     * This system detects entities with a PendingPhysicsBodyComponent, creates the
     * corresponding Box2D body and shapes, and then replaces the pending component
     * with a live PhysicsBodyComponent containing the body ID.
     */
    class PhysicsBodyCreationSystem {
    public:
        /**
         * @brief Constructs a new PhysicsBodyCreationSystem.
         *
         * @param context The engine context containing the physics world and registry.
         */
        PhysicsBodyCreationSystem(const EngineContext& context);

        /**
         * @brief Updates the system, processing new pending bodies.
         *
         * Iterates through entities with PendingPhysicsBodyComponent, creates
         * the Box2D bodies, assigns user data, and adds the PhysicsBodyComponent.
         */
        void update();

    private:
        /** @brief Reference to the EnTT registry. */
        entt::registry& m_registry;
        
        /** @brief ID of the Box2D world. */
        b2WorldId m_worldId;
        
        /** @brief Logger instance. */
        std::shared_ptr<ILogger> m_logger;
    };

} // namespace engine