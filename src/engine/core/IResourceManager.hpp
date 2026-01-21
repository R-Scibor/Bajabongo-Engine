#pragma once

#include <string>
#include <memory>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Audio/SoundBuffer.hpp>

namespace engine {

    // For Phase 5A, we alias TextureHandle directly to sf::Texture.
    // In the future, this could be a wrapper class or a handle ID.
    using TextureHandle = sf::Texture;
    using SoundBufferHandle = sf::SoundBuffer;

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

        /**
         * @brief Loads a sound buffer from disk or returns a cached one.
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

        /**
         * @brief Registers a path for a music track (streaming).
         * @param id Unique identifier for the music track.
         * @param path File path to the audio file.
         */
        virtual void registerMusicPath(const std::string& id, const std::string& path) = 0;

        /**
         * @brief Retrieves the path for a registered music track.
         * @param id Unique identifier for the music track.
         * @return The file path, or empty string if not found.
         */
        virtual std::string getMusicPath(const std::string& id) const = 0;
    };

} // namespace engine
