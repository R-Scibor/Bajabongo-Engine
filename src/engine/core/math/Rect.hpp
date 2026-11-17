#pragma once

#include "engine/core/math/Vector2.hpp"

namespace engine {
    /**
     * @brief Generic axis-aligned rectangle structure.
     * @tparam T The arithmetic type for coordinates and dimensions (e.g., float, int).
     */
    template<typename T>
    struct Rect {
        T left;   ///< X coordinate of the left edge.
        T top;    ///< Y coordinate of the top edge.
        T width;  ///< Width of the rectangle.
        T height; ///< Height of the rectangle.

        /// @brief Default constructor. Creates a zero-sized rectangle at (0,0).
        Rect() : left(0), top(0), width(0), height(0) {}

        /// @brief Parameterized constructor.
        Rect(T left, T top, T width, T height) : left(left), top(top), width(width), height(height) {}
    };

    /// @brief Exclusive-of-nothing right edge: left + width.
    template<typename T>
    T rectRight(const Rect<T>& r) {
        return r.left + r.width;
    }

    /// @brief Bottom edge: top + height (y grows downward in screen space).
    template<typename T>
    T rectBottom(const Rect<T>& r) {
        return r.top + r.height;
    }

    /// @brief Inclusive AABB point test. Edge points count as inside.
    template<typename T>
    bool contains(const Rect<T>& r, T x, T y) {
        return x >= r.left && x <= r.left + r.width && y >= r.top && y <= r.top + r.height;
    }

    template<typename T>
    bool contains(const Rect<T>& r, const Vector2<T>& point) {
        return contains(r, point.x, point.y);
    }

    /// @brief Axis-aligned overlap test. Touching edges count as an intersection.
    template<typename T>
    bool intersects(const Rect<T>& a, const Rect<T>& b) {
        return a.left <= b.left + b.width && a.left + a.width >= b.left && a.top <= b.top + b.height &&
               a.top + a.height >= b.top;
    }
} // namespace engine
