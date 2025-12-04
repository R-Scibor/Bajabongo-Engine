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
#include <entt/entt.hpp>

namespace engine {

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
                
                if (type == "imagelayer") {
                    std::string imageName = layer.value("image", "");
                    
                    float opacity = layer.value("opacity", 1.0f);
                    bool visible = layer.value("visible", true);

                    if (visible) {
                        auto entity = m_context.m_registry->create();
                        // Scale map background by 4x
                        float scale = 4.0f;
                        m_context.m_registry->emplace<TransformComponent>(entity, Vector2f{0.0f, 0.0f}, 0.0f, Vector2f{scale, scale});
                        
                        m_context.m_registry->emplace<RenderableComponent>(entity, "map_sprite", 0, sf::Color(255, 255, 255, static_cast<std::uint8_t>(opacity * 255)));
                        
                        if (m_logger) m_logger->info("Created map background entity.");
                    }

                } else if (type == "objectgroup" && layer.value("name", "") == "Collision") {
                    if (layer.contains("objects")) {
                        int count = 0;
                        float scale = 4.0f;
                        for (const auto& obj : layer["objects"]) {
                            float x = obj.value("x", 0.0f) * scale;
                            float y = obj.value("y", 0.0f) * scale;
                            float w = obj.value("width", 0.0f) * scale;
                            float h = obj.value("height", 0.0f) * scale;
                            float rot = obj.value("rotation", 0.0f);

                            float centerX = x + w * 0.5f;
                            float centerY = y + h * 0.5f;

                            auto entity = m_context.m_registry->create();
                            
                            m_context.m_registry->emplace<PendingPhysicsBodyComponent>(
                                entity,
                                Vector2f{centerX, centerY},
                                Vector2f{w, h},
                                true,
                                1.0f, 
                                false, 
                                false, 
                                0.0f, 
                                false, 
                                Vector2f{0.f, 0.f}, 
                                rot * (3.14159f / 180.0f) 
                            );
                            
                            m_context.m_registry->emplace<TransformComponent>(entity, Vector2f{centerX, centerY}, rot, Vector2f{1.0f, 1.0f});

                            count++;
                        }
                        if (m_logger) m_logger->info("Created {} collision bodies.", count);
                    }
                }
            }
        }

        return true;
    }

} // namespace engine