#pragma once

#include "engine/core/math/MathAliases.hpp"

namespace engine
{
    struct PendingPhysicsBodyComponent
    {
        Vector2f position;      // Initial position of the body
        Vector2f size;          // Size of the box shape
        bool isStatic = false;  // True for static bodies, false for dynamic
        float density = 1.0f;   // Density of the shape (mass)
        bool isSensor = false;  // True if the shape is a sensor (no collision response)
        bool fixedRotation = false; // True to prevent body rotation
    };
}