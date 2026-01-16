#include "engine/pch.h"
#include "EnemyAISystem.hpp"
#include "game/components/EnemyComponent.hpp"
#include "game/components/PlayerComponent.hpp"
#include "game/components/HealthComponent.hpp"
#include "game/components/PatrolBehaviorComponent.hpp"
#include "game/components/RushBehaviorComponent.hpp"
#include "game/components/WeaponComponent.hpp"
#include "game/components/TurretBehaviorComponent.hpp"
#include "game/components/SniperBehaviorComponent.hpp"
#include "engine/components/TransformComponent.hpp"
#include "engine/components/PhysicsBodyComponent.hpp"
#include "engine/core/ILoggerManager.hpp"
#include "engine/physics/PhysicsConstants.hpp"
#include "game/components/VisibilityComponent.hpp"
#include "engine/components/LifetimeComponent.hpp"
#include <box2d/box2d.h>
#include <cmath>
#include <random>
#include <limits>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

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
    // Phase-based update (will be implemented incrementally)
    updateDetection(fixedDeltaTime);
    updateStates(fixedDeltaTime);
    updateMovement(fixedDeltaTime);
    updateCombat(fixedDeltaTime);
    updateStuckDetection(fixedDeltaTime);
}

void EnemyAISystem::updateDetection(float dt) {
    // DIAGNOSTIC: Count entities
    auto playerView = mRegistry.view<PlayerComponent, engine::TransformComponent>();
    auto enemyView = mRegistry.view<EnemyComponent, engine::TransformComponent>();
    
    // Find player
    entt::entity playerEntity = entt::null;
    engine::Vector2f playerPos{0.0f, 0.0f};
    
    // Explicitly iterating using each with correct lambda signature
    playerView.each([&](entt::entity entity, const PlayerComponent&, const engine::TransformComponent& transform) {
        playerEntity = entity;
        playerPos = transform.position;
    });
    
    if (playerEntity == entt::null) {
        return;
    }
    
    // Check each enemy
    enemyView.each([&](entt::entity entity, EnemyComponent& enemy, const engine::TransformComponent& transform) {
        
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
        
        if (distanceSq < detectionRadiusSq) {
            // Line-of-sight check
            bool hasLoS = hasLineOfSight(transform.position, playerPos);
            
            enemy.hasLineOfSight = hasLoS;
            
            if (hasLoS) {
                //if (mLogger) {mLogger->info("Enemy {} DETECTED player!", entt::to_integral(entity));}
                
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
                enemy.lastSeenTimer += dt; // Use passed delta time
                if (enemy.lastSeenTimer > enemy.memoryDuration) {
                    enemy.targetEntity = entt::null;
                }
            }
        }
    });
}

void EnemyAISystem::updateStates(float dt) {
    auto view = mRegistry.view<EnemyComponent, engine::TransformComponent>();
    
    view.each([&](entt::entity entity, EnemyComponent& enemy, const engine::TransformComponent& transform) {
        
        // Update state timer
        enemy.stateTimer += dt;
        
        EnemyState oldState = enemy.currentState;
        
        // State transition logic
        switch (enemy.currentState) {
            case EnemyState::Idle:
                transitionFromIdle(entity, enemy, transform);
                break;
            case EnemyState::Patrol:
                transitionFromPatrol(entity, enemy, transform);
                break;
            case EnemyState::Alert:
                transitionFromAlert(entity, enemy, transform);
                break;
            case EnemyState::Chase:
                transitionFromChase(entity, enemy, transform);
                break;
            case EnemyState::Shoot:
                transitionFromShoot(entity, enemy, transform);
                break;
            case EnemyState::Dead:
                // Terminal state
                break;
            // Other states...
        }
        
        // Log state changes
        if (oldState != enemy.currentState && mLogger) {
            mLogger->debug("Enemy {} transitioned from {} to {}", 
                           entt::to_integral(entity), 
                           static_cast<int>(oldState), 
                           static_cast<int>(enemy.currentState));
        }
    });
}

void EnemyAISystem::transitionFromIdle(entt::entity entity, EnemyComponent& enemy, const engine::TransformComponent& transform) {
    // Idle → Patrol: If has waypoints
    if (mRegistry.any_of<PatrolBehaviorComponent>(entity)) {
        enemy.currentState = EnemyState::Patrol;
        enemy.stateTimer = 0.0f;
        return;
    }
    
    // Idle → Chase: If player detected
    if (enemy.targetEntity != entt::null && enemy.hasLineOfSight) {
        enemy.currentState = EnemyState::Chase;
        enemy.stateTimer = 0.0f;
        return;
    }
    
    // Idle → Alert: If alerted by ally
    if (enemy.isAlerted && enemy.targetEntity == entt::null) {
        enemy.currentState = EnemyState::Alert;
        enemy.stateTimer = 0.0f;
        return;
    }
}

void EnemyAISystem::transitionFromPatrol(entt::entity entity, EnemyComponent& enemy, const engine::TransformComponent& transform) {
    // Check HP for retreat
    if (auto* health = mRegistry.try_get<HealthComponent>(entity)) {
        if (health->currentHp < health->maxHp * enemy.retreatHpPercent) {
            enemy.currentState = EnemyState::Retreat;
            enemy.stateTimer = 0.0f;
            return;
        }
    }
    
    // Patrol → Chase: If player spotted
    if (enemy.targetEntity != entt::null && enemy.hasLineOfSight) {
        enemy.currentState = EnemyState::Chase;
        enemy.stateTimer = 0.0f;
        return;
    }
    
    // Patrol → Alert: If alerted
    if (enemy.isAlerted && enemy.targetEntity == entt::null) {
        enemy.currentState = EnemyState::Alert;
        enemy.stateTimer = 0.0f;
        return;
    }
}

