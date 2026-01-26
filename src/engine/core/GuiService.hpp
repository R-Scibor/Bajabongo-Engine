#pragma once

#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Time.hpp>
#include <memory>

namespace engine {

    class ILogger;

    /**
     * @brief Service wrapper for the Immediate Mode GUI (ImGui).
     * * Handles initialization, rendering, and event forwarding between SFML and ImGui.
     * Typically used for debug tools, level editors, and in-game overlays.
     */
    class GuiService {
    public:
        GuiService();
        ~GuiService();

        /**
         * @brief Sets the logger for this service to report initialization status and errors.
         * @param logger Shared pointer to the logger instance.
         */
        void SetLogger(std::shared_ptr<ILogger> logger);

        /**
         * @brief Initializes the ImGui-SFML context.
         * * Must be called before any other ImGui operations.
         * @param window Reference to the main SFML render window.
         */
        void Init(sf::RenderWindow& window);

        /**
         * @brief Shuts down the ImGui-SFML context and frees resources.
         */
        void Shutdown();

        /**
         * @brief Forwards SFML events to the ImGui system.
         * * This method checks if the UI wants to "capture" the input (e.g., typing in a text box
         * or clicking a window).
         * * @param window The window that received the event.
         * @param event The SFML event to process.
         * @return true if the event was consumed by the UI (mouse/keyboard captured).
         * @return false if the event should be passed through to the game logic.
         */
        bool HandleEvent(const sf::Window& window, const sf::Event& event);

        /**
         * @brief Starts a new ImGui frame.
         * * Calculates delta time and updates mouse/keyboard state for the UI.
         * @param window The render window (used for mouse position and size).
         * @param dt Time elapsed since the last frame.
         */
        void BeginFrame(sf::RenderWindow& window, const sf::Time& dt);

        /**
         * @brief Renders the ImGui draw data to the window.
         * * Should be called after drawing game objects but before displaying the window.
         * @param window The target render window.
         */
        void Render(sf::RenderWindow& window);

    private:
        bool m_initialized = false;
        std::shared_ptr<ILogger> m_logger;
    };

} // namespace engine