#pragma once

#include "Vector2.hpp"
#include "Rect.hpp"

namespace engine
{
    // --- Vector Aliases ---

    /// @brief 2D Vector with float precision. Standard for positions and physics.
    using Vector2f = Vector2<float>;

    /// @brief 2D Vector with integer precision. Useful for grid coordinates.
    using Vector2i = Vector2<int>;

    /// @brief 2D Vector with unsigned integer precision. Useful for sizes and indices.
    using Vector2u = Vector2<unsigned int>;

    // --- Rect Aliases ---

    /// @brief Rectangle with float coordinates. Standard for bounding boxes.
    using Rectf = Rect<float>;

    /// @brief Rectangle with integer coordinates. Useful for UI or tile regions.
    using Recti = Rect<int>;
}