void EnemyAISystem::transitionFromAlert(entt::entity entity, EnemyComponent& enemy, const engine::TransformComponent& transform) {
    // Alert → Chase: If player spotted
    if (enemy.targetEntity != entt::null && enemy.hasLineOfSight) {
        enemy.currentState = EnemyState::Chase;
        enemy.stateTimer = 0.0f;
        return;
    }
    
    // Alert → Approach: Move to last known position
    if (enemy.targetEntity == entt::null && enemy.isAlerted) {
        enemy.currentState = EnemyState::Approach;
        enemy.stateTimer = 0.0f;
        return;
    }
    
    // Alert → Patrol/Idle: Timeout
    if (enemy.stateTimer > 10.0f) {
        if (mRegistry.any_of<PatrolBehaviorComponent>(entity)) {
            enemy.currentState = EnemyState::Patrol;
        } else {
            enemy.currentState = EnemyState::Idle;
        }
        enemy.isAlerted = false;
        enemy.stateTimer = 0.0f;
        return;
    }
}

void EnemyAISystem::transitionFromChase(entt::entity entity, EnemyComponent& enemy, const engine::TransformComponent& transform) {
    // Check HP for retreat
    if (auto* health = mRegistry.try_get<HealthComponent>(entity)) {
        if (health->currentHp < health->maxHp * enemy.retreatHpPercent) {
            enemy.currentState = EnemyState::Retreat;
            enemy.stateTimer = 0.0f;
            return;
        }
    }
    
    // Chase → Shoot: In range with LoS
    if (enemy.targetEntity != entt::null && enemy.hasLineOfSight) {
        if (!mRegistry.valid(enemy.targetEntity)) {
            enemy.targetEntity = entt::null;
            return;
        }
        
        const auto& targetTransform = mRegistry.get<engine::TransformComponent>(enemy.targetEntity);
        engine::Vector2f toTarget = targetTransform.position - transform.position;
        float distSq = toTarget.x * toTarget.x + toTarget.y * toTarget.y;
        
        if (distSq < enemy.attackRange * enemy.attackRange) {
            enemy.currentState = EnemyState::Shoot;
            enemy.stateTimer = 0.0f;
            return;
        }
    }
    
    // Chase → Approach: Lost LoS
    if (enemy.targetEntity != entt::null && !enemy.hasLineOfSight) {
        enemy.currentState = EnemyState::Approach;
        enemy.stateTimer = 0.0f;
        return;
    }
    
    // Chase → Rush: Special behavior
    if (mRegistry.any_of<RushBehaviorComponent>(entity)) {
        auto& rush = mRegistry.get<RushBehaviorComponent>(entity);
        if (enemy.targetEntity != entt::null && mRegistry.valid(enemy.targetEntity)) {
            const auto& targetTransform = mRegistry.get<engine::TransformComponent>(enemy.targetEntity);
            engine::Vector2f toTarget = targetTransform.position - transform.position;
            float distSq = toTarget.x * toTarget.x + toTarget.y * toTarget.y;
            
            if (distSq < rush.rushActivationRange * rush.rushActivationRange && rush.rushCooldownTimer <= 0.0f) {
                enemy.currentState = EnemyState::Rush;
                enemy.stateTimer = 0.0f;
                return;
            }
        }
    }
}

void EnemyAISystem::transitionFromShoot(entt::entity entity, EnemyComponent& enemy, const engine::TransformComponent& transform) {
    // Shoot → Retreat: Low HP
    if (auto* health = mRegistry.try_get<HealthComponent>(entity)) {
        if (health->currentHp < health->maxHp * enemy.retreatHpPercent) {
            enemy.currentState = EnemyState::Retreat;
            enemy.stateTimer = 0.0f;
            return;
        }
    }
    
    // Shoot → Chase: Target moved out of range
    if (enemy.targetEntity != entt::null && mRegistry.valid(enemy.targetEntity)) {
        const auto& targetTransform = mRegistry.get<engine::TransformComponent>(enemy.targetEntity);
        engine::Vector2f toTarget = targetTransform.position - transform.position;
        float distSq = toTarget.x * toTarget.x + toTarget.y * toTarget.y;
        
        if (distSq >= enemy.attackRange * enemy.attackRange) {
            enemy.currentState = EnemyState::Chase;
            enemy.stateTimer = 0.0f;
            return;
        }
    }
    
    // Shoot → Approach: Lost LoS
    if (!enemy.hasLineOfSight) {
        enemy.currentState = EnemyState::Approach;
        enemy.stateTimer = 0.0f;
        return;
    }
}

