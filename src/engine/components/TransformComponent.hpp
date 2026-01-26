#pragma once

#include "engine/core/math/MathAliases.hpp"

namespace engine
{
    /**
     * @brief Component representing the entity's spatial state in the world.
     * * This is the "source of truth" for where the entity is.
     * Systems (like Rendering or Physics) usually read from or write to this component.
     */
    struct TransformComponent
    {
        Vector2f position;           ///< World coordinates (X, Y).
        float rotation = 0.0f;       ///< Rotation (typically in radians for physics, degrees for SFML - ensure consistency).
        Vector2f scale = { 1.0f, 1.0f }; ///< Scale factor (1.0 is default size).
    };
}