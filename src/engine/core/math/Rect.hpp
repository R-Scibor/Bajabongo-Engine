#pragma once

namespace engine
{
    template<typename T>
    struct Rect
    {
        T left, top, width, height;

        Rect() : left(0), top(0), width(0), height(0) {}
        Rect(T left, T top, T width, T height) : left(left), top(top), width(width), height(height) {}
    };
}