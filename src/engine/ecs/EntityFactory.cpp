#include "engine/pch.h"
#include "EntityFactory.hpp"
#include "engine/components/TransformComponent.hpp"
#include "engine/components/RenderableComponent.hpp"
#include "engine/components/PhysicsBodyComponent.hpp"
#include "engine/components/PendingPhysicsBodyComponent.hpp"
#include "engine/components/AnimationComponent.hpp"
#include "engine/core/ILoggerManager.hpp"
#include "engine/core/ILogger.hpp"
#include <SFML/Graphics/Color.hpp>

namespace engine {

    namespace ComponentNames {
        static constexpr const char* Transform = "Transform";
        static constexpr const char* Renderable = "Renderable";
        static constexpr const char* Physics = "Physics";
        static constexpr const char* Animation = "Animation";
    }

    EntityFactory::EntityFactory(EngineContext& context, std::shared_ptr<ArchetypeManager> archetypeManager)
        : m_context(context), m_archetypeManager(archetypeManager) {
        if (m_context.m_logManager) {
            m_logger = m_context.m_logManager->GetLogger("EntityFactory");
        }
        registerDefaultLoaders();
    }

    void EntityFactory::registerComponentLoader(const std::string& componentName, ComponentLoaderFn loader) {
        m_componentRegistry[componentName] = loader;
    }

    entt::entity EntityFactory::spawn(const std::string& archetypeId, const Vector2f& position) {
        const auto* archetypeData = m_archetypeManager->getArchetype(archetypeId);
        if (!archetypeData) {
            if (m_logger) m_logger->error("EntityFactory: Archetype '{}' not found", archetypeId);
            return entt::null;
        }

        auto entity = m_context.m_registry->create();

        // 1. Initialize TransformComponent with spawn position (crucial for other components)
        m_context.m_registry->emplace<TransformComponent>(entity, position, 0.f, Vector2f{1.f, 1.f});

        // 2. Iterate over keys in JSON and call registered loaders
        for (auto& [key, value] : archetypeData->items()) {
            auto it = m_componentRegistry.find(key);
            if (it != m_componentRegistry.end()) {
                try {
                    it->second(*m_context.m_registry, entity, value);
                } catch (const std::exception& e) {
                    if (m_logger) m_logger->error("EntityFactory: Error loading component '{}' for archetype '{}': {}", key, archetypeId, e.what());
                }
            } else {
                if (m_logger) m_logger->warn("EntityFactory: No loader registered for component '{}'", key);
            }
        }

        return entity;
    }

    void EntityFactory::registerDefaultLoaders() {
        // --- Transform ---
        registerComponentLoader(ComponentNames::Transform, [](entt::registry& registry, entt::entity entity, const nlohmann::json& data) {
            auto& transform = registry.get<TransformComponent>(entity);
            
            if (data.contains("rotation")) {
                transform.rotation = data["rotation"];
            }
            if (data.contains("scale") && data["scale"].is_array()) {
                transform.scale.x = data["scale"][0];
                transform.scale.y = data["scale"][1];
            }
        });

        // --- Renderable ---
        registerComponentLoader(ComponentNames::Renderable, [](entt::registry& registry, entt::entity entity, const nlohmann::json& data) {
            std::string spriteId = data.value("spriteId", "");
            int layer = data.value("layer", 0);
            sf::Color color = sf::Color::White;

            if (data.contains("color") && data["color"].is_array()) {
                auto& c = data["color"];
                if (c.size() >= 3) {
                    color.r = c[0].get<int>();
                    color.g = c[1].get<int>();
                    color.b = c[2].get<int>();
                    color.a = c.size() > 3 ? c[3].get<int>() : 255;
                }
            }

            if (!spriteId.empty()) {
                registry.emplace<RenderableComponent>(entity, spriteId, layer, color);
            }
        });

        // --- Physics ---
        registerComponentLoader(ComponentNames::Physics, [this](entt::registry& registry, entt::entity entity, const nlohmann::json& data) {
            Vector2f position = {0.f, 0.f};
            if (registry.all_of<TransformComponent>(entity)) {
                position = registry.get<TransformComponent>(entity).position;
            }

            Vector2f size = {0.f, 0.f};
            if (data.contains("size") && data["size"].is_array()) {
                size.x = data["size"][0];
                size.y = data["size"][1];
            }

            // Validation for size
            if (size.x <= 0.001f || size.y <= 0.001f) {
                if (m_logger) m_logger->warn("EntityFactory: Physics component has invalid size for entity {}", static_cast<uint32_t>(entity));
                size = {32.f, 32.f}; // Fallback
            }

            bool isStatic = (data.value("type", "static") == "static");
            float density = data.value("density", 1.0f);
            bool isSensor = data.value("isSensor", false);
            bool fixedRotation = data.value("fixedRotation", false);

            registry.emplace<PendingPhysicsBodyComponent>(entity, PendingPhysicsBodyComponent{
                .position = position,
                .size = size,
                .isStatic = isStatic,
                .density = density,
                .isSensor = isSensor,
                .fixedRotation = fixedRotation
            });
        });

        // --- Animation ---
        registerComponentLoader(ComponentNames::Animation, [](entt::registry& registry, entt::entity entity, const nlohmann::json& data) {
            AnimationComponent animComp;
            animComp.currentClipId = data.value("defaultClip", "");
            animComp.isPlaying = data.value("playing", false);
            
            registry.emplace<AnimationComponent>(entity, animComp);
        });
    }

} // namespace engine