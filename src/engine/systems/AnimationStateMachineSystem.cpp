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
    // Helper to get suffix for a state
    auto getSuffix = [](AnimationState s) -> std::string {
        switch (s) {
            case AnimationState::Idle: return "_idle";
            case AnimationState::Walk: return "_walk_anim";
            case AnimationState::Aim:  return "_aim";
            case AnimationState::Dead: return "_dead";
            case AnimationState::Heal: return "_heal";
            default:                   return "_idle";
        }
    };

    // Helper to check if a specific clip ID exists (trying _anim variant too)
    auto tryResolve = [&](const std::string& baseId) -> std::string {
        if (!m_context.m_animationLibrary) return "";
        
        // Try with _anim first (legacy/consistency)
        std::string withAnim = baseId + "_anim";
        if (m_context.m_animationLibrary->getClip(withAnim)) return withAnim;
        
        // Try exact match
        if (m_context.m_animationLibrary->getClip(baseId)) return baseId;
        
        return "";
    };

    std::string currentSet = animSetId;
    
    // Loop to strip suffixes from animSetId (e.g. player_ak -> player)
    while (true) {
        std::string suffix = getSuffix(state);
        std::string candidateId = currentSet + suffix;
        
        // 1. Try exact state match
        std::string result = tryResolve(candidateId);
        if (!result.empty()) return result;

        // 2. Apply State-based Fallbacks (within this set)
        // For Aim: We prefer "Correct Weapon, Wrong State" (Idle) over "Wrong Weapon, Correct State"
        // so we check for Idle immediately before stripping the set name.
        if (state == AnimationState::Aim) {
            std::string idleSuffix = getSuffix(AnimationState::Idle);
            std::string idleId = currentSet + idleSuffix;
            result = tryResolve(idleId);
            if (!result.empty()) return result;
        }

        // Note: For Heal, we prefer "Wrong Weapon, Correct State" (Generic Heal) over "Correct Weapon, Wrong State" (Idle).
        // So we skip the local fallback and allow the loop to try the parent set's Heal first.

        // 3. Try to strip the set name for the next iteration
        // e.g. "player_ak" -> "player"
        size_t lastUnderscore = currentSet.find_last_of('_');
        if (lastUnderscore != std::string::npos && lastUnderscore > 0) {
            currentSet = currentSet.substr(0, lastUnderscore);
        } else {
            break; // No more parents
        }
    }

    // If absolutely nothing found, return the constructed ID for the original request
    
    // Final Global Fallback for Heal: If we couldn't find Heal in ANY set, try finding Idle in ANY set.
    if (state == AnimationState::Heal) {
        return getClipId(AnimationState::Idle, facing, animSetId);
    }

    return animSetId + getSuffix(state);
}

} // namespace engine