void EnemyAISystem::updateMovement(float dt) {
    auto view = mRegistry.view<EnemyComponent, engine::TransformComponent, engine::PhysicsBodyComponent>();
    
    view.each([&](entt::entity entity, EnemyComponent& enemy, const engine::TransformComponent& transform, engine::PhysicsBodyComponent& bodyComp) {
        
        if (!b2Body_IsValid(bodyComp.bodyId)) return;
        
        // Handle Stun/Shove Timer
        if (enemy.stunTimer > 0.0f) {
            enemy.stunTimer -= dt;
            if (enemy.stunTimer > 0.0f) {
                // Skip movement updates while stunned to allow physics impulse to work
                return; 
            }
        }
        
        engine::Vector2f desiredVelocity{0.0f, 0.0f};
        
        // Movement based on current state
        switch (enemy.currentState) {
            case EnemyState::Patrol:
                desiredVelocity = calculatePatrolMovement(entity, enemy, transform);
                break;
            case EnemyState::Chase:
            case EnemyState::Approach:
                desiredVelocity = calculateChaseMovement(entity, enemy, transform);
                break;
            case EnemyState::Rush:
                desiredVelocity = calculateRushMovement(entity, enemy, transform);
                break;
            case EnemyState::Retreat:
                desiredVelocity = calculateRetreatMovement(entity, enemy, transform);
                break;
            case EnemyState::Shoot:
            case EnemyState::Turret:
                // Stationary while shooting
                desiredVelocity = {0.0f, 0.0f};
                break;
            default:
                desiredVelocity = {0.0f, 0.0f};
                break;
        }
        
        // Apply raycast-assisted pathfinding with wall sliding
        engine::Vector2f finalVelocity = applyWallSliding(transform.position, desiredVelocity, dt);
        
        // Apply stuck nudge if active
        if (enemy.stuckCounter > 0) {
             // Simple lateral nudge based on stuck counter parity to alternate directions or just one consistent side
             // For corners, usually 90 degrees to desired velocity works best.
             // We'll try to nudge them to the right relative to their movement first.
             float nudgeSpeed = enemy.moveSpeed * 0.5f;
             
             // Create a perpendicular vector (x, y) -> (-y, x)
             engine::Vector2f nudgeDir = {-desiredVelocity.y, desiredVelocity.x};
             float nudgeMag = std::sqrt(nudgeDir.x * nudgeDir.x + nudgeDir.y * nudgeDir.y);
             
             if (nudgeMag > 0.01f) {
                 nudgeDir = {nudgeDir.x / nudgeMag, nudgeDir.y / nudgeMag};
                 
                 // Combine velocities
                 finalVelocity.x += nudgeDir.x * nudgeSpeed;
                 finalVelocity.y += nudgeDir.y * nudgeSpeed;
             }
        }
        
        b2Body_SetLinearVelocity(bodyComp.bodyId, {finalVelocity.x, finalVelocity.y});
        
        // Update last valid position if moving successfully
        float velMagSq = finalVelocity.x * finalVelocity.x + finalVelocity.y * finalVelocity.y;
        if (velMagSq > 1.0f) { // Moving at least 1 unit/sec
            enemy.lastValidPosition = transform.position;
        }
    });
}

engine::Vector2f EnemyAISystem::calculatePatrolMovement(entt::entity entity, EnemyComponent& enemy, const engine::TransformComponent& transform) {
    auto* patrol = mRegistry.try_get<PatrolBehaviorComponent>(entity);
    if (!patrol || patrol->waypoints.empty()) {
        return {0.0f, 0.0f};
    }
    
    // Get current waypoint
    engine::Vector2f targetWaypoint = patrol->waypoints[patrol->currentWaypointIndex];
    engine::Vector2f toWaypoint = {
        targetWaypoint.x - transform.position.x,
        targetWaypoint.y - transform.position.y
    };
    float distSq = toWaypoint.x * toWaypoint.x + toWaypoint.y * toWaypoint.y;
    
    // Check if reached waypoint
    if (distSq < patrol->waypointReachThreshold * patrol->waypointReachThreshold) {
        // Advance to next waypoint
        if (patrol->loopMode) {
            patrol->currentWaypointIndex = (patrol->currentWaypointIndex + 1) % patrol->waypoints.size();
        } else {
            // Ping-pong mode
            if (patrol->reversing) {
                if (patrol->currentWaypointIndex > 0) {
                    patrol->currentWaypointIndex--;
                }
                if (patrol->currentWaypointIndex == 0) {
                    patrol->reversing = false;
                }
            } else {
                patrol->currentWaypointIndex++;
                if (patrol->currentWaypointIndex >= patrol->waypoints.size() - 1) {
                    patrol->currentWaypointIndex = patrol->waypoints.size() - 1;
                    patrol->reversing = true;
                }
            }
        }
        
        // Recalculate after waypoint change
        targetWaypoint = patrol->waypoints[patrol->currentWaypointIndex];
        toWaypoint = {
            targetWaypoint.x - transform.position.x,
            targetWaypoint.y - transform.position.y
        };
        distSq = toWaypoint.x * toWaypoint.x + toWaypoint.y * toWaypoint.y;
    }
    
    // Normalize and apply speed
    if (distSq > 0.01f) {
        float dist = std::sqrt(distSq);
        return {
            (toWaypoint.x / dist) * enemy.moveSpeed,
            (toWaypoint.y / dist) * enemy.moveSpeed
        };
    }
    
    return {0.0f, 0.0f};
}

