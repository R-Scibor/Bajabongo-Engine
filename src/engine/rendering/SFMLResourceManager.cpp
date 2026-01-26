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
        // 1. Check cache first
        auto it = m_textures.find(id);
        if (it != m_textures.end()) {
            return it->second;
        }

        // 2. Load from disk
        auto texture = std::make_shared<sf::Texture>();
        if (!texture->loadFromFile(path)) {
            // 3. Fallback strategy
            // Try user-specified fallback path (e.g. placeholder asset)
            if (texture->loadFromFile("../../assets/textures/eg.png")) {
                if (m_logger) {
                    m_logger->warn("Failed to load texture: {}. Used fallback: ../../assets/textures/eg.png", path);
                }
            } else {
                if (m_logger) {
                    m_logger->error("Failed to load texture: {} and fallback. Using generated 128x128 magenta.", path);
                }
                
                // 4. Ultimate fallback: Generate 128x128 Magenta texture
                // This makes missing assets obvious visually without crashing.
                sf::Image fallbackImage;
                // Note: SFML 3 API change: create -> resize
                fallbackImage.resize({128, 128}, sf::Color::Magenta);
                (void)texture->loadFromImage(fallbackImage);
            }
        }

        // Force nearest-neighbor interpolation for crisp pixel art look
        // (This assumes the engine is 2D pixel-art based. For HD games, this might be a parameter).
        texture->setSmooth(false);

        // 5. Cache and return
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
            // Logging error on every frame might spam the console if a texture is missing,
            // but it's useful for debugging.
            m_logger->error("Texture not found: {}", id);
        }
        return nullptr;
    }

    std::shared_ptr<SoundBufferHandle> SFMLResourceManager::loadSoundBuffer(const std::string& id, const std::string& path) {
        auto it = m_soundBuffers.find(id);
        if (it != m_soundBuffers.end()) {
            return it->second;
        }

        auto buffer = std::make_shared<sf::SoundBuffer>();
        if (!buffer->loadFromFile(path)) {
            if (m_logger) {
                m_logger->error("Failed to load sound buffer: {}", path);
            }
            return nullptr;
        }

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