#include "SFMLResourceManager.hpp"
#include "engine/core/ILoggerManager.hpp"
#include <filesystem>

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
            if (m_logger) {
                m_logger->error("Failed to load texture: {}", path);
            }
            return nullptr;
        }

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