engine::Vector2f EnemyAISystem::calculateChaseMovement(entt::entity entity, EnemyComponent& enemy, const engine::TransformComponent& transform) {
    if (enemy.targetEntity == entt::null || !mRegistry.valid(enemy.targetEntity)) {
        return {0.0f, 0.0f};
    }
    
    engine::Vector2f targetPos;
    if (enemy.currentState == EnemyState::Chase && enemy.hasLineOfSight) {
        targetPos = mRegistry.get<engine::TransformComponent>(enemy.targetEntity).position;
    } else {
        // Approach last known position
        targetPos = enemy.lastKnownPosition;
    }
    
    engine::Vector2f toTarget = {
        targetPos.x - transform.position.x,
        targetPos.y - transform.position.y
    };
    float distSq = toTarget.x * toTarget.x + toTarget.y * toTarget.y;
    
    if (distSq > 0.01f) {
        float dist = std::sqrt(distSq);
        return {
            (toTarget.x / dist) * enemy.moveSpeed,
            (toTarget.y / dist) * enemy.moveSpeed
        };
    }
    
    return {0.0f, 0.0f};
}

engine::Vector2f EnemyAISystem::calculateRushMovement(entt::entity entity, EnemyComponent& enemy, const engine::TransformComponent& transform) {
    auto* rush = mRegistry.try_get<RushBehaviorComponent>(entity);
    if (!rush) {
        return calculateChaseMovement(entity, enemy, transform);
    }
    
    if (enemy.targetEntity == entt::null || !mRegistry.valid(enemy.targetEntity)) {
        return {0.0f, 0.0f};
    }
    
    engine::Vector2f targetPos = mRegistry.get<engine::TransformComponent>(enemy.targetEntity).position;
    engine::Vector2f toTarget = {
        targetPos.x - transform.position.x,
        targetPos.y - transform.position.y
    };
    float distSq = toTarget.x * toTarget.x + toTarget.y * toTarget.y;
    
    if (distSq > 0.01f) {
        float dist = std::sqrt(distSq);
        return {
            (toTarget.x / dist) * rush->rushSpeed,
            (toTarget.y / dist) * rush->rushSpeed
        };
    }
    
    return {0.0f, 0.0f};
}

engine::Vector2f EnemyAISystem::calculateRetreatMovement(entt::entity entity, EnemyComponent& enemy, const engine::TransformComponent& transform) {
    // Run away from player
    if (enemy.targetEntity == entt::null || !mRegistry.valid(enemy.targetEntity)) {
        // Run to last valid position
        engine::Vector2f toSafe = {
            enemy.lastValidPosition.x - transform.position.x,
            enemy.lastValidPosition.y - transform.position.y
        };
        float distSq = toSafe.x * toSafe.x + toSafe.y * toSafe.y;
        if (distSq > 0.01f) {
            float dist = std::sqrt(distSq);
            return {
                (toSafe.x / dist) * enemy.moveSpeed * 1.2f,
                (toSafe.y / dist) * enemy.moveSpeed * 1.2f
            };
        }
        return {0.0f, 0.0f};
    }
    
    engine::Vector2f playerPos = mRegistry.get<engine::TransformComponent>(enemy.targetEntity).position;
    engine::Vector2f awayFromPlayer = {
        transform.position.x - playerPos.x,
        transform.position.y - playerPos.y
    };
    float distSq = awayFromPlayer.x * awayFromPlayer.x + awayFromPlayer.y * awayFromPlayer.y;
    
    if (distSq > 0.01f) {
        float dist = std::sqrt(distSq);
        return {
            (awayFromPlayer.x / dist) * enemy.moveSpeed * 1.2f,
            (awayFromPlayer.y / dist) * enemy.moveSpeed * 1.2f
        };
    }
    
    return {0.0f, 0.0f};
}

engine::Vector2f EnemyAISystem::applyWallSliding(const engine::Vector2f& position, const engine::Vector2f& desiredVelocity, float dt) {
    float velocityMag = std::sqrt(desiredVelocity.x * desiredVelocity.x + desiredVelocity.y * desiredVelocity.y);
    if (velocityMag < 0.01f) {
        return desiredVelocity;
    }
    
    // Normalize direction
    engine::Vector2f direction = {
        desiredVelocity.x / velocityMag,
        desiredVelocity.y / velocityMag
    };
    
    // Increased lookahead for better wall anticipation
    float rayLength = velocityMag * dt * 2.5f; 
    
    // Setup raycast filter
    b2QueryFilter filter = b2DefaultQueryFilter();
    filter.maskBits = engine::PhysicsCategory::Wall | engine::PhysicsCategory::VisibilityBlocker;
    
    // Prepare Box2D 3.0 raycast parameters
    b2Vec2 origin = {position.x, position.y};
    b2Vec2 translation = {direction.x * rayLength, direction.y * rayLength};
    
    // Perform raycast
    b2RayResult result = b2World_CastRayClosest(mWorldId, origin, translation, filter);
    
    if (!result.hit) {
        // Path is clear
        return desiredVelocity;
    }
    
    // Hit a wall - calculate slide direction
    engine::Vector2f hitNormal = {result.normal.x, result.normal.y};
    
    // Dot product to check if moving into wall
    float dotProduct = direction.x * hitNormal.x + direction.y * hitNormal.y;
    
    if (dotProduct > -0.01f) { // Only stop if moving AWAY, not parallel
        return desiredVelocity; // Continue with original velocity
    }
    
    // Project velocity onto wall surface (perpendicular to normal)
    engine::Vector2f slideDirection = {
        direction.x - hitNormal.x * dotProduct,
        direction.y - hitNormal.y * dotProduct
    };
    
    // Normalize slide direction
    float slideMag = std::sqrt(slideDirection.x * slideDirection.x + slideDirection.y * slideDirection.y);
    if (slideMag > 0.01f) {
        // Apply velocity boost to slide faster along walls
        float boostFactor = 1.1f;
        return {
            (slideDirection.x / slideMag) * velocityMag * boostFactor,
            (slideDirection.y / slideMag) * velocityMag * boostFactor
        };
    }
    
    // Stuck against wall
    return {0.0f, 0.0f};
}

