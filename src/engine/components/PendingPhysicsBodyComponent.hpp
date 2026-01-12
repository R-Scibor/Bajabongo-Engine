#pragma once

#include "engine/core/math/MathAliases.hpp"
#include "engine/physics/PhysicsConstants.hpp"
#include <vector>

namespace engine
{
    struct FixtureDef {
        Vector2f size;
        Vector2f offset = {0.0f, 0.0f};
        float density = 1.0f;
        bool isSensor = false;
        unsigned int categoryBits = PhysicsCategory::Default;
        unsigned int maskBits = PhysicsCategory::All;
    };

    struct PendingPhysicsBodyComponent
    {
        Vector2f position;      // Initial position of the body
        
        // Body properties
        bool isStatic = false;  // True for static bodies, false for dynamic
        bool fixedRotation = false; // True to prevent body rotation
        float linearDamping = 0.0f; // Linear damping (air resistance / friction)
        bool isBullet = false;      // True if the body should be treated as a bullet (CCD)
        Vector2f initialVelocity = {0.0f, 0.0f}; // Initial linear velocity
        float rotation = 0.0f; // Initial rotation in radians

        // Support for multiple fixtures (shapes)
        std::vector<FixtureDef> fixtures;

        // Compatibility constructor/helper for single-fixture bodies
        // This is what the existing code was using implicitly
        // We can add a method or just rely on manual construction if needed, 
        // but for now, we'll refactor the creation code.
    };
}
