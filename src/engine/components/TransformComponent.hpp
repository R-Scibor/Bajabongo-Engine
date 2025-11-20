#pragma once

#include "engine/core/math/MathAliases.hpp"

namespace engine
{
    struct TransformComponent
    {
        Vector2f position;
        float rotation = 0.0f;
        Vector2f scale = { 1.0f, 1.0f };
    };
}