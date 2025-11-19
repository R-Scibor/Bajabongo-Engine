#pragma once

#include <string>
#include <memory>
#include <SFML/Graphics/Texture.hpp>

namespace engine {

    // For Phase 5A, we alias TextureHandle directly to sf::Texture.
    // In the future, this could be a wrapper class or a handle ID.
    using TextureHandle = sf::Texture;

    class IResourceManager {
    public:
        virtual ~IResourceManager() = default;

        /**
         * @brief Loads a texture from disk or returns a cached one.
         * @param id Unique identifier for the texture (e.g., "player_sheet").
         * @param path File path to the texture image.
         * @return Shared pointer to the texture, or nullptr if loading failed.
         */
        virtual std::shared_ptr<TextureHandle> loadTexture(const std::string& id, const std::string& path) = 0;

        /**
         * @brief Retrieves a previously loaded texture.
         * @param id Unique identifier for the texture.
         * @return Shared pointer to the texture, or nullptr if not found.
         */
        virtual std::shared_ptr<TextureHandle> getTexture(const std::string& id) const = 0;
    };

} // namespace engine
