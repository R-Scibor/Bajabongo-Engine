#pragma once

#include "engine/core/math/MathAliases.hpp"

namespace engine
{
    struct PendingPhysicsBodyComponent
    {
        Vector2f position;
        Vector2f size;
        bool isStatic = false;
        float density = 1.0f;
    };
}