#pragma once

namespace engine
{
    template<typename T>
    struct Vector2
    {
        T x, y;

        Vector2() : x(0), y(0) {}
        Vector2(T x, T y) : x(x), y(y) {}

        Vector2<T>& operator+=(const Vector2<T>& other)
        {
            x += other.x;
            y += other.y;
            return *this;
        }

        Vector2<T>& operator-=(const Vector2<T>& other)
        {
            x -= other.x;
            y -= other.y;
            return *this;
        }
    };

    template<typename T>
    Vector2<T> operator+(const Vector2<T>& left, const Vector2<T>& right)
    {
        return Vector2<T>(left.x + right.x, left.y + right.y);
    }

    template<typename T>
    Vector2<T> operator-(const Vector2<T>& left, const Vector2<T>& right)
    {
        return Vector2<T>(left.x - right.x, left.y - right.y);
    }

    template<typename T, typename U>
    Vector2<T> operator*(const Vector2<T>& left, U scalar)
    {
        return Vector2<T>(left.x * scalar, left.y * scalar);
    }

    template<typename T, typename U>
    Vector2<T> operator/(const Vector2<T>& left, U scalar)
    {
        return Vector2<T>(left.x / scalar, left.y / scalar);
    }
}