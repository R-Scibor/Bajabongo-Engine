#include "engine/pch.h"
#include "EnemyAISystem.hpp"
#include "game/components/EnemyComponent.hpp"
#include "game/components/PlayerComponent.hpp"
#include "game/components/HealthComponent.hpp"
#include "engine/components/TransformComponent.hpp"
#include "engine/components/PhysicsBodyComponent.hpp"
#include "engine/core/ILoggerManager.hpp"
#include "engine/physics/PhysicsConstants.hpp"
#include <box2d/box2d.h>

namespace game {

EnemyAISystem::EnemyAISystem(engine::EngineContext& context)
    : mContext(context)
    , mRegistry(*context.m_registry)
    , mWorldId(context.m_physicsWorld)
{
    if (context.m_logManager) {
        mLogger = context.m_logManager->GetLogger("EnemyAI");
    }
    
    if (mLogger) {
        mLogger->info("EnemyAISystem initialized.");
    }
}

void EnemyAISystem::update(float fixedDeltaTime) {
    if (mLogger) {
        mLogger->trace("EnemyAISystem::update() called, dt={:.4f}", fixedDeltaTime);
    }
    
    // Phase-based update (will be implemented incrementally)
    updateDetection();
    updateStates(fixedDeltaTime);
    updateMovement(fixedDeltaTime);
    updateCombat();
    updateStuckDetection(fixedDeltaTime);
}

void EnemyAISystem::updateDetection() {
    // DIAGNOSTIC: Count entities
    auto playerView = mRegistry.view<PlayerComponent, engine::TransformComponent>();
    auto enemyView = mRegistry.view<EnemyComponent, engine::TransformComponent>();
    
    if (mLogger) {
        mLogger->debug("Detection pass: {} players, {} enemies", 
                      playerView.size_hint(), 
                      enemyView.size_hint());
    }

    // Find player
    entt::entity playerEntity = entt::null;
    engine::Vector2f playerPos{0.0f, 0.0f};
    
    // Explicitly iterating using each with correct lambda signature
    playerView.each([&](entt::entity entity, const PlayerComponent&, const engine::TransformComponent& transform) {
        playerEntity = entity;
        playerPos = transform.position;
        
        if (mLogger) {
            mLogger->debug("Player found: entity={}, pos=({:.1f}, {:.1f})", 
                          entt::to_integral(entity), playerPos.x, playerPos.y);
        }
    });
    
    if (playerEntity == entt::null) {
        if (mLogger) {
            mLogger->warn("No player entity found! Detection cannot run.");
        }
        return;
    }
    
    // Check each enemy
    enemyView.each([&](entt::entity entity, EnemyComponent& enemy, const engine::TransformComponent& transform) {
        
        if (mLogger) {
            mLogger->debug("Enemy entity={}, pos=({:.1f}, {:.1f}), state={}", 
                          entt::to_integral(entity), 
                          transform.position.x, transform.position.y,
                          static_cast<int>(enemy.currentState));
        }

        // Skip dead enemies
        if (auto* health = mRegistry.try_get<HealthComponent>(entity)) {
            if (health->currentHp <= 0.0f) {
                enemy.currentState = EnemyState::Dead;
                return; 
            }
        }
        
        // Distance check
        engine::Vector2f toPlayer = playerPos - transform.position;
        float distanceSq = toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y;
        float distance = std::sqrt(distanceSq);
        float detectionRadiusSq = enemy.detectionRadius * enemy.detectionRadius;
        
        if (mLogger) {
            mLogger->debug("Enemy {} -> Player: distance={:.1f}, detectionRadius={:.1f}, inRange={}", 
                          entt::to_integral(entity), 
                          distance, 
                          enemy.detectionRadius,
                          distance < enemy.detectionRadius);
        }
        
        if (distanceSq < detectionRadiusSq) {
            // Line-of-sight check
            bool hasLoS = hasLineOfSight(transform.position, playerPos);
            
            if (mLogger) {
                mLogger->debug("Enemy {} in range! LoS check result: {}", 
                              entt::to_integral(entity), hasLoS);
            }
            
            enemy.hasLineOfSight = hasLoS;
            
            if (hasLoS) {
                if (mLogger) {
                    mLogger->info("Enemy {} DETECTED player!", entt::to_integral(entity));
                }
                
                enemy.targetEntity = playerEntity;
                enemy.lastKnownPosition = playerPos;
                enemy.lastSeenTimer = 0.0f;
                
                // Alert nearby enemies
                if (!enemy.isAlerted) {
                    enemy.isAlerted = true;
                    alertNearbyEnemies(entity, playerPos);
                }
            }
        } else {
            enemy.hasLineOfSight = false;
            
            // Memory decay
            if (enemy.targetEntity != entt::null) {
                enemy.lastSeenTimer += 0.016f; // Approximate frame time
                if (enemy.lastSeenTimer > enemy.memoryDuration) {
                    enemy.targetEntity = entt::null;
                }
            }
        }
    });
}

void EnemyAISystem::updateStates(float dt) {
    // Stub for Phase 3
}

void EnemyAISystem::updateMovement(float dt) {
    // Stub for Phase 4
}

void EnemyAISystem::updateCombat() {
    // Stub for Phase 5
}

void EnemyAISystem::updateStuckDetection(float dt) {
    // Stub for Phase 6
}

bool EnemyAISystem::hasLineOfSight(const engine::Vector2f& start, const engine::Vector2f& end) {
    if (mLogger) {
        mLogger->trace("LoS raycast: ({:.1f}, {:.1f}) -> ({:.1f}, {:.1f})", 
                      start.x, start.y, end.x, end.y);
    }
    
    b2QueryFilter filter = b2DefaultQueryFilter();
    filter.categoryBits = 0xFFFFFFFF; // Check all categories
    filter.maskBits = engine::PhysicsCategory::VisibilityBlocker | engine::PhysicsCategory::Wall;
    
    // Calculate ray direction
    engine::Vector2f direction = end - start;
    float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    
    if (length < 0.01f) return true; // Same position = visible
    
    direction = direction / length; // Normalize
    
    b2Vec2 origin = {start.x, start.y};
    b2Vec2 translation = {direction.x * length, direction.y * length};
    
    b2RayResult result = b2World_CastRayClosest(mWorldId, origin, translation, filter);
    
    if (mLogger) {
        if (result.hit) {
            mLogger->debug("LoS blocked! Hit at fraction={:.4f}", result.fraction);
        } else {
            mLogger->trace("Raycast result: hit={}, fraction={:.4f}", result.hit, result.fraction);
        }
    }
    
    return !result.hit;
}

void EnemyAISystem::alertNearbyEnemies(entt::entity source, const engine::Vector2f& position) {
    if (mLogger) {
        mLogger->info("Alerting enemies near entity={} at ({:.1f}, {:.1f})", 
                     entt::to_integral(source), position.x, position.y);
    }
    
    auto& sourceEnemy = mRegistry.get<EnemyComponent>(source);
    const auto& sourceTransform = mRegistry.get<engine::TransformComponent>(source);
    
    float alertRadiusSq = sourceEnemy.alertRadius * sourceEnemy.alertRadius;
    
    int alertCount = 0;
    
    auto enemyView = mRegistry.view<EnemyComponent, engine::TransformComponent>();
    enemyView.each([&](entt::entity entity, EnemyComponent& enemy, const engine::TransformComponent& transform) {
        if (entity == source) return;
        
        engine::Vector2f toSource = sourceTransform.position - transform.position;
        float distSq = toSource.x * toSource.x + toSource.y * toSource.y;
        float dist = std::sqrt(distSq);
        
        if (mLogger) {
            mLogger->debug("Checking enemy {}: distance={:.1f}, alertRadius={:.1f}", 
                          entt::to_integral(entity), dist, sourceEnemy.alertRadius);
        }
        
        if (distSq < alertRadiusSq) {
            enemy.currentState = EnemyState::Alert;
            enemy.lastKnownPosition = position;
            enemy.isAlerted = true;
            alertCount++;
            
            if (mLogger) {
                mLogger->info("Enemy {} alerted by enemy {}", entt::to_integral(entity), entt::to_integral(source));
            }
        }
    });
    
    if (mLogger) {
        mLogger->info("Alert complete: {} enemies notified", alertCount);
    }
}

engine::Vector2f EnemyAISystem::castRayWithSlide(const engine::Vector2f& origin, 
                                                  const engine::Vector2f& direction, float distance) {
    // Stub for Phase 4
    return origin + direction * distance;
}

} // namespace game
