#pragma once

#include "engine/core/IInputManager.hpp"
#include <memory>

namespace sf {
    class RenderWindow;
    class Event;
}

namespace engine {
    class ILoggerManager;
    class ILogger;

    class SFMLInputManager : public IInputManager {
    public:
        SFMLInputManager(ILoggerManager& logManager, sf::RenderWindow& window);
        void processEvent(const sf::Event& event) override;
        void update() override;
        bool isKeyPressed(engine::KeyCode key) const override;
        bool isMouseButtonPressed(engine::MouseCode button) const override;
        engine::Vector2i getMousePosition() const override;
        float consumeMouseScrollDelta() override;
    private:
        std::shared_ptr<ILogger> m_logger;
        sf::RenderWindow& m_window;
        float m_scrollDelta = 0.0f;
    };

} // namespace engine