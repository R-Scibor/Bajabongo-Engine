#pragma once

#include <entt/entity/entity.hpp>

namespace engine
{
    /**
     * @brief Fired when two solid physics shapes begin touching.
     */
    struct PhysicsContactBeginEvent
    {
        entt::entity entityA;
        entt::entity entityB;
    };

    /**
     * @brief Fired when two solid physics shapes stop touching.
     */
    struct PhysicsContactEndEvent
    {
        entt::entity entityA;
        entt::entity entityB;
    };

    /**
     * @brief Fired when a shape enters a sensor (trigger) volume.
     */
    struct PhysicsSensorBeginEvent
    {
        entt::entity sensorEntity;  // The entity that has the sensor shape
        entt::entity visitorEntity; // The entity that entered the sensor
    };

    /**
     * @brief Fired when a shape exits a sensor (trigger) volume.
     */
    struct PhysicsSensorEndEvent
    {
        entt::entity sensorEntity;
        entt::entity visitorEntity;
    };
}