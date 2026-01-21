#include "SFMLResourceManager.hpp"
#include "engine/core/ILoggerManager.hpp"
#include <filesystem>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Audio/SoundBuffer.hpp>

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

    std::shared_ptr<SoundBufferHandle> SFMLResourceManager::loadSoundBuffer(const std::string& id, const std::string& path) {
        // Check cache first
        auto it = m_soundBuffers.find(id);
        if (it != m_soundBuffers.end()) {
            return it->second;
        }

        // Load from disk
        auto buffer = std::make_shared<sf::SoundBuffer>();
        if (!buffer->loadFromFile(path)) {
            if (m_logger) {
                m_logger->error("Failed to load sound buffer: {}", path);
            }
            return nullptr;
        }

        // Cache and return
        m_soundBuffers[id] = buffer;
        
        if (m_logger) {
            m_logger->info("Loaded sound buffer: {} from {}", id, path);
        }
        
        return buffer;
    }

    std::shared_ptr<SoundBufferHandle> SFMLResourceManager::getSoundBuffer(const std::string& id) const {
        auto it = m_soundBuffers.find(id);
        if (it != m_soundBuffers.end()) {
            return it->second;
        }

        if (m_logger) {
            m_logger->error("Sound buffer not found: {}", id);
        }
        return nullptr;
    }

    void SFMLResourceManager::registerMusicPath(const std::string& id, const std::string& path) {
        m_musicPaths[id] = path;
         if (m_logger) {
            m_logger->info("Registered music path: {} -> {}", id, path);
        }
    }

    std::string SFMLResourceManager::getMusicPath(const std::string& id) const {
        auto it = m_musicPaths.find(id);
        if (it != m_musicPaths.end()) {
            return it->second;
        }
        
        if (m_logger) {
            m_logger->error("Music path not found for id: {}", id);
        }
        return "";
    }

} // namespace engine
