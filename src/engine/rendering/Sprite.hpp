#pragma once

#include <string>
#include <unordered_map>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>

namespace engine {

    /**
     * @brief Describes a sprite's properties within a texture atlas.
     */
    struct SpriteDesc {
        std::string textureId;
        sf::IntRect uvRect;      // The region of the texture to use
        sf::Vector2f origin;     // The pivot point (default 0,0 is top-left)
    };

    /**
     * @brief Manages a registry of named sprite definitions.
     * 
     * In Phase 5A, we register these manually in code. 
     * In Phase 5B, we will load them from data files.
     */
    class SpriteManager {
    public:
        void registerSprite(const std::string& id, const SpriteDesc& desc) {
            m_sprites[id] = desc;
        }

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