void EnemyAISystem::updateCombat(float dt) {
    auto view = mRegistry.view<EnemyComponent, engine::TransformComponent, WeaponComponent>();
    
    view.each([&](entt::entity entity, EnemyComponent& enemy, engine::TransformComponent& transform, WeaponComponent& weapon) {
        // Reset weapon flags each frame
        weapon.wantsToShoot = false;
        weapon.wantsToReload = false;
        
        // Only shoot in specific states
        if (enemy.currentState != EnemyState::Shoot && 
            enemy.currentState != EnemyState::Camp && 
            enemy.currentState != EnemyState::Turret) {
            return;
        }
        
        // Validate target
        if (enemy.targetEntity == entt::null || !mRegistry.valid(enemy.targetEntity)) {
            return;
        }
        
        // Check line of sight
        if (!enemy.hasLineOfSight) {
            return;
        }
        
        // Get target position
        // Explicitly specifying the template argument for get
        const auto& targetTransform = mRegistry.get<engine::TransformComponent>(enemy.targetEntity);
        engine::Vector2f toTarget = targetTransform.position - transform.position;
        float distanceSq = toTarget.x * toTarget.x + toTarget.y * toTarget.y;
        
        // Calculate desired aim angle
        float desiredAimAngle = std::atan2(toTarget.y, toTarget.x);
        
        // TURRET BEHAVIOR
        if (auto* turret = mRegistry.try_get<TurretBehaviorComponent>(entity)) {
            float angleDiff = desiredAimAngle - weapon.aimAngle;
            
            // Normalize to [-PI, PI]
            while (angleDiff > M_PI) angleDiff -= 2.0f * M_PI;
            while (angleDiff < -M_PI) angleDiff += 2.0f * M_PI;
            
            // Rotate weapon aim
            float maxRotation = turret->rotationSpeed * dt;
            if (std::abs(angleDiff) < maxRotation) {
                weapon.aimAngle = desiredAimAngle;
            } else {
                weapon.aimAngle += (angleDiff > 0.0f ? maxRotation : -maxRotation);
            }
            
            // Flip sprite based on aim direction (LEFT vs RIGHT)
            float scaleX = std::abs(transform.scale.x);
            if (weapon.aimAngle > M_PI / 2.0f || weapon.aimAngle < -M_PI / 2.0f) {
                // Aiming left
                transform.scale.x = -scaleX;
            } else {
                // Aiming right
                transform.scale.x = scaleX;
            }
            
            // Shoot if aligned
            if (std::abs(angleDiff) < turret->shootAngleTolerance) {
                weapon.wantsToShoot = true;
            }
        }
        // SNIPER BEHAVIOR
        else if (auto* sniper = mRegistry.try_get<SniperBehaviorComponent>(entity)) {
            weapon.aimAngle = desiredAimAngle;
            
            if (!sniper->isAiming) {
                sniper->isAiming = true;
                sniper->aimTimer = 0.0f;
                if (mLogger) {
                    mLogger->debug("Enemy {} (Sniper) started aiming", entt::to_integral(entity));
                }
            }
            
            sniper->aimTimer += dt;
            
            if (sniper->aimTimer >= sniper->aimDuration) {
                weapon.wantsToShoot = true;
                sniper->isAiming = false;
                sniper->aimTimer = 0.0f;
                if (mLogger) {
                    mLogger->debug("Enemy {} (Sniper) fired", entt::to_integral(entity));
                }
            }
        }
        // NORMAL BEHAVIOR
        else {
            weapon.aimAngle = desiredAimAngle;
            
            if (distanceSq < enemy.attackRange * enemy.attackRange) {
                weapon.wantsToShoot = true;
            }
        }
        
        // AUTO-RELOAD
        if (weapon.currentAmmo == 0 && weapon.totalAmmo > 0) {
            weapon.wantsToReload = true;
        }
    });
}

