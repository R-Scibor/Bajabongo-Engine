#include "SFMLResourceManager.hpp"
#include "engine/core/ILoggerManager.hpp"
#include <filesystem>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Color.hpp>

namespace engine {

    SFMLResourceManager::SFMLResourceManager(std::shared_ptr<ILoggerManager> loggerManager) {
        if (loggerManager) {
            m_logger = loggerManager->GetLogger("SFMLResourceManager");
        }
    }

    std::shared_ptr<TextureHandle> SFMLResourceManager::loadTexture(const std::string& id, const std::string& path) {
        // Check cache first
        auto it = m_textures.find(id);
        if (it != m_textures.end()) {
            return it->second;
        }

        // Load from disk
        auto texture = std::make_shared<sf::Texture>();
        if (!texture->loadFromFile(path)) {
            // Try user-specified fallback path
            if (texture->loadFromFile("../../assets/textures/eg.png")) {
                if (m_logger) {
                    m_logger->warn("Failed to load texture: {}. Used fallback: ../../assets/textures/eg.png", path);
                }
            } else {
                if (m_logger) {
                    m_logger->error("Failed to load texture: {} and fallback. Using generated 128x128 magenta.", path);
                }
                
                // Create 128x128 Magenta fallback
                sf::Image fallbackImage;
                // SFML 3 uses resize instead of create
                fallbackImage.resize({128, 128}, sf::Color::Magenta);
                (void)texture->loadFromImage(fallbackImage);
            }
        }

        // Force nearest-neighbor interpolation for crisp pixel art
        texture->setSmooth(false);

        // Cache and return
        m_textures[id] = texture;
        
        if (m_logger) {
            m_logger->info("Loaded texture: {} from {}", id, path);
        }
        
        return texture;
    }

    std::shared_ptr<TextureHandle> SFMLResourceManager::getTexture(const std::string& id) const {
        auto it = m_textures.find(id);
        if (it != m_textures.end()) {
            return it->second;
        }

        if (m_logger) {
            m_logger->error("Texture not found: {}", id);
        }
        return nullptr;
    }

} // namespace engine
