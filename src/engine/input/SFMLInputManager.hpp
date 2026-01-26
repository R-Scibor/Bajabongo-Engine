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

    /**
     * @brief Concrete implementation of IInputManager using the SFML library.
     * * This class acts as a bridge between the engine's abstract input definitions
     * (KeyCode, MouseCode) and SFML's specific input handling (sf::Keyboard, sf::Mouse).
     * It requires a reference to the active sf::RenderWindow to calculate 
     * relative mouse coordinates correctly.
     */
    class SFMLInputManager : public IInputManager {
    public:
        /**
         * @brief Constructs the input manager.
         * @param logManager Reference to the logger manager for creating the input logger.
         * @param window Reference to the main game window (required for mouse position relative to window).
         */
        SFMLInputManager(ILoggerManager& logManager, sf::RenderWindow& window);

        /**
         * @brief Processes SFML window events to update internal input state.
         * * Specifically handles event-driven inputs like Mouse Wheel scrolling,
         * which cannot be easily polled via `sf::Keyboard::isKeyPressed`.
         * @param event The SFML event popped from the window's event queue.
         */
        void processEvent(const sf::Event& event) override;

        /**
         * @brief Prepares the input manager for the next frame.
         * * Currently minimal implementation, as scroll delta clearing is handled
         * via `consumeMouseScrollDelta`.
         */
        void update() override;

        /**
         * @brief Checks if a specific key is currently held down using SFML polling.
         * @param key The engine-specific KeyCode.
         * @return true if pressed, false otherwise.
         */
        bool isKeyPressed(engine::KeyCode key) const override;

        /**
         * @brief Checks if a specific mouse button is currently held down using SFML polling.
         * @param button The engine-specific MouseCode.
         * @return true if pressed, false otherwise.
         */
        bool isMouseButtonPressed(engine::MouseCode button) const override;

        /**
         * @brief Gets the current mouse position relative to the window's client area.
         * @return Vector2i (x, y) coordinates.
         */
        engine::Vector2i getMousePosition() const override;

        /**
         * @brief Returns the accumulated scroll delta and resets the internal counter.
         * * This mechanism ensures that scroll events are not lost if the logic update
         * runs at a different frequency than the input polling.
         * @return The scroll amount (positive or negative).
         */
        float consumeMouseScrollDelta() override;

    private:
        std::shared_ptr<ILogger> m_logger;
        
        /// @brief Reference to the window, needed for relative mouse coordinates.
        sf::RenderWindow& m_window;
        
        /// @brief Accumulator for mouse wheel movement since the last consume call.
        float m_scrollDelta = 0.0f;
    };

} // namespace engine