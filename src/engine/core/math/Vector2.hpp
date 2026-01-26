#pragma once

namespace engine
{
    /**
     * @brief Generic 2D vector structure.
     * * Represents a point or a direction in 2D space.
     * @tparam T The arithmetic type (float, int, unsigned int).
     */
    template<typename T>
    struct Vector2
    {
        T x; ///< X component.
        T y; ///< Y component.

        /// @brief Default constructor. Initializes to (0,0).
        Vector2() : x(0), y(0) {}

        /// @brief Parameterized constructor.
        Vector2(T x, T y) : x(x), y(y) {}

        /// @brief Adds another vector to this one component-wise.
        Vector2<T>& operator+=(const Vector2<T>& other)
        {
            x += other.x;
            y += other.y;
            return *this;
        }

        /// @brief Subtracts another vector from this one component-wise.
        Vector2<T>& operator-=(const Vector2<T>& other)
        {
            x -= other.x;
            y -= other.y;
            return *this;
        }
    };

    // --- Global Operators ---

    /**
     * @brief Component-wise addition of two vectors.
     * @return New vector (left.x + right.x, left.y + right.y).
     */
    template<typename T>
    Vector2<T> operator+(const Vector2<T>& left, const Vector2<T>& right)
    {
        return Vector2<T>(left.x + right.x, left.y + right.y);
    }

    /**
     * @brief Component-wise subtraction of two vectors.
     * @return New vector (left.x - right.x, left.y - right.y).
     */
    template<typename T>
    Vector2<T> operator-(const Vector2<T>& left, const Vector2<T>& right)
    {
        return Vector2<T>(left.x - right.x, left.y - right.y);
    }

    /**
     * @brief Scalar multiplication.
     * @return New vector scaled by the scalar value.
     */
    template<typename T, typename U>
    Vector2<T> operator*(const Vector2<T>& left, U scalar)
    {
        return Vector2<T>(left.x * scalar, left.y * scalar);
    }

    /**
     * @brief Scalar division.
     * @return New vector divided by the scalar value.
     */
    template<typename T, typename U>
    Vector2<T> operator/(const Vector2<T>& left, U scalar)
    {
        return Vector2<T>(left.x / scalar, left.y / scalar);
    }
}