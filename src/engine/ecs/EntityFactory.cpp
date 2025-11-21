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

    EntityFactory::EntityFactory(EngineContext& context, std::shared_ptr<ArchetypeManager> archetypeManager)
        : m_context(context), m_archetypeManager(archetypeManager) {
        if (m_context.m_logManager) {
            m_logger = m_context.m_logManager->GetLogger("EntityFactory");
        }
    }

    entt::entity EntityFactory::spawn(const std::string& archetypeId, const Vector2f& position) {
        const auto* archetypeData = m_archetypeManager->getArchetype(archetypeId);
        if (!archetypeData) {
            if (m_logger) m_logger->error("EntityFactory: Archetype '{}' not found", archetypeId);
            return entt::null;
        }

        auto entity = m_context.m_registry->create();

        if (archetypeData->contains("Transform")) {
            addTransform(entity, (*archetypeData)["Transform"], position);
        } else {
            // Always add a transform even if not specified, using spawn position
             m_context.m_registry->emplace<TransformComponent>(entity, position, 0.f, Vector2f{1.f, 1.f});
        }

        if (archetypeData->contains("Renderable")) {
            addRenderable(entity, (*archetypeData)["Renderable"]);
        }

        if (archetypeData->contains("Physics")) {
            addPhysics(entity, (*archetypeData)["Physics"], position);
        }

        if (archetypeData->contains("Animation")) {
            addAnimation(entity, (*archetypeData)["Animation"]);
        }

        return entity;
    }

    void EntityFactory::addTransform(entt::entity entity, const nlohmann::json& data, const Vector2f& spawnPos) {
        Vector2f pos = spawnPos;
        float rot = 0.f;
        Vector2f scale = {1.f, 1.f};

        // Override with JSON data if present (relative to spawn, or absolute? 
        // Usually archetype defines defaults. Let's assume JSON defines offsets or defaults, 
        // but the spawn position passed to function overrides or adds to it.
        // For simplicity in this task: if position is passed to Spawn, use it. 
        // If JSON has position, maybe it's an offset?
        // Task says: "spawn(archetypeId, position)". Let's assume "position" arg is the world position.
        
        // However, we might want to respect rotation/scale from JSON.
        if (data.contains("rotation")) rot = data["rotation"];
        if (data.contains("scale") && data["scale"].is_array()) {
            scale.x = data["scale"][0];
            scale.y = data["scale"][1];
        }
        
        m_context.m_registry->emplace<TransformComponent>(entity, pos, rot, scale);
    }

    void EntityFactory::addRenderable(entt::entity entity, const nlohmann::json& data) {
        std::string spriteId = data.value("spriteId", "");
        int layer = data.value("layer", 0);
        
        // Color parsing could be added here, defaulting to White
        sf::Color color = sf::Color::White;

        if (!spriteId.empty()) {
            m_context.m_registry->emplace<RenderableComponent>(entity, spriteId, layer, color);
        }
    }

    void EntityFactory::addPhysics(entt::entity entity, const nlohmann::json& data, const Vector2f& position) {
        std::string typeStr = data.value("type", "static");
        bool isStatic = (typeStr == "static");
        
        Vector2f size = {0.f, 0.f};
        if (data.contains("size") && data["size"].is_array()) {
            size.x = data["size"][0];
            size.y = data["size"][1];
        }

        float density = data.value("density", 1.0f);
        bool isSensor = data.value("isSensor", false);
        bool fixedRotation = data.value("fixedRotation", false);

        m_context.m_registry->emplace<PendingPhysicsBodyComponent>(
            entity,
            position,
            size,
            isStatic,
            density,
            isSensor,
            fixedRotation
        );
    }

    void EntityFactory::addAnimation(entt::entity entity, const nlohmann::json& data) {
        AnimationComponent animComp;
        animComp.currentClipId = data.value("defaultClip", "");
        animComp.isPlaying = data.value("playing", false);
        // Loop, speed, etc could be added here if Component supports it
        
        m_context.m_registry->emplace<AnimationComponent>(entity, animComp);
    }

} // namespace engine