#pragma once

#include "engine/core/IInputManager.hpp"
#include <memory>

namespace sf
{
    class RenderWindow;
}

namespace Bajabongo
{
    class ILoggerManager;
    class ILogger;
}

namespace engine {

    class SFMLInputManager : public IInputManager {
    public:
        SFMLInputManager(Bajabongo::ILoggerManager& logManager, sf::RenderWindow& window);
        void processEvents() override;
        bool isKeyPressed(engine::KeyCode key) const override;
        engine::Vector2i getMousePosition() const override;
    private:
        std::shared_ptr<Bajabongo::ILogger> m_logger;
        sf::RenderWindow& m_window;
    };

} // namespace engine