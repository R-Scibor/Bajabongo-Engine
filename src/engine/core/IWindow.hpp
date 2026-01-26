#pragma once
#include <string>
#include <optional>
#include "engine/core/math/MathAliases.hpp"

// Forward declaration for the event type
namespace sf {
    class Event;
}

namespace engine
{
    /**
     * @brief Defines a pure abstract interface for the application window.
     * * This decouples the engine core from the specific windowing library (SFML/GLFW).
     * * The Application class interacts exclusively with this interface.
     */
    class IWindow {
    public:
        virtual ~IWindow() = default;

        /**
         * @brief Creates and displays the OS window.
         * @param title The text displayed in the title bar.
         * @param width The width of the client area in pixels.
         * @param height The height of the client area in pixels.
         * @param fullscreen Whether to create the window in fullscreen exclusive mode.
         */
        virtual void create(const std::string& title, unsigned int width, unsigned int height, bool fullscreen = false) = 0;

        /**
         * @brief Closes the window and destroys the context.
         */
        virtual void close() = 0;

        /**
         * @brief Checks if the window is currently open and valid.
         * @return true if the window exists, false otherwise.
         */
        virtual bool isOpen() const = 0;

        /**
         * @brief Polls the window for a single pending event.
         * * Wrapper around the OS message pump.
         * @return An optional containing the event if one was popped from the queue, otherwise std::nullopt.
         */
        virtual std::optional<sf::Event> pollEvent() = 0;

        /**
         * @brief Provides a native, platform-specific handle to the window.
         * * Required by some external libraries (like ImGui or separate rendering backends).
         * @return A raw void pointer to the native handle (e.g., HWND on Windows, Window on X11).
         */
        virtual void* getNativeHandle() const = 0;

        /**
         * @brief Returns the current dimensions of the window's client area.
         * @return Vector2u containing (width, height).
         */
        virtual Vector2u getSize() const = 0;

        /**
         * @brief Limits the framerate to a maximum fixed frequency.
         * * Useful for preventing 100% GPU usage in simple scenes.
         * @param limit The target framerate (e.g., 60). 0 disables the limit.
         */
        virtual void setFramerateLimit(unsigned int limit) = 0;
    };
}