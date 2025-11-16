#pragma once
#include <string>
#include <optional>

// Forward declaration for the event type
namespace sf {
    class Event;
}

namespace engine
{
    /**
     * @brief Defines a pure abstract interface for a window.
     * This decouples the engine from any specific windowing library (like SFML).
     * The Application core will only interact with this interface.
     */
    class IWindow {
    public:
        virtual ~IWindow() = default;

        /**
         * @brief Creates and displays the window.
         * @param title The title of the window.
         * @param width The width of the window in pixels.
         * @param height The height of the window in pixels.
         */
        virtual void create(const std::string& title, unsigned int width, unsigned int height) = 0;

        /**
         * @brief Closes the window.
         */
        virtual void close() = 0;

        /**
         * @brief Checks if the window is currently open.
         * @return true if the window is open, false otherwise.
         */
        virtual bool isOpen() const = 0;

        /**
         * @brief Polls for a pending event.
         * @return An optional containing the event if one is pending, otherwise an empty optional.
         */
        virtual std::optional<sf::Event> pollEvent() = 0;

        /**
         * @brief Provides a native, platform-specific handle to the window.
         * This is used by the rendering backend to attach to the window.
         * @return A void pointer to the native window handle (e.g., HWND on Windows).
         */
        virtual void* getNativeHandle() const = 0;
    };
}