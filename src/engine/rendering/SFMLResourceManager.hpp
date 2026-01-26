#pragma once

#include "engine/core/IResourceManager.hpp"
#include "engine/core/ILogger.hpp"
#include <unordered_map>
#include <memory>
#include <string>

namespace engine {
    class ILoggerManager;

    /**
     * @brief Concrete implementation of the Resource Manager using SFML.
     * * Handles loading and caching of Textures and SoundBuffers from the disk.
     * * Implements fallback mechanisms (e.g., generating a magenta texture if a file is missing)
     * to prevent the game from crashing due to missing assets.
     */
    class SFMLResourceManager : public IResourceManager {
    public:
        /**
         * @brief Constructs the manager and acquires a logger instance.
         * @param loggerManager Reference to the central logger factory.
         */
        explicit SFMLResourceManager(std::shared_ptr<ILoggerManager> loggerManager);
        ~SFMLResourceManager() override = default;

        /**
         * @brief Loads a texture from disk into VRAM.
         * * If loading fails, it attempts a fallback path, and finally generates a 128x128 Magenta square.
         * * Automatically sets texture filtering to Nearest Neighbor (pixel art style).
         * @param id The unique key to store this texture under.
         * @param path The relative file path to the image.
         */
        std::shared_ptr<TextureHandle> loadTexture(const std::string& id, const std::string& path) override;
        
        /**
         * @brief Retrieves a cached texture.
         * @return Pointer to the texture, or nullptr if id doesn't exist.
         */
        std::shared_ptr<TextureHandle> getTexture(const std::string& id) const override;

        /**
         * @brief Loads a sound effect into memory.
         * @param id Unique key.
         * @param path File path.
         */
        std::shared_ptr<SoundBufferHandle> loadSoundBuffer(const std::string& id, const std::string& path) override;
        
        std::shared_ptr<SoundBufferHandle> getSoundBuffer(const std::string& id) const override;

        /**
         * @brief Registers a path for streaming music (does not load into RAM).
         */
        void registerMusicPath(const std::string& id, const std::string& path) override;
        
        std::string getMusicPath(const std::string& id) const override;

    private:
        std::shared_ptr<ILogger> m_logger;
        
        /// @brief Cache for textures to avoid reloading the same file twice.
        std::unordered_map<std::string, std::shared_ptr<TextureHandle>> m_textures;
        
        /// @brief Cache for short sound effects.
        std::unordered_map<std::string, std::shared_ptr<SoundBufferHandle>> m_soundBuffers;
        
        /// @brief Registry of paths for music streams.
        std::unordered_map<std::string, std::string> m_musicPaths;
    };

} // namespace engine