#pragma once

#include <string>
#include <unordered_map>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>

namespace engine {

    /**
     * @brief Describes a single sprite within a larger texture (Texture Atlas).
     * * Instead of loading thousands of small image files, we load one big texture
     * and define multiple `SpriteDesc` pointing to regions of it.
     */
    struct SpriteDesc {
        /// @brief ID of the texture resource (loaded in ResourceManager).
        std::string textureId;
        
        /// @brief The region of the texture to display (Left, Top, Width, Height).
        sf::IntRect uvRect;      
        
        /**
         * @brief The pivot point of the sprite in local pixels.
         * * (0,0) is top-left. For a character standing on the ground, 
         * this is usually (width/2, height) [bottom-center].
         */
        sf::Vector2f origin;     
    };

    /**
     * @brief Registry of named sprite definitions.
     * * Decouples the game logic (which requests "Player_Idle_01") from 
     * the texture coordinates (which might change if the atlas is repacked).
     * * @note In Phase 5A, we register these manually in code. 
     * In Phase 5B, we will load them from data files (e.g., JSON or XML).
     */
    class SpriteManager {
    public:
        /**
         * @brief Registers a sprite definition under a unique ID.
         */
        void registerSprite(const std::string& id, const SpriteDesc& desc) {
            m_sprites[id] = desc;
        }

        /**
         * @brief Retrieves a sprite definition by ID.
         * @return Pointer to the descriptor, or nullptr if not found.
         */
        const SpriteDesc* getSprite(const std::string& id) const {
            auto it = m_sprites.find(id);
            if (it != m_sprites.end()) {
                return &it->second;
            }
            return nullptr;
        }

    private:
        std::unordered_map<std::string, SpriteDesc> m_sprites;
    };

} // namespace engine