void EnemyAISystem::updateStuckDetection(float dt) {
    auto view = mRegistry.view<EnemyComponent, engine::TransformComponent>();
    
    // Use .each() instead of range-based for loop which seems to be problematic in this ENTT version
    view.each([&](entt::entity entity, EnemyComponent& enemy, const engine::TransformComponent& transform) {
        // Only check if enemy is trying to move
        if (enemy.currentState == EnemyState::Idle || 
            enemy.currentState == EnemyState::Dead ||
            enemy.currentState == EnemyState::Shoot ||
            enemy.currentState == EnemyState::Turret) {
            // Not moving states - reset counter
            if (enemy.stuckCounter > 0) {
                enemy.stuckCounter = 0;
                if (mLogger) {
                    mLogger->debug("Enemy {} stopped moving, reset stuck counter", 
                                   entt::to_integral(entity));
                }
            }
            return; // Continue to next entity
        }
        
        enemy.stuckCheckTimer += dt;
        
        if (enemy.stuckCheckTimer >= enemy.stuckCheckInterval) {
            // Calculate distance moved since last check
            engine::Vector2f delta = transform.position - enemy.lastCheckedPosition;
            float distanceMovedSq = delta.x * delta.x + delta.y * delta.y;
            float distanceMoved = std::sqrt(distanceMovedSq);
            
            // Threshold: 10 units over 2 seconds = very slow/stuck
            const float MIN_MOVEMENT_THRESHOLD = 10.0f;
            
            if (distanceMoved < MIN_MOVEMENT_THRESHOLD) {
                enemy.stuckCounter++;
                
                if (mLogger) {
                    mLogger->warn("Enemy {} stuck check {}/6 (moved {:.2f} units in {:.1f}s)", 
                                  entt::to_integral(entity), 
                                  enemy.stuckCounter,
                                  distanceMoved,
                                  enemy.stuckCheckInterval);
                }
                
                // Execute recovery attempts starting at check 3
                if (enemy.stuckCounter >= 3) {
                    executeUnstuckBehavior(entity, enemy, transform);
                }
            } else {
                // Making progress - reset counter and log success
                if (enemy.stuckCounter > 0) {
                    if (mLogger) {
                        mLogger->info("Enemy {} recovered from stuck state after {} checks", 
                                      entt::to_integral(entity), 
                                      enemy.stuckCounter);
                    }
                    enemy.stuckCounter = 0;
                }
            }
            
            // Update tracking position
            enemy.lastCheckedPosition = transform.position;
            enemy.stuckCheckTimer = 0.0f;
        }
    });
}

bool EnemyAISystem::isPositionVisibleToPlayer(const engine::Vector2f& position) {
    // Query all entities with VisibilityComponent (typically just the player)
    auto visibilityView = mRegistry.view<VisibilityComponent, engine::TransformComponent>();
    
    // We need to capture result outside lambda
    bool isVisible = false;
    
    visibilityView.each([&](entt::entity entity, const VisibilityComponent& visibility, const engine::TransformComponent& transform) {
        if (isVisible) return; // Already found visible
        
        // Calculate position relative to viewer
        engine::Vector2f viewerPos = transform.position + visibility.offset;
        engine::Vector2f toTarget = position - viewerPos;
        float distSq = toTarget.x * toTarget.x + toTarget.y * toTarget.y;
        
        // Check if within view radius
        if (distSq > visibility.viewRadius * visibility.viewRadius) {
            return; // Too far, not visible
        }
        
        // Check if within minimum view radius (always visible)
        if (distSq < visibility.minViewRadius * visibility.minViewRadius) {
            isVisible = true; // Inside minimum radius, definitely visible
            return;
        }
        
        // Check if within view cone
        float angleToTarget = std::atan2(toTarget.y, toTarget.x);
        float halfFOV = (visibility.viewAngle * 3.14159f / 180.0f) / 2.0f;
        
        // Normalize angle difference to [-π, π]
        float angleDiff = angleToTarget - visibility.viewDirection;
        while (angleDiff > 3.14159f) angleDiff -= 2.0f * 3.14159f;
        while (angleDiff < -3.14159f) angleDiff += 2.0f * 3.14159f;
        
        if (std::abs(angleDiff) <= halfFOV) {
            // Within FOV cone - check line of sight
            if (hasLineOfSight(viewerPos, position)) {
                isVisible = true; // Fully visible
                return;
            }
        }
    });
    
    return isVisible;
}

engine::Vector2f EnemyAISystem::findSafeUnstuckPosition(entt::entity entity, 
                                                         const engine::Vector2f& preferredPos) {
    // First choice: Use preferred position if not visible
    if (!isPositionVisibleToPlayer(preferredPos)) {
        return preferredPos;
    }
    
    if (mLogger) {
        mLogger->debug("Enemy {} preferred unstuck position is visible, searching for alternative", 
                       entt::to_integral(entity));
    }
    
    // Second choice: Try waypoints (if entity has PatrolBehavior)
    if (auto* patrol = mRegistry.try_get<PatrolBehaviorComponent>(entity)) {
        for (const auto& waypoint : patrol->waypoints) {
            if (!isPositionVisibleToPlayer(waypoint)) {
                if (mLogger) {
                    mLogger->debug("Enemy {} found safe waypoint at ({:.1f}, {:.1f})", 
                                   entt::to_integral(entity), waypoint.x, waypoint.y);
                }
                return waypoint;
            }
        }
    }
    
    // Third choice: Sample random positions in a radius around preferred position
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<float> angleDist(0.0f, 6.28318f); // 0 to 2π
    std::uniform_real_distribution<float> radiusDist(100.0f, 300.0f);
    
    for (int attempt = 0; attempt < 8; ++attempt) {
        float angle = angleDist(gen);
        float radius = radiusDist(gen);
        
        engine::Vector2f candidate = {
            preferredPos.x + std::cos(angle) * radius,
            preferredPos.y + std::sin(angle) * radius
        };
        
        if (!isPositionVisibleToPlayer(candidate)) {
            if (mLogger) {
                mLogger->debug("Enemy {} found safe random position at ({:.1f}, {:.1f}) after {} attempts", 
                               entt::to_integral(entity), candidate.x, candidate.y, attempt + 1);
            }
            return candidate;
        }
    }
    
    // Last resort: Return preferred position anyway (better than nothing)
    if (mLogger) {
        mLogger->warn("Enemy {} could not find safe unstuck position, using preferred (visible)", 
                      entt::to_integral(entity));
    }
    return preferredPos;
}

void EnemyAISystem::executeUnstuckBehavior(entt::entity entity, EnemyComponent& enemy, 
                                            const engine::TransformComponent& transform) {
    auto* bodyComp = mRegistry.try_get<engine::PhysicsBodyComponent>(entity);
    if (!bodyComp || !b2Body_IsValid(bodyComp->bodyId)) {
        if (mLogger) {
            mLogger->error("Enemy {} has invalid physics body, cannot execute unstuck", 
                           entt::to_integral(entity));
        }
        enemy.currentState = EnemyState::Dead;
        return;
    }
    
    // ==================================================================
    // ATTEMPT 1: Random Impulse (Stuck Check #3 - after 6 seconds)
    // ==================================================================
    if (enemy.stuckCounter == 3) {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_real_distribution<float> angleDist(0.0f, 6.28318f);
        std::uniform_real_distribution<float> forceDist(100.0f, 200.0f);
        
        float angle = angleDist(gen);
        float force = forceDist(gen);
        b2Vec2 impulse = {std::cos(angle) * force, std::sin(angle) * force};
        
        b2Body_ApplyLinearImpulseToCenter(bodyComp->bodyId, impulse, true);
        
        if (mLogger) {
            mLogger->info("Enemy {} unstuck attempt 1/4: random impulse (angle={:.2f}°, force={:.1f})", 
                          entt::to_integral(entity), 
                          angle * 180.0f / 3.14159f, 
                          force);
        }
        return; // Don't reset counter, let it increment on next check
    }
    
    // ==================================================================
    // ATTEMPT 2: Teleport to Last Valid Position (Stuck Check #4 - after 8 seconds)
    // ==================================================================
    else if (enemy.stuckCounter == 4) {
        // First check if CURRENT enemy position is visible to player
        if (isPositionVisibleToPlayer(transform.position)) {
             if (mLogger) {
                mLogger->warn("Enemy {} is currently visible, skipping teleport to avoid popping", 
                              entt::to_integral(entity));
            }
            // If visible, we might skip teleport this frame or try a small invisible nudge?
            // For now, let's just skip to next attempt counter but NOT teleport
            // effectively delaying teleport until not visible or force kill
            
            // However, to follow the design philosophy: "Teleporting while the player is watching breaks immersion"
            // We should NOT teleport if visible.
            // But we can increment counter to eventually kill it if it stays stuck & visible forever.
            return; 
        }

        // Validate last valid position exists and isn't zero
        if (enemy.lastValidPosition.x != 0.0f || enemy.lastValidPosition.y != 0.0f) {
            // Find safe position (not in player's view)
            engine::Vector2f safePosition = findSafeUnstuckPosition(entity, enemy.lastValidPosition);
            
            // Teleport to safe position
            b2Body_SetTransform(bodyComp->bodyId, 
                                 {safePosition.x, safePosition.y}, 
                                 b2MakeRot(0.0f));
            b2Body_SetLinearVelocity(bodyComp->bodyId, {0.0f, 0.0f});
            b2Body_SetAngularVelocity(bodyComp->bodyId, 0.0f);
            b2Body_SetAwake(bodyComp->bodyId, true);
            
            if (mLogger) {
                bool wasVisible = isPositionVisibleToPlayer(enemy.lastValidPosition);
                mLogger->info("Enemy {} unstuck attempt 2/4: teleport to last valid position ({:.1f}, {:.1f}) [{}]", 
                              entt::to_integral(entity), 
                              safePosition.x, 
                              safePosition.y,
                              wasVisible ? "adjusted for visibility" : "safe");
            }
        } else {
            // No valid position stored, skip to next attempt
            if (mLogger) {
                mLogger->warn("Enemy {} has no valid lastValidPosition, skipping attempt 2", 
                              entt::to_integral(entity));
            }
            enemy.stuckCounter = 5; // Force skip to next attempt
        }
        return;
    }
    
    // ==================================================================
    // ATTEMPT 3: Teleport to Nearest Waypoint (Stuck Check #5 - after 10 seconds)
    // ==================================================================
    else if (enemy.stuckCounter == 5) {
        // First check if CURRENT enemy position is visible to player
        if (isPositionVisibleToPlayer(transform.position)) {
             if (mLogger) {
                mLogger->warn("Enemy {} is currently visible, skipping waypoint teleport", 
                              entt::to_integral(entity));
            }
            return; 
        }

        if (auto* patrol = mRegistry.try_get<PatrolBehaviorComponent>(entity)) {
            if (!patrol->waypoints.empty()) {
                // Find nearest waypoint
                engine::Vector2f nearestWaypoint = patrol->waypoints[0];
                float minDistSq = std::numeric_limits<float>::max();
                int nearestIndex = 0;
                
                for (size_t i = 0; i < patrol->waypoints.size(); ++i) {
                    const auto& wp = patrol->waypoints[i];
                    engine::Vector2f delta = wp - transform.position;
                    float distSq = delta.x * delta.x + delta.y * delta.y;
                    if (distSq < minDistSq) {
                        minDistSq = distSq;
                        nearestWaypoint = wp;
                        nearestIndex = i;
                    }
                }
                
                // Find safe position around nearest waypoint
                engine::Vector2f safePosition = findSafeUnstuckPosition(entity, nearestWaypoint);
                
                // Teleport to safe waypoint
                b2Body_SetTransform(bodyComp->bodyId, 
                                     {safePosition.x, safePosition.y}, 
                                     b2MakeRot(0.0f));
                b2Body_SetLinearVelocity(bodyComp->bodyId, {0.0f, 0.0f});
                b2Body_SetAngularVelocity(bodyComp->bodyId, 0.0f);
                b2Body_SetAwake(bodyComp->bodyId, true);
                
                // Reset patrol state
                enemy.currentState = EnemyState::Patrol;
                patrol->currentWaypointIndex = nearestIndex;
                patrol->reversing = false;
                
                if (mLogger) {
                    bool wasVisible = isPositionVisibleToPlayer(nearestWaypoint);
                    mLogger->info("Enemy {} unstuck attempt 3/4: teleport to waypoint #{} ({:.1f}, {:.1f}) [{}]", 
                                  entt::to_integral(entity), 
                                  nearestIndex,
                                  safePosition.x, 
                                  safePosition.y,
                                  wasVisible ? "adjusted for visibility" : "safe");
                }
                return;
            }
        }
        
        // No patrol component or waypoints - skip to death
        if (mLogger) {
            mLogger->warn("Enemy {} has no waypoints for attempt 3, skipping to attempt 4", 
                          entt::to_integral(entity));
        }
        enemy.stuckCounter = 6;
        return;
    }
    
    // ==================================================================
    // ATTEMPT 4: Last Resort (Stuck Check #6 - after 12 seconds)
    // ==================================================================
    else if (enemy.stuckCounter >= 6) {
        
        // If visible, DO NOT KILL. Try aggressive physics shove instead.
        if (isPositionVisibleToPlayer(transform.position)) {
             if (mLogger) {
                mLogger->warn("Enemy {} is visible, applying aggressive shove instead of kill", 
                              entt::to_integral(entity));
            }
            
            // Raycast in 4 directions to find clear path
            b2Vec2 directions[4] = {
                {1.0f, 0.0f},  // Right
                {-1.0f, 0.0f}, // Left
                {0.0f, 1.0f},  // Down
                {0.0f, -1.0f}  // Up
            };
            
            b2Vec2 bestDir = {0.0f, 0.0f};
            float maxDist = -1.0f;
            
            b2QueryFilter filter = b2DefaultQueryFilter();
            filter.maskBits = engine::PhysicsCategory::Wall | engine::PhysicsCategory::LowObstacle; // Check walls and low obstacles
            
            for(int i=0; i<4; ++i) {
                 b2Vec2 origin = {transform.position.x, transform.position.y};
                 b2Vec2 translation = {directions[i].x * 100.0f, directions[i].y * 100.0f}; // 100 units check
                 
                 b2RayResult result = b2World_CastRayClosest(mWorldId, origin, translation, filter);
                 
                 float dist = 100.0f;
                 if(result.hit) {
                     // b2RayResult usually has fraction. 
                     // If b2World_GetPoint failed, we rely on fraction.
                     dist = result.fraction * 100.0f;
                 }
                 
                 if(dist > maxDist) {
                     maxDist = dist;
                     bestDir = directions[i];
                 }
            }
            
            // Apply STRONG impulse in best direction
            if (maxDist > 10.0f) {
                 float shoveForce = 1000.0f; // Very strong shove
                 b2Vec2 impulse = {bestDir.x * shoveForce, bestDir.y * shoveForce};
                 b2Body_ApplyLinearImpulseToCenter(bodyComp->bodyId, impulse, true);
                 
                 // Disable movement controller for 0.5s to let physics take effect
                 enemy.stunTimer = 0.5f;
                 
                  if (mLogger) {
                    mLogger->info("Enemy {} shoved with force {:.1f} in dir ({:.1f}, {:.1f})", 
                                  entt::to_integral(entity), shoveForce, bestDir.x, bestDir.y);
                }
            }
            
            // Don't reset counter so we keep trying this or eventually kill if he becomes invisible
            return;
        }

        if (mLogger) {
            mLogger->error("Enemy {} PERMANENTLY STUCK after all recovery attempts. Marking as Dead.", 
                           entt::to_integral(entity));
        }
        
        // Mark as dead
        enemy.currentState = EnemyState::Dead;
        
        if (auto* health = mRegistry.try_get<HealthComponent>(entity)) {
            health->currentHp = 0.0f;
        }
        
        // Optional: Add lifetime component for gradual fadeout
        if (!mRegistry.any_of<engine::LifetimeComponent>(entity)) {
            mRegistry.emplace<engine::LifetimeComponent>(entity, 3.0f); // Despawn after 3s
        }
        
        // Disable physics collision
        b2Body_SetLinearVelocity(bodyComp->bodyId, {0.0f, 0.0f});
        b2Body_Disable(bodyComp->bodyId);
    }
}

bool EnemyAISystem::hasLineOfSight(const engine::Vector2f& start, const engine::Vector2f& end) {
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
        
        if (distSq < alertRadiusSq) {
            enemy.currentState = EnemyState::Alert;
            enemy.lastKnownPosition = position;
            enemy.isAlerted = true;
            alertCount++;
        }
    });
    
    if (mLogger && alertCount > 0) {
        mLogger->info("Alert complete: {} enemies notified", alertCount);
    }
}


} // namespace game
