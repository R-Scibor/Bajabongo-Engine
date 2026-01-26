#pragma once

#include <string>
#include <memory>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Audio/SoundBuffer.hpp>

namespace engine {

    // For Phase 5A, we alias TextureHandle directly to sf::Texture.
    // In the future, this could be a wrapper class or a handle ID (Flyweight pattern).
    using TextureHandle = sf::Texture;
    using SoundBufferHandle = sf::SoundBuffer;

    /**
     * @brief Abstract interface for the Asset Management system.
     * * Handles loading, caching, and retrieving game assets (Textures, Sounds).
     * * Ensures assets are loaded only once to save memory.
     */
    class IResourceManager {
    public:
        virtual ~IResourceManager() = default;

        // --- Textures ---

        /**
         * @brief Loads a texture from disk into the cache.
         * * If the texture is already loaded, it returns the existing cached instance.
         * @param id Unique identifier for the texture (e.g., "player_sheet").
         * @param path File path to the texture image (e.g., "assets/textures/player.png").
         * @return Shared pointer to the texture, or nullptr if loading failed.
         */
        virtual std::shared_ptr<TextureHandle> loadTexture(const std::string& id, const std::string& path) = 0;

        /**
         * @brief Retrieves a previously loaded texture from the cache.
         * @param id Unique identifier for the texture.
         * @return Shared pointer to the texture, or nullptr if the ID is not found.
         */
        virtual std::shared_ptr<TextureHandle> getTexture(const std::string& id) const = 0;

        // --- Sound Effects (Short Audio) ---

        /**
         * @brief Loads a sound buffer from disk or returns a cached one.
         * * SoundBuffers are fully loaded into memory. Best for short SFX (gunshots, UI).
         * @param id Unique identifier for the sound.
         * @param path File path to the audio file.
         * @return Shared pointer to the sound buffer, or nullptr if loading failed.
         */
        virtual std::shared_ptr<SoundBufferHandle> loadSoundBuffer(const std::string& id, const std::string& path) = 0;

        /**
         * @brief Retrieves a previously loaded sound buffer.
         * @param id Unique identifier for the sound.
         * @return Shared pointer to the sound buffer, or nullptr if not found.
         */
        virtual std::shared_ptr<SoundBufferHandle> getSoundBuffer(const std::string& id) const = 0;

        // --- Music (Streaming Audio) ---

        /**
         * @brief Registers a file path for a music track.
         * * Music is typically streamed from disk rather than loaded entirely into RAM.
         * * This method only stores the path for later use by the AudioSystem.
         * @param id Unique identifier for the music track.
         * @param path File path to the audio file.
         */
        virtual void registerMusicPath(const std::string& id, const std::string& path) = 0;

        /**
         * @brief Retrieves the registered file path for a music track.
         * @param id Unique identifier for the music track.
         * @return The file path string, or empty string if not found.
         */
        virtual std::string getMusicPath(const std::string& id) const = 0;
    };

} // namespace engine