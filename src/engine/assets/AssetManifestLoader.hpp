/**
 * @file AssetManifestLoader.hpp
 * @brief Defines the AssetManifestLoader class responsible for loading assets from JSON manifests.
 */

#pragma once

#include <string>
#include <memory>
#include <nlohmann/json_fwd.hpp>

namespace engine {

    class IResourceManager;
    class SpriteManager;
    class AnimationLibrary;
    class ILogger;

    /**
     * @brief Loads assets (textures, sprites, animations) from a JSON manifest file.
     */
    class AssetManifestLoader {
    public:
        /**
         * @brief Constructs the loader with necessary dependencies.
         * @param resourceManager Handles loading of raw textures.
         * @param spriteManager Handles registration of sprite definitions (UVs, pivots).
         * @param animationLibrary Handles registration of animation clips.
         * @param logger Logger for error and info messages.
         */
        AssetManifestLoader(std::shared_ptr<IResourceManager> resourceManager,
                            std::shared_ptr<SpriteManager> spriteManager,
                            std::shared_ptr<AnimationLibrary> animationLibrary,
                            std::shared_ptr<ILogger> logger);

        ~AssetManifestLoader() = default;

        /**
         * @brief Loads assets from the specified JSON manifest file.
         * @param manifestPath Path to the JSON file.
         * @return true if loading succeeded, false otherwise.
         */
        bool load(const std::string& manifestPath);

    private:
        std::shared_ptr<IResourceManager> m_resourceManager;
        std::shared_ptr<SpriteManager> m_spriteManager;
        std::shared_ptr<AnimationLibrary> m_animationLibrary;
        std::shared_ptr<ILogger> m_logger;

        // Helper methods for different sections

        /**
         * @brief Parses and loads textures from the JSON manifest.
         * @param j JSON object containing the "textures" array.
         */
        void loadTextures(const nlohmann::json& j);

        /**
         * @brief Parses and registers a single sprite definition.
         * @param textureId The ID of the texture this sprite belongs to.
         * @param spriteJson JSON object defining the sprite (region, origin, etc.).
         */
        void loadSprite(const std::string& textureId, const nlohmann::json& spriteJson);

        /**
         * @brief Parses and registers an animation definition.
         * @param textureId The ID of the texture this animation uses.
         * @param animJson JSON object defining the animation (frames, duration, etc.).
         */
        void loadAnimation(const std::string& textureId, const nlohmann::json& animJson);
        
        /**
         * @brief Parses and loads sound effects.
         * @param j JSON object containing the "sounds" array.
         */
        void loadSounds(const nlohmann::json& j);

        /**
         * @brief Parses and loads music tracks.
         * @param j JSON object containing the "music" array.
         */
        void loadMusic(const nlohmann::json& j);
    };

} // namespace engine