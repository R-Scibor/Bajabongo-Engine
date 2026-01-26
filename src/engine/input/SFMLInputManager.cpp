#include "SFMLInputManager.hpp"
#include "engine/core/ILoggerManager.hpp"
#include "engine/core/ILogger.hpp"
#include "engine/core/input/MouseCode.hpp"
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

namespace {
    /**
     * @brief Helper function to map engine KeyCodes to SFML KeyCodes.
     * * This translation layer decouples the core engine logic from SFML constants.
     */
    sf::Keyboard::Key toSFMLKey(engine::KeyCode key) {
        switch (key) {
            case engine::KeyCode::A: return sf::Keyboard::Key::A;
            case engine::KeyCode::B: return sf::Keyboard::Key::B;
            case engine::KeyCode::C: return sf::Keyboard::Key::C;
            case engine::KeyCode::D: return sf::Keyboard::Key::D;
            case engine::KeyCode::E: return sf::Keyboard::Key::E;
            case engine::KeyCode::F: return sf::Keyboard::Key::F;
            case engine::KeyCode::G: return sf::Keyboard::Key::G;
            case engine::KeyCode::H: return sf::Keyboard::Key::H;
            case engine::KeyCode::I: return sf::Keyboard::Key::I;
            case engine::KeyCode::J: return sf::Keyboard::Key::J;
            case engine::KeyCode::K: return sf::Keyboard::Key::K;
            case engine::KeyCode::L: return sf::Keyboard::Key::L;
            case engine::KeyCode::M: return sf::Keyboard::Key::M;
            case engine::KeyCode::N: return sf::Keyboard::Key::N;
            case engine::KeyCode::O: return sf::Keyboard::Key::O;
            case engine::KeyCode::P: return sf::Keyboard::Key::P;
            case engine::KeyCode::Q: return sf::Keyboard::Key::Q;
            case engine::KeyCode::R: return sf::Keyboard::Key::R;
            case engine::KeyCode::S: return sf::Keyboard::Key::S;
            case engine::KeyCode::T: return sf::Keyboard::Key::T;
            case engine::KeyCode::U: return sf::Keyboard::Key::U;
            case engine::KeyCode::V: return sf::Keyboard::Key::V;
            case engine::KeyCode::W: return sf::Keyboard::Key::W;
            case engine::KeyCode::X: return sf::Keyboard::Key::X;
            case engine::KeyCode::Y: return sf::Keyboard::Key::Y;
            case engine::KeyCode::Z: return sf::Keyboard::Key::Z;
            case engine::KeyCode::Num0: return sf::Keyboard::Key::Num0;
            case engine::KeyCode::Num1: return sf::Keyboard::Key::Num1;
            case engine::KeyCode::Num2: return sf::Keyboard::Key::Num2;
            case engine::KeyCode::Num3: return sf::Keyboard::Key::Num3;
            case engine::KeyCode::Num4: return sf::Keyboard::Key::Num4;
            case engine::KeyCode::Num5: return sf::Keyboard::Key::Num5;
            case engine::KeyCode::Num6: return sf::Keyboard::Key::Num6;
            case engine::KeyCode::Num7: return sf::Keyboard::Key::Num7;
            case engine::KeyCode::Num8: return sf::Keyboard::Key::Num8;
            case engine::KeyCode::Num9: return sf::Keyboard::Key::Num9;
            case engine::KeyCode::Space: return sf::Keyboard::Key::Space;
            case engine::KeyCode::LShift: return sf::Keyboard::Key::LShift;
            case engine::KeyCode::RShift: return sf::Keyboard::Key::RShift;
            case engine::KeyCode::LControl: return sf::Keyboard::Key::LControl;
            case engine::KeyCode::RControl: return sf::Keyboard::Key::RControl;
            case engine::KeyCode::LAlt: return sf::Keyboard::Key::LAlt;
            case engine::KeyCode::RAlt: return sf::Keyboard::Key::RAlt;
            case engine::KeyCode::LSystem: return sf::Keyboard::Key::LSystem;
            case engine::KeyCode::RSystem: return sf::Keyboard::Key::RSystem;
            case engine::KeyCode::Enter: return sf::Keyboard::Key::Enter;
            case engine::KeyCode::Escape: return sf::Keyboard::Key::Escape;
            case engine::KeyCode::Tab: return sf::Keyboard::Key::Tab;
            // ... add all other key mappings here
            default: return sf::Keyboard::Key::Unknown;
        }
    }

    /**
     * @brief Helper function to map engine MouseCodes to SFML Mouse Buttons.
     */
    sf::Mouse::Button toSFMLMouse(engine::MouseCode button) {
        switch (button) {
            case engine::MouseCode::Left: return sf::Mouse::Button::Left;
            case engine::MouseCode::Right: return sf::Mouse::Button::Right;
            case engine::MouseCode::Middle: return sf::Mouse::Button::Middle;
            case engine::MouseCode::XButton1: return sf::Mouse::Button::Extra1;
            case engine::MouseCode::XButton2: return sf::Mouse::Button::Extra2;
            default: return sf::Mouse::Button::Left;
        }
    }
}

namespace engine {

    SFMLInputManager::SFMLInputManager(ILoggerManager& logManager, sf::RenderWindow& window)
        : m_window(window)
    {
        m_logger = logManager.GetLogger("Input");
        m_logger->info("SFMLInputManager initialized.");
    }

    void SFMLInputManager::processEvent(const sf::Event& event) {
        // This manager relies on polling for keys, but we keep this hook for event-driven inputs.
        // Currently, events are processed directly by the Application or forwarded to GuiService.
        
        // C++23 style check for variant-based SFML events (SFML 3.x)
        if (const auto* wheelEvent = event.getIf<sf::Event::MouseWheelScrolled>()) {
            if (wheelEvent->wheel == sf::Mouse::Wheel::Vertical) {
                m_scrollDelta += wheelEvent->delta;
            }
        }
    }

    void SFMLInputManager::update() {
        // We do NOT clear m_scrollDelta here anymore, because it needs to be consumed
        // by the logic systems (e.g. CameraSystem).
        // If we clear it here, and logic runs on a different frequency than input/render, we miss events.
    }

    bool SFMLInputManager::isKeyPressed(engine::KeyCode key) const {
        bool isPressed = sf::Keyboard::isKeyPressed(toSFMLKey(key));
        // Trace logging is commented out or used carefully to avoid flooding logs every frame
        /*
        if (isPressed) {
            m_logger->trace("Key {} is pressed.", static_cast<int>(key));
        }
        */
        return isPressed;
    }

    bool SFMLInputManager::isMouseButtonPressed(engine::MouseCode button) const {
        return sf::Mouse::isButtonPressed(toSFMLMouse(button));
    }

    engine::Vector2i SFMLInputManager::getMousePosition() const {
        // Get position relative to the active window (client area)
        sf::Vector2i sfmlPosition = sf::Mouse::getPosition(m_window);
        engine::Vector2i position = { sfmlPosition.x, sfmlPosition.y };
        return position;
    }

    float SFMLInputManager::consumeMouseScrollDelta() {
        float delta = m_scrollDelta;
        m_scrollDelta = 0.0f; // Reset after consumption
        return delta;
    }

} // namespace engine