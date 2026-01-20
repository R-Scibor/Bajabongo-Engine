#pragma once

#include <SFML/Graphics/Color.hpp>

namespace game {

    struct HitFlashComponent {
        // Configuration
        float flashDuration = 0.15f;      // Total duration (hold + fade)
        float flashHoldDuration = 0.05f;  // How long to hold the solid flash color
        sf::Color flashColor = sf::Color::White;

        // State
        bool isFlashing = false;
        float currentTimer = 0.0f;
        sf::Color originalColor = sf::Color::White; // Stores the color before flashing
    };

}
