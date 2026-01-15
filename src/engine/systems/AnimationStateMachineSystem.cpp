#include "engine/pch.h"
#include "AnimationStateMachineSystem.hpp"
#include "engine/core/EngineContext.hpp"
#include "engine/core/ILoggerManager.hpp"
#include "engine/core/ILogger.hpp"
#include "engine/components/AnimationStateComponent.hpp"
#include "engine/components/AnimationComponent.hpp"
#include "engine/components/TransformComponent.hpp"
#include "engine/components/MetaComponent.hpp"
#include <entt/entt.hpp>
#include <cmath>

namespace engine {

AnimationStateMachineSystem::AnimationStateMachineSystem(EngineContext& context)
    : m_context(context) {
    if (context.m_logManager) {
        m_logger = context.m_logManager->GetLogger("AnimStateMachine");
    }
}

void AnimationStateMachineSystem::update() {
    auto& registry = *m_context.m_registry;
    
    // Process all entities with AnimationStateComponent + AnimationComponent
    auto view = registry.view<AnimationStateComponent, AnimationComponent, TransformComponent>();
    
    for (auto entity : view) {
        auto& stateComp = view.get<AnimationStateComponent>(entity);
        auto& animComp = view.get<AnimationComponent>(entity);
        auto& transform = view.get<TransformComponent>(entity);
        
        // Only update if state changed
        if (!stateComp.stateChanged()) {
            continue;
        }
        
        // Get entity type from MetaComponent (e.g., "player", "enemy")
        std::string entityType = "player"; // Default
        if (auto* meta = registry.try_get<MetaComponent>(entity)) {
            // Check if name contains "player" or "enemy" or use specific archetypes
            // For now, let's assume meta->name (archetypeId) works as prefix, 
            // but we might need to map specific archetypes to generalized types
            // e.g., "player_with_hat" -> "player"
            if (meta->name.find("player") != std::string::npos) {
                entityType = "player";
            } else if (meta->name.find("target") != std::string::npos || meta->name.find("enemy") != std::string::npos) {
                entityType = "enemy"; // Or whatever prefix your enemy assets use
            } else {
                 entityType = meta->name;
            }
        }
        
        // Map state + direction -> clip ID
        std::string newClipId = getClipId(stateComp.state, stateComp.facing, entityType);
        
        // Update animation if clip changed
        if (animComp.currentClipId != newClipId) {
            animComp.currentClipId = newClipId;
            animComp.reset();
            
            if (m_logger) {
                m_logger->trace("Entity {} -> {}", static_cast<uint32_t>(entity), newClipId);
            }
        }
        
        // Update sprite flip based on facing direction
        // Right = positive scale, Left = negative scale
        float scaleX = std::abs(transform.scale.x);
        if (stateComp.facing == FacingDirection::Left) {
            transform.scale.x = -scaleX;
        } else {
            transform.scale.x = scaleX;
        }
        
        // Commit state change
        stateComp.commitState();
    }
}

std::string AnimationStateMachineSystem::getClipId(AnimationState state, FacingDirection facing, const std::string& entityType) {
    // Naming convention: <entityType>_<state>_<direction>
    // Examples: "player_walk_right", "enemy_idle_right"
    // Since we only have left/right, we use "right" as base and flip sprite for left
    
    std::string stateStr;
    switch (state) {
        case AnimationState::Idle:
            stateStr = "idle";
            break;
        case AnimationState::Walk:
            stateStr = "walk";
            break;
        default:
            stateStr = "idle";
    }
    
    // For now, our assets seem to be:
    // player_idle, player_walk_right, etc.
    // If state is walk, we might need direction suffix if assets have it.
    // If state is idle, maybe no suffix?
    // Let's look at resources.json: "player_idle", "player_walk_right"
    
    // NOTE: Based on user feedback:
    // Idle: player_idle, enemy_idle (no suffix)
    // Walk: player_walk_right, enemy_walk_right (WITH suffix)
    
    if (state == AnimationState::Walk) {
        return entityType + "_" + stateStr + "_right";
    }
    
    // Default/Idle usually just <type>_<state> based on resources.json (e.g. "player_idle")
    return entityType + "_" + stateStr;
}

} // namespace engine
