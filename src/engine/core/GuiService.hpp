#pragma once

#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Time.hpp>

namespace engine {

    class GuiService {
    public:
        GuiService();
        ~GuiService();

        /**
         * @brief Initializes the ImGui-SFML context.
         */
        void Init(sf::RenderWindow& window);

        /**
         * @brief Shuts down the ImGui-SFML context.
         */
        void Shutdown();

        /**
         * @brief Handles SFML events for ImGui.
         * @return true if the event was consumed by the UI (mouse/keyboard captured), false otherwise.
         */
        bool HandleEvent(const sf::Window& window, const sf::Event& event);

        /**
         * @brief Starts a new ImGui frame.
         * @note Requires the window to update mouse position and display size.
         */
        void BeginFrame(sf::RenderWindow& window, const sf::Time& dt);

        /**
         * @brief Renders the ImGui draw data.
         */
        void Render(sf::RenderWindow& window);

    private:
        bool m_initialized = false;
    };

} // namespace engine