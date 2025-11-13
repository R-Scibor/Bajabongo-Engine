#pragma once

#include "engine/core/math/MathAliases.hpp"

namespace engine
{
    struct TransformComponent
    {
        Vector2f position;
        float rotation = 0.0f;
    };
}