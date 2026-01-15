#include "engine/pch.h"
#include "EnemyAISystem.hpp"
#include "engine/components/AnimationStateComponent.hpp"
#include "engine/components/PhysicsBodyComponent.hpp"
#include "engine/components/TransformComponent.hpp"
#include "game/components/PlayerComponent.hpp"
#include <entt/entt.hpp>
#include <box2d/box2d.h>

namespace game {

EnemyAISystem::EnemyAISystem(engine::EngineContext& context)
    : m_context(context) {}

void EnemyAISystem::update(float dt) {
    auto& registry = *m_context.m_registry;
    
    // Simple: All entities with AnimationState but WITHOUT PlayerComponent are enemies
    auto view = registry.view<engine::AnimationStateComponent, 
                               engine::PhysicsBodyComponent>
                        (entt::exclude<PlayerComponent>);
    
    for (auto entity : view) {
        auto& animState = view.get<engine::AnimationStateComponent>(entity);
        auto& body = view.get<engine::PhysicsBodyComponent>(entity);
        
        if (!b2Body_IsValid(body.bodyId)) continue;
        
        // Check if enemy is moving
        b2Vec2 velocity = b2Body_GetLinearVelocity(body.bodyId);
        float speed = std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
        
        // Set animation state based on movement
        if (speed > 0.1f) {
            animState.state = engine::AnimationState::Walk;
            // Set facing based on velocity direction
            if (velocity.x < -0.01f) {
                animState.facing = engine::FacingDirection::Left;
            } else if (velocity.x > 0.01f) {
                animState.facing = engine::FacingDirection::Right;
            }
        } else {
            animState.state = engine::AnimationState::Idle;
        }
    }
}

} // namespace game
