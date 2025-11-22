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
        void loadTextures(const nlohmann::json& j);
        void loadSprite(const std::string& textureId, const nlohmann::json& spriteJson);
        void loadAnimation(const std::string& textureId, const nlohmann::json& animJson);
    };

} // namespace engine