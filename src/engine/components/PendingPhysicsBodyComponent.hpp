#pragma once

#include "engine/core/math/MathAliases.hpp"
#include "engine/physics/PhysicsConstants.hpp"
#include <vector>

namespace engine
{
    /**
     * @brief Defines a single shape (fixture) attached to a physics body.
     */
    struct FixtureDef {
        Vector2f size;              ///< Dimensions of the shape (e.g., width/height for box).
        Vector2f offset = {0.0f, 0.0f}; ///< Offset from the body's center of mass.
        float density = 1.0f;       ///< Mass density (affects total mass of the body).
        bool isSensor = false;      ///< If true, detects overlap but produces no physical collision response.
        
        /// @brief Collision category bits (what I am).
        unsigned int categoryBits = PhysicsCategory::Default;
        
        /// @brief Collision mask bits (what I collide with).
        unsigned int maskBits = PhysicsCategory::All;
    };

    /**
     * @brief A temporary "blueprint" component used to request the creation of a physics body.
     * * The PhysicsSystem consumes this component, creates the actual Box2D body/fixtures,
     * and then typically replaces this component with a `PhysicsBodyComponent` (Runtime handle).
     */
    struct PendingPhysicsBodyComponent
    {
        Vector2f position;           ///< Initial world position of the body center.
        
        // --- Body Properties ---
        
        bool isStatic = false;       ///< If true, body has infinite mass and doesn't move (e.g., ground).
        bool fixedRotation = false;  ///< If true, prevents the physics engine from rotating the body.
        float linearDamping = 0.0f;  ///< Simulates air friction (slows down linear movement).
        
        /**
         * @brief Continuous Collision Detection (CCD) flag.
         * * Set to true for fast-moving objects (like bullets) to prevent tunneling through thin walls.
         */
        bool isBullet = false;      
        
        Vector2f initialVelocity = {0.0f, 0.0f}; ///< Starting linear velocity.
        float rotation = 0.0f;       ///< Initial rotation in radians.

        /// @brief List of shapes (fixtures) to attach to this body.
        std::vector<FixtureDef> fixtures;
    };
}