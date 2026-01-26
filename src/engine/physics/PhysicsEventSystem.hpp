#pragma once

#include <box2d/id.h>
#include <entt/fwd.hpp>
#include <memory>

namespace engine
{
    struct EngineContext;
    class ILogger;

    /**
     * @class PhysicsEventSystem
     * @brief Polls Box2D for contact and sensor events and dispatches them via EnTT.
     *
     * This system bridges the gap between the C-API event polling of Box2D 3.0
     * and the C++ ECS event architecture. It converts low-level physical interactions
     * into high-level events that other systems can subscribe to.
     */
    class PhysicsEventSystem
    {
    public:
        /**
         * @brief Constructs a new PhysicsEventSystem.
         *
         * @param context The engine context containing necessary subsystems.
         */
        explicit PhysicsEventSystem(EngineContext& context);

        /**
         * @brief Pulls events from Box2D and pushes them to the event bus.
         *
         * This method retrieves contact and sensor events accumulated during the
         * last physics step and dispatches them using the EnTT dispatcher.
         *
         * @note Should be called AFTER b2World_Step but BEFORE game logic updates.
         */
        void update();

    private:
        /** @brief Polls for physical contact events (collisions) and dispatches them. */
        void pollContacts();
        
        /** @brief Polls for sensor trigger events and dispatches them. */
        void pollSensors();

        /** @brief ID of the Box2D world. */
        b2WorldId m_worldId;
        
        /** @brief Shared pointer to the event dispatcher. */
        std::shared_ptr<entt::dispatcher> m_dispatcher;
        
        /** @brief Shared pointer to the EnTT registry. */
        std::shared_ptr<entt::registry> m_registry;
        
        /** @brief Logger instance. */
        std::shared_ptr<ILogger> m_logger;
    };
}