#include "engine/pch.h"
#include "AssetManifestLoader.hpp"
#include "engine/core/IResourceManager.hpp"
#include "engine/rendering/Sprite.hpp"
#include "engine/rendering/AnimationClip.hpp"
#include "engine/core/ILogger.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>

namespace engine {

    AssetManifestLoader::AssetManifestLoader(std::shared_ptr<IResourceManager> resourceManager,
                                             std::shared_ptr<SpriteManager> spriteManager,
                                             std::shared_ptr<AnimationLibrary> animationLibrary,
                                             std::shared_ptr<ILogger> logger)
        : m_resourceManager(std::move(resourceManager))
        , m_spriteManager(std::move(spriteManager))
        , m_animationLibrary(std::move(animationLibrary))
        , m_logger(std::move(logger))
    {
    }

    bool AssetManifestLoader::load(const std::string& manifestPath) {
        std::ifstream file(manifestPath);
        if (!file.is_open()) {
            if (m_logger) {
                m_logger->error("AssetManifestLoader: Failed to open manifest file: {}", manifestPath);
            }
            return false;
        }

        nlohmann::json j;
        try {
            file >> j;
        } catch (const nlohmann::json::parse_error& e) {
            if (m_logger) {
                m_logger->error("AssetManifestLoader: JSON parse error in {}: {}", manifestPath, e.what());
            }
            return false;
        }

        if (m_logger) {
            m_logger->info("AssetManifestLoader: Loading manifest: {}", manifestPath);
        }

        // Checkpoint 2.3: Implement Texture Loop
        if (j.contains("textures")) {
            loadTextures(j["textures"]);
        }

        return true;
    }

    void AssetManifestLoader::loadTextures(const nlohmann::json& textureArray) {
        if (!textureArray.is_array()) {
            if (m_logger) {
                m_logger->warn("AssetManifestLoader: 'textures' field is not an array.");
            }
            return;
        }

        for (const auto& textureJson : textureArray) {
            if (!textureJson.contains("id") || !textureJson.contains("path")) {
                if (m_logger) {
                    m_logger->warn("AssetManifestLoader: Skipping invalid texture entry (missing id or path).");
                }
                continue;
            }

            std::string id = textureJson["id"];
            std::string path = textureJson["path"];

            if (m_resourceManager) {
                m_resourceManager->loadTexture(id, path);
            }
        }
    }

} // namespace engine