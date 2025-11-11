#pragma once

#include "engine/core/input/KeyCode.hpp"
#include "engine/core/math/MathAliases.hpp"

namespace engine {

    class IInputManager {
    public:
        virtual ~IInputManager() = default;

        virtual bool isKeyPressed(engine::KeyCode key) const = 0;
        virtual engine::Vector2i getMousePosition() const = 0;
    };

} // namespace engine