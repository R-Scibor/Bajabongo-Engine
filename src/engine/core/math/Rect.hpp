#pragma once

namespace engine
{
    /**
     * @brief Generic axis-aligned rectangle structure.
     * @tparam T The arithmetic type for coordinates and dimensions (e.g., float, int).
     */
    template<typename T>
    struct Rect
    {
        T left;   ///< X coordinate of the left edge.
        T top;    ///< Y coordinate of the top edge.
        T width;  ///< Width of the rectangle.
        T height; ///< Height of the rectangle.

        /// @brief Default constructor. Creates a zero-sized rectangle at (0,0).
        Rect() : left(0), top(0), width(0), height(0) {}

        /// @brief Parameterized constructor.
        Rect(T left, T top, T width, T height) : left(left), top(top), width(width), height(height) {}
    };
}