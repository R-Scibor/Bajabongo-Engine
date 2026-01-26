#pragma once

#include <string>
#include <SFML/Graphics/Color.hpp>

namespace engine
{
    /**
     * @brief Component containing data required to draw a 2D sprite for the entity.
     */
    struct RenderableComponent
    {
        /// @brief Resource ID for the sprite texture (looked up in ResourceManager).
        std::string spriteId;
        
        /**
         * @brief Rendering order depth.
         * * Higher values are drawn on top of lower values. 
         * * Useful for ensuring characters appear in front of background objects.
         */
        int layer = 0;

        /**
         * @brief Color tint applied to the sprite.
         * * Use `sf::Color::White` for the original texture color.
         * * Useful for damage flashes (Red) or transparency (Alpha channel).
         */
        sf::Color color = sf::Color::White;
    };
}