#pragma once

#include "engine/core/input/KeyCode.hpp"
#include "engine/core/input/MouseCode.hpp"
#include "engine/core/math/MathAliases.hpp"

namespace sf {
    class Event;
}

namespace engine {

    /**
     * @brief Abstract interface for the Input System.
     * * Decouples game logic from the underlying input library (SFML).
     * * Provides both event-based updates and polling methods for state checks.
     */
    class IInputManager {
    public:
        virtual ~IInputManager() = default;

        /**
         * @brief Processes a raw system event to update internal input state.
         * * Should be called inside the window's event polling loop.
         * @param event The raw SFML event (e.g., KeyPressed, MouseWheelScrolled).
         */
        virtual void processEvent(const sf::Event& event) = 0;

        /**
         * @brief Updates input state at the end/beginning of a frame.
         * * Crucial for resetting single-frame states like "Key Just Pressed" 
         * or clearing the accumulated mouse scroll delta.
         */
        virtual void update() = 0;

        /**
         * @brief Checks if a specific keyboard key is currently held down.
         * @param key The engine-specific KeyCode.
         * @return true if the key is down, false otherwise.
         */
        virtual bool isKeyPressed(engine::KeyCode key) const = 0;

        /**
         * @brief Checks if a specific mouse button is currently held down.
         * @param button The engine-specific MouseCode.
         * @return true if the button is down, false otherwise.
         */
        virtual bool isMouseButtonPressed(engine::MouseCode button) const = 0;

        /**
         * @brief Retrieves the current mouse cursor position relative to the window.
         * @return Vector2i containing (x, y) coordinates.
         */
        virtual engine::Vector2i getMousePosition() const = 0;

        /**
         * @brief Returns the accumulated mouse wheel scroll delta and resets it.
         * * This is a "consume" operation. Calling it twice in the same frame
         * will likely return 0.0f on the second call.
         * @return Float representing scroll amount (positive = up/forward, negative = down/backward).
         */
        virtual float consumeMouseScrollDelta() = 0;
    };

} // namespace engine