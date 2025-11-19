#pragma once

#include <string>
#include <SFML/Graphics/Color.hpp>

namespace engine
{
    struct RenderableComponent
    {
        // Replaces the old 'radius'
        std::string spriteId;
        
        // For sorting draw order (higher = on top)
        int layer = 0;

        // For tinting (e.g., damage flash, night time)
        sf::Color color = sf::Color::White;
    };
}
