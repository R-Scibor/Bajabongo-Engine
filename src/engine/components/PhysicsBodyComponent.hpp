#pragma once

#include <box2d/id.h>

namespace engine
{
    /**
     * @brief Runtime component holding the handle to the live Box2D physics body.
     * * This component indicates that the entity is currently part of the physics simulation.
     * Use `bodyId` to apply forces, impulses, or query transforms via the Box2D API.
     */
    struct PhysicsBodyComponent
    {
        /// @brief The unique identifier (handle) for the Box2D body.
        b2BodyId bodyId;
    };
}