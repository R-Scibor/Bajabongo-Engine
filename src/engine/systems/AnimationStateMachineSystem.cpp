#include "engine/pch.h"
#include "AnimationStateMachineSystem.hpp"
#include "engine/core/EngineContext.hpp"
#include "engine/core/ILoggerManager.hpp"
#include "engine/core/ILogger.hpp"
#include "engine/components/AnimationStateComponent.hpp"
#include "engine/components/AnimationComponent.hpp"
#include "engine/components/TransformComponent.hpp"
#include "engine/components/MetaComponent.hpp"
#include "engine/rendering/AnimationClip.hpp"
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
    
    // Explicitly iterate to avoid iterator issues
    view.each([&](entt::entity entity, AnimationStateComponent& stateComp, AnimationComponent& animComp, TransformComponent& transform) {
        // Only update if state changed
        if (!stateComp.stateChanged()) {
            return;
        }
        
        // 1. Determine Animation Set (fallback logic)
        std::string animSet = stateComp.animationSetId;
        
        if (animSet.empty()) {
            // Fallback: Try MetaComponent (Archetype Name)
            if (auto* meta = registry.try_get<MetaComponent>(entity)) {
                // Heuristic: If archetype name contains "player" or "enemy", guess the set
                if (meta->name.find("player") != std::string::npos) {
                    animSet = "player";
                } else if (meta->name.find("enemy") != std::string::npos) {
                     // e.g., "enemy_grunt" -> "enemy" (or keep full name if unique assets exist)
                     // For now, map to generic "player" set if they share assets (like enemy_1 currently does)
                     // BUT if we want true decoupling, we should rely on what's provided.
                     // Let's default to the meta name itself if nothing else matches.
                     animSet = "player"; // HACK for current assets where enemy uses player anims
                } else {
                     animSet = meta->name;
                }
            } else {
                animSet = "player"; // Absolute fallback
            }
        }
        
        // 2. Map state + direction -> clip ID
        std::string newClipId = getClipId(stateComp.state, stateComp.facing, animSet);
        
        // 3. Update animation if clip changed
        if (animComp.currentClipId != newClipId) {
            animComp.currentClipId = newClipId;
            animComp.reset();
            
            if (m_logger) {
                m_logger->trace("Entity {} -> {}", static_cast<uint32_t>(entity), newClipId);
            }
        }
        
        // 4. Update sprite flip based on facing direction
        // Right = positive scale, Left = negative scale
        // NOTE: We assume ALL side-view animations are authoring-time facing RIGHT.
        // Therefore, facing LEFT requires a flip.
        float scaleX = std::abs(transform.scale.x);
        if (stateComp.facing == FacingDirection::Left) {
            transform.scale.x = -scaleX;
        } else {
            transform.scale.x = scaleX;
        }
        
        // Commit state change
        stateComp.commitState();
    });
}

std::string AnimationStateMachineSystem::getClipId(AnimationState state, FacingDirection facing, const std::string& animSetId) {
    // Naming convention: <animSetId>_<state>[_suffix]
    // 
    // Rules from User/AssetBaker:
    // 1. Idle: "<animSetId>_idle" (e.g., "player_idle")
    // 2. Walk: "<animSetId>_walk_anim" (e.g., "player_walk_anim")
    //
    // Note: We ignore facing in the string construction because we use flipping for Left/Right.
    
    std::string clipId = animSetId;
    
    switch (state) {
        case AnimationState::Idle:
            clipId += "_idle";
            break;
        case AnimationState::Walk:
            clipId += "_walk_anim";
            break;
        case AnimationState::Aim:
            clipId += "_aim";
            break;
        case AnimationState::Dead:
            clipId += "_dead";
            break;
        default:
            clipId += "_idle";
            break;
    }

    if (m_context.m_animationLibrary) {
        // Try to use _anim first
        std::string withAnim = clipId + "_anim";
        if (m_context.m_animationLibrary->getClip(withAnim)) {
            return withAnim;
        }
        
        // If not present, try to use without _anim
        if (m_context.m_animationLibrary->getClip(clipId)) {
            return clipId;
        }

        // Fallback: If "Aim" is missing, try "Idle"
        if (state == AnimationState::Aim) {
            std::string idleClip = animSetId + "_idle";
             if (m_context.m_animationLibrary->getClip(idleClip)) {
                return idleClip;
            }
        }
    }
    
    return clipId;
}

} // namespace engine
