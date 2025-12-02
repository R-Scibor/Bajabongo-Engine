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

            // Checkpoint 3.2: Connect to Main Loop
            if (textureJson.contains("sprites") && textureJson["sprites"].is_array()) {
                for (const auto& spriteJson : textureJson["sprites"]) {
                    loadSprite(id, spriteJson);
                }
            }

            // Checkpoint 4.1: Connect Animation Loop
            if (textureJson.contains("animations") && textureJson["animations"].is_array()) {
                for (const auto& animJson : textureJson["animations"]) {
                    loadAnimation(id, animJson);
                }
            }
        }
    }

    // Checkpoint 3.1: Implement parseSprite Helper
    void AssetManifestLoader::loadSprite(const std::string& textureId, const nlohmann::json& spriteJson) {
        if (!m_spriteManager) return;

        if (!spriteJson.contains("id") || !spriteJson.contains("region") || !spriteJson.contains("origin")) {
            if (m_logger) {
                m_logger->warn("AssetManifestLoader: Invalid sprite definition in texture '{}'", textureId);
            }
            return;
        }

        std::string spriteId = spriteJson["id"];
        auto region = spriteJson["region"]; // array of 4 ints
        auto origin = spriteJson["origin"]; // array of 2 floats

        if (!region.is_array() || region.size() != 4 || !origin.is_array() || origin.size() != 2) {
             if (m_logger) {
                m_logger->warn("AssetManifestLoader: Invalid region/origin format for sprite '{}'", spriteId);
            }
            return;
        }

        engine::SpriteDesc desc;
        desc.textureId = textureId;
        // SFML 3 requires Vector2i for position and size
        desc.uvRect = sf::IntRect(
            sf::Vector2i(region[0].get<int>(), region[1].get<int>()),
            sf::Vector2i(region[2].get<int>(), region[3].get<int>())
        );
        desc.origin = sf::Vector2f(origin[0].get<float>(), origin[1].get<float>());

        m_spriteManager->registerSprite(spriteId, desc);
    }

    void AssetManifestLoader::loadAnimation(const std::string& textureId, const nlohmann::json& animJson) {
        if (!m_spriteManager || !m_animationLibrary) return;

        if (!animJson.contains("id") || !animJson.contains("frameStart") ||
            !animJson.contains("frameSize") || !animJson.contains("frameCount") ||
            !animJson.contains("duration")) {
             if (m_logger) {
                m_logger->warn("AssetManifestLoader: Invalid animation definition in texture '{}'", textureId);
            }
            return;
        }

        std::string animId = animJson["id"];
        auto frameStart = animJson["frameStart"]; // [x, y]
        auto frameSize = animJson["frameSize"];   // [w, h]
        int frameCount = animJson["frameCount"];
        float duration = animJson["duration"];
        bool loop = animJson.value("loop", false);
        int columns = animJson.value("columns", frameCount); // Default to horizontal strip if 0 or missing

        if (columns <= 0) columns = frameCount; // Safety fallback

        std::vector<std::string> spriteIds;
        spriteIds.reserve(frameCount);

        // Check for optional origin override
        sf::Vector2f customOrigin;
        bool hasCustomOrigin = false;
        if (animJson.contains("origin") && animJson["origin"].is_array() && animJson["origin"].size() == 2) {
            customOrigin.x = animJson["origin"][0].get<float>();
            customOrigin.y = animJson["origin"][1].get<float>();
            hasCustomOrigin = true;
        }

        // Checkpoint 4.2: The Slicing Loop
        for (int i = 0; i < frameCount; ++i) {
            int col = i % columns;
            int row = i / columns;

            int x = frameStart[0].get<int>() + (col * frameSize[0].get<int>());
            int y = frameStart[1].get<int>() + (row * frameSize[1].get<int>());
            int w = frameSize[0].get<int>();
            int h = frameSize[1].get<int>();

            std::string frameId = animId + "_" + std::to_string(i);

            engine::SpriteDesc desc;
            desc.textureId = textureId;
            desc.uvRect = sf::IntRect(sf::Vector2i(x, y), sf::Vector2i(w, h));
            
            if (hasCustomOrigin) {
                desc.origin = customOrigin;
            } else {
                desc.origin = sf::Vector2f(w * 0.5f, h * 0.5f); // Default pivot: Center
            }

            m_spriteManager->registerSprite(frameId, desc);
            spriteIds.push_back(frameId);
        }

        // Checkpoint 4.3: Clip Registration
        engine::AnimationClip clip;
        clip.name = animId;
        clip.loop = loop;
        clip.frameDuration = duration;
        clip.spriteIds = spriteIds;

        m_animationLibrary->registerClip(animId, clip);
    }

} // namespace engine