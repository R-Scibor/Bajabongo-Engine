#pragma once

#include "engine/core/IInputManager.hpp"

namespace engine {

    class SFMLInputManager : public IInputManager {
    public:
        bool isKeyPressed(engine::KeyCode key) const override;
        engine::Vector2i getMousePosition() const override;
    };

} // namespace engine