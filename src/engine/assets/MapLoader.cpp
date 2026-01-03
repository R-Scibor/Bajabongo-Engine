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
#include "engine/components/PendingPhysicsBodyComponent.hpp"
#include "engine/components/HalfCollisionComponent.hpp"
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
                        // TODO: Implement Entity spawning here using ArchetypeManager
                        if (m_logger) m_logger->warn("Entity spawning from map not yet implemented.");
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

        if (visible) {
            auto entity = m_context.m_registry->create();
            // Scale map background by 1.5x (as per previous logic, keeping it consistent)
            float scale = 1.5f;
            
            // Image layers usually start at 0,0
            m_context.m_registry->emplace<TransformComponent>(entity, Vector2f{0.0f, 0.0f}, 0.0f, Vector2f{scale, scale});
            
            // Use the image name as the texture key (assuming it's loaded)
            // Note: In a real engine, we'd probably want to strip paths or have a resource mapping.
            // For now, using the filename directly as the texture ID.
            m_context.m_registry->emplace<RenderableComponent>(entity, "map_sprite", 0, sf::Color(255, 255, 255, static_cast<std::uint8_t>(opacity * 255)));
            
            if (m_logger) m_logger->info("Created map background entity for image: {}", imageName);
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
        float scale = 1.5f; // Hardcoded scale from original implementation
        
        float x = obj.value("x", 0.0f) * scale;
        float y = obj.value("y", 0.0f) * scale;
        float w = obj.value("width", 0.0f) * scale;
        float h = obj.value("height", 0.0f) * scale;
        float rot = obj.value("rotation", 0.0f);

        // Box2D uses center position, Tiled uses top-left
        float centerX = x + w * 0.5f;
        float centerY = y + h * 0.5f;

        auto entity = m_context.m_registry->create();

        // If it's half collision, add the tag component
        if (isHalfCollision) {
            m_context.m_registry->emplace<HalfCollisionComponent>(entity);
        }
        
        // Create Physics Body
        // isStatic = true
        // isSensor = false
        // isBullet = false
        // fixedRotation = false (though for static it doesn't matter much)
        m_context.m_registry->emplace<PendingPhysicsBodyComponent>(
            entity,
            Vector2f{centerX, centerY},
            Vector2f{w, h},
            true, // isStatic
            1.0f, // density
            false, // isSensor
            false, // isBullet
            0.0f, // angularDamping
            false, // fixedRotation
            Vector2f{0.f, 0.f}, // linearVelocity
            rot * (3.14159f / 180.0f) // angle in radians
        );
        
        // Visual debug transform (optional, but good for consistency)
        m_context.m_registry->emplace<TransformComponent>(entity, Vector2f{centerX, centerY}, rot, Vector2f{1.0f, 1.0f});
    }

} // namespace engine
