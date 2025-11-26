#pragma once

#include <box2d/id.h>
#include <entt/fwd.hpp>
#include <memory>

namespace engine
{
    struct EngineContext;
    class ILogger;

    /**
     * @brief Polls Box2D for contact and sensor events and dispatches them via EnTT.
     * 
     * This system bridges the gap between the C-API event polling of Box2D 3.0
     * and the C++ ECS event architecture.
     */
    class PhysicsEventSystem
    {
    public:
        explicit PhysicsEventSystem(EngineContext& context);

        /**
         * @brief Pulls events from Box2D and pushes them to the event bus.
         * Should be called AFTER b2World_Step but BEFORE game logic updates.
         */
        void update();

    private:
        void pollContacts();
        void pollSensors();

        b2WorldId m_worldId;
        std::shared_ptr<entt::dispatcher> m_dispatcher;
        std::shared_ptr<entt::registry> m_registry;
        std::shared_ptr<ILogger> m_logger;
    };
}