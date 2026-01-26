/**
 * @file MapLoader.cpp
 * @brief Implementation of the MapLoader class.
 */

#include "engine/pch.h"
#include "MapLoader.hpp"
#include <fstream>
#include <cstdint>
#include <nlohmann/json.hpp>
#include "engine/core/EngineContext.hpp"
#include "engine/core/ILoggerManager.hpp"
#include "engine/core/ILogger.hpp"
#include "engine/components/TransformComponent.hpp"
#include "engine/components/RenderableComponent.hpp"
#include "engine/components/WorldBoundsComponent.hpp"
#include "engine/components/PendingPhysicsBodyComponent.hpp"
#include "engine/components/HalfCollisionComponent.hpp"
#include "engine/physics/PhysicsConstants.hpp"
#include "engine/ecs/EntityFactory.hpp"
#include "game/components/VisibleToPlayerComponent.hpp"
#include <entt/entt.hpp>

namespace engine {

    // Constants for layer names and types
    namespace {
        const std::string LAYER_TYPE_IMAGE = "imagelayer";
        const std::string LAYER_TYPE_OBJECT = "objectgroup";
        const std::string LAYER_NAME_COLLISION = "Collision";
        const std::string LAYER_NAME_HALF_COLLISION = "HalfCollision";
        const std::string LAYER_NAME_ENTITIES = "Entities";
    }

    MapLoader::MapLoader(EngineContext& context)
        : m_context(context) {
        if (context.m_logManager) {
            m_logger = context.m_logManager->GetLogger("MapLoader");
        }
    }

    bool MapLoader::load(const std::string& filepath) {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            if (m_logger) m_logger->error("Failed to open map file: {}", filepath);
            return false;
        }

        nlohmann::json mapJson;
        try {
            file >> mapJson;
        } catch (const nlohmann::json::parse_error& e) {
            if (m_logger) m_logger->error("Failed to parse map JSON: {}", e.what());
            return false;
        }

        if (m_logger) m_logger->info("Loading map from {}", filepath);

        // Read mapScale from JSON, defaulting to global default if not present
        float mapScale = mapJson.value("mapScale", EngineContext::MAP_SCALE);
        m_context.mapScale = mapScale;
        if (m_logger) m_logger->info("Map Scale set to: {}", mapScale);

        // Iterate through layers and delegate processing based on type/name
        if (mapJson.contains("layers")) {
            for (const auto& layer : mapJson["layers"]) {
                std::string type = layer.value("type", "");
                std::string name = layer.value("name", "");

                if (type == LAYER_TYPE_IMAGE) {
                    processImageLayer(layer);
                } else if (type == LAYER_TYPE_OBJECT) {
                    if (name == LAYER_NAME_COLLISION) {
                        processObjectLayer(layer, false);
                    } else if (name == LAYER_NAME_HALF_COLLISION) {
                        processObjectLayer(layer, true);
                    } else if (name == LAYER_NAME_ENTITIES) {
                        processEntityLayer(layer);
                    }
                }
            }
        }

        return true;
    }

    void MapLoader::processImageLayer(const nlohmann::json& layer) {
        std::string imageName = layer.value("image", "");
        float opacity = layer.value("opacity", 1.0f);
        bool visible = layer.value("visible", true);

        // Extract bounds from image layer if available
        float imageWidth = layer.value("imagewidth", 0.0f);
        float imageHeight = layer.value("imageheight", 0.0f);

        if (visible) {
            auto entity = m_context.m_registry->create();
            // Use scale from context
            float scale = m_context.mapScale;
            
            // Image layers usually start at 0,0
            m_context.m_registry->emplace<TransformComponent>(entity, Vector2f{0.0f, 0.0f}, 0.0f, Vector2f{scale, scale});
            
            // Image name as texture key
            std::string textureKey = "map_sprite"; // Default fallback
            if (!imageName.empty()) {
                // Remove path if present (e.g. "../assets/textures/map.png" -> "map.png")
                size_t lastSlash = imageName.find_last_of("/\\");
                std::string filename = (lastSlash != std::string::npos) ? imageName.substr(lastSlash + 1) : imageName;

                // Remove extension (e.g. "map.png" -> "map")
                size_t lastDot = filename.find_last_of(".");
                if (lastDot != std::string::npos) {
                    filename = filename.substr(0, lastDot);
                }
                
                // Append _sprite
                textureKey = filename + "_sprite";
            }

            m_context.m_registry->emplace<RenderableComponent>(entity, textureKey, 0, sf::Color(255, 255, 255, static_cast<std::uint8_t>(opacity * 255)));
            
            // Map background should always be visible
            game::VisibleToPlayerComponent visComp;
            visComp.alwaysVisible = true;
            m_context.m_registry->emplace<game::VisibleToPlayerComponent>(entity, visComp);
            
            // Add WorldBoundsComponent if dimensions are valid
            if (imageWidth > 0 && imageHeight > 0) {
                 m_context.m_registry->emplace<WorldBoundsComponent>(entity, imageWidth * scale, imageHeight * scale);
                 if (m_logger) m_logger->info("Added WorldBoundsComponent: {}x{}", imageWidth * scale, imageHeight * scale);
            }

            if (m_logger) m_logger->info("Created map background entity for image: {}", imageName);
        }
    }

    void MapLoader::processEntityLayer(const nlohmann::json& layer) {
        if (!m_context.m_entityFactory) {
            if (m_logger) m_logger->error("MapLoader: EntityFactory is null, cannot spawn entities.");
            return;
        }

        if (layer.contains("objects")) {
            int count = 0;
            float scale = m_context.mapScale;

            for (const auto& obj : layer["objects"]) {
                // Tiled renamed "Type" to "Class" in recent versions.
                // We check "type" first (legacy), then "class" (new), then fallback to "name".
                std::string type = obj.value("type", "");
                if (type.empty()) {
                    type = obj.value("class", "");
                }
                if (type.empty()) {
                    // Fallback: check if "name" is the type
                    type = obj.value("name", "");
                }

                if (type.empty()) {
                    if (m_logger) m_logger->warn("MapLoader: Object in Entities layer has no 'type' or 'class' defined. Skipping.");
                    continue;
                }

                float x = obj.value("x", 0.0f) * scale;
                float y = obj.value("y", 0.0f) * scale;
                
                // Tiled objects can be points or rects.
                // If it's a point, we just use x,y.
                // If it's a rect (e.g. for a portal trigger area), we might need center.
                // Assume point objects use center logic
                // Tiled (top-left) -> Center conversion if width/height exist.
                float w = obj.value("width", 0.0f) * scale;
                float h = obj.value("height", 0.0f) * scale;

                Vector2f spawnPos = { x + w * 0.5f, y + h * 0.5f };

                entt::entity spawnedEntity = m_context.m_entityFactory->spawn(type, spawnPos);
                if (spawnedEntity != entt::null) {
                    count++;
                } else {
                    if (m_logger) m_logger->warn("MapLoader: Failed to spawn entity of type '{}'", type);
                }
            }
            if (m_logger) m_logger->info("Spawned {} entities from layer '{}'.", count, layer.value("name", "Entities"));
        }
    }

    void MapLoader::processObjectLayer(const nlohmann::json& layer, bool isHalfCollision) {
        if (layer.contains("objects")) {
            int count = 0;
            for (const auto& obj : layer["objects"]) {
                createCollisionBody(obj, isHalfCollision);
                count++;
            }
            if (m_logger) m_logger->info("Created {} objects for layer '{}'.", count, layer.value("name", "Unknown"));
        }
    }

    void MapLoader::createCollisionBody(const nlohmann::json& obj, bool isHalfCollision) {
        float scale = m_context.mapScale;
        
        float x = obj.value("x", 0.0f) * scale;
        float y = obj.value("y", 0.0f) * scale;
        float w = obj.value("width", 0.0f) * scale;
        float h = obj.value("height", 0.0f) * scale;
        float rot = obj.value("rotation", 0.0f);

        // Box2D uses center position, Tiled uses top-left
        float centerX = x + w * 0.5f;
        float centerY = y + h * 0.5f;

        auto entity = m_context.m_registry->create();

        unsigned int categoryBits = PhysicsCategory::Wall | PhysicsCategory::VisibilityBlocker;
        unsigned int maskBits = PhysicsCategory::All;

        // If it's half collision, add the tag component
        if (isHalfCollision) {
            m_context.m_registry->emplace<HalfCollisionComponent>(entity);
            categoryBits = PhysicsCategory::LowObstacle;
            // Wall: All.
            // LowObstacle: Collides with Player, Enemy. Does NOT collide with Projectile.
            maskBits = PhysicsCategory::Player | PhysicsCategory::Enemy;
        }
        
        // Create Physics Body
        // isStatic = true
        // isSensor = false
        // isBullet = false
        // fixedRotation = false (though for static it doesn't matter much)
        PendingPhysicsBodyComponent pending{
            .position = Vector2f{centerX, centerY},
            .isStatic = true,
            .fixedRotation = false,
            .rotation = rot * (3.14159f / 180.0f)
        };

        // Add single fixture
        FixtureDef fixDef;
        fixDef.size = Vector2f{w, h};
        fixDef.density = 1.0f;
        fixDef.isSensor = false;
        fixDef.categoryBits = categoryBits;
        fixDef.maskBits = maskBits;
        
        pending.fixtures.push_back(fixDef);
        
        m_context.m_registry->emplace<PendingPhysicsBodyComponent>(entity, pending);
        
        // Visual debug transform (optional, but good for consistency)
        m_context.m_registry->emplace<TransformComponent>(entity, Vector2f{centerX, centerY}, rot, Vector2f{1.0f, 1.0f});
    }

} // namespace engine
