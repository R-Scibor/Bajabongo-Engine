#include "engine/pch.h"
#include "EnemyMovementSystem.hpp"
#include "game/components/EnemyComponent.hpp"
#include "game/components/PatrolBehaviorComponent.hpp"
#include "game/components/RushBehaviorComponent.hpp"
#include "engine/events/AudioEvents.hpp"
#include "engine/components/TransformComponent.hpp"
#include "engine/components/PhysicsBodyComponent.hpp"
#include "engine/components/AnimationStateComponent.hpp"
#include "engine/core/ILoggerManager.hpp"
#include "engine/physics/PhysicsConstants.hpp"
#include <box2d/box2d.h>
#include <cmath>
#include <cstdlib>


namespace game::ai {


    EnemyMovementSystem::EnemyMovementSystem(engine::EngineContext& context)
        : mContext(context)
    {
        if (context.m_logManager) {
            mLogger = context.m_logManager->GetLogger("EnemyMovement");
        }
    }


    void EnemyMovementSystem::update(float dt) {
        // Iterate over all enemies with Transform and PhysicsBody
        auto view = mContext.m_registry->view<EnemyComponent, engine::TransformComponent, engine::PhysicsBodyComponent>();
        
        view.each([&](entt::entity entity, EnemyComponent& enemy, const engine::TransformComponent& transform, engine::PhysicsBodyComponent& bodyComp) {
            
            // Skip if physics body is invalid (shouldn't happen, but defensive)
            if (!b2Body_IsValid(bodyComp.bodyId)) return;
            
            // Stun timer blocks all movement — enemy is frozen
            if (enemy.stunTimer > 0.0f) {
                enemy.stunTimer -= dt;
                if (enemy.stunTimer > 0.0f) {
                    return; // Still stunned, bail
                }
            }
            
            // Calculate desired velocity based on current AI state
            engine::Vector2f desiredVelocity{0.0f, 0.0f};
            
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
                    desiredVelocity = {0.0f, 0.0f}; // Stationary when shooting
                    break;
                default:
                    desiredVelocity = {0.0f, 0.0f};
                    break;
            }
            
            // Apply wall sliding: raycast ahead and slide along obstacles
            engine::Vector2f finalVelocity = applyWallSliding(transform.position, desiredVelocity, dt);
            
            // Stuck recovery nudge: if enemy is stuck, apply perpendicular force
            if (enemy.stuckCounter > 0) {
                 float nudgeSpeed = enemy.moveSpeed * 0.5f;
                 
                 // Pick a reference velocity (prefer finalVelocity if non-zero, else desiredVelocity)
                 float refVelX = (std::abs(finalVelocity.x) > 0.01f || std::abs(finalVelocity.y) > 0.01f) ? finalVelocity.x : desiredVelocity.x;
                 float refVelY = (std::abs(finalVelocity.x) > 0.01f || std::abs(finalVelocity.y) > 0.01f) ? finalVelocity.y : desiredVelocity.y;
                 
                 // Perpendicular vector: rotate 90 degrees (swap x/y, negate one)
                 engine::Vector2f nudgeDir = {-refVelY, refVelX};
                 float nudgeMag = std::sqrt(nudgeDir.x * nudgeDir.x + nudgeDir.y * nudgeDir.y);
                 
                 if (nudgeMag > 0.01f) {
                     nudgeDir = {nudgeDir.x / nudgeMag, nudgeDir.y / nudgeMag}; // Normalize
                     
                     // Apply nudge to final velocity
                     finalVelocity.x += nudgeDir.x * nudgeSpeed;
                     finalVelocity.y += nudgeDir.y * nudgeSpeed;
                 }
            }
            
            // Write final velocity to Box2D body (Box2D 3.0 C API: b2Body_SetLinearVelocity)
            b2Body_SetLinearVelocity(bodyComp.bodyId, {finalVelocity.x, finalVelocity.y});
            
            // --- Animation State Sync ---
            // Update AnimationStateComponent to drive sprite playback (handled by AnimationSystem)
            if (auto* animState = mContext.m_registry->try_get<engine::AnimationStateComponent>(entity)) {
                float velMagSq = finalVelocity.x * finalVelocity.x + finalVelocity.y * finalVelocity.y;
                
                // Update animation state: Walk if moving fast enough, else Idle
                // Threshold: 10.0f (arbitrary, adjust to taste)
                if (velMagSq > 10.0f) {
                    animState->state = engine::AnimationState::Walk;
                } else {
                    animState->state = engine::AnimationState::Idle;
                }
                
                // Update facing direction based on horizontal velocity
                // Only update if there's significant movement to avoid jitter
                if (finalVelocity.x < -0.1f) {
                    animState->facing = engine::FacingDirection::Left;
                } else if (finalVelocity.x > 0.1f) {
                    animState->facing = engine::FacingDirection::Right;
                }
            }

            // Cache velocity magnitude squared for footstep and lastValidPosition logic
            float velMagSq = finalVelocity.x * finalVelocity.x + finalVelocity.y * finalVelocity.y;
            
            // --- Footstep Audio ---
            // Play footstep sounds when enemy is moving (cheap positional audio)
            if (velMagSq > 10.0f) {
                enemy.stepTimer -= dt;
                if (enemy.stepTimer <= 0.0f) {
                    enemy.stepTimer = enemy.stepInterval; // Reset timer
                    
                    // Randomize footstep sound variant (st1, st2, st3)
                    int r = std::rand() % 3;
                    std::string soundId;
                    if (r == 0) soundId = "st1-footstep";
                    else if (r == 1) soundId = "st2-footstep";
                    else soundId = "st3-footstep-sfx";

                    // Vary pitch slightly for variety (85%–105%)
                    float pitch = 0.85f + (static_cast<float>(std::rand() % 20) / 100.0f);

                    // Enqueue positional audio event with lower volume than player
                    mContext.m_dispatcher->enqueue<engine::PlaySoundEvent>({
                        .id = soundId,
                        .volume = 20.0f, // Quieter than player footsteps (usually ~50)
                        .pitch = pitch,
                        .loop = false,
                        .sourceEntity = entity,
                        .position = transform.position,
                        .minDistance = 300.0f,
                        .attenuation = 2.0f
                    });
                }
            } else {
                // Reset timer when not moving to avoid instant step on next move
                enemy.stepTimer = 0.1f;
            }

            // --- Last Valid Position Cache (for Retreat/Unstuck) ---
            // If enemy is moving, cache current position as "safe position"
            // Used by Retreat state and StuckRecovery teleport logic
            if (velMagSq > 1.0f) {
                enemy.lastValidPosition = transform.position;
            } else {
                 // If lastValidPosition is uninitialized (0,0), set it to current pos
                 if (enemy.lastValidPosition.x == 0.0f && enemy.lastValidPosition.y == 0.0f) {
                     enemy.lastValidPosition = transform.position;
                 }
            }
        });
    }


    // --- PATROL MOVEMENT ---
    // Move along predefined waypoints (looping or ping-pong)
    engine::Vector2f EnemyMovementSystem::calculatePatrolMovement(entt::entity entity, EnemyComponent& enemy, const engine::TransformComponent& transform) {
        auto* patrol = mContext.m_registry->try_get<PatrolBehaviorComponent>(entity);
        if (!patrol || patrol->waypoints.empty()) {
            return {0.0f, 0.0f}; // No patrol behavior, no movement
        }
        
        // Bounds check (shouldn't happen, but defensive)
        if (patrol->currentWaypointIndex >= patrol->waypoints.size()) {
            patrol->currentWaypointIndex = 0;
        }
        
        // Get target waypoint
        engine::Vector2f targetWaypoint = patrol->waypoints[patrol->currentWaypointIndex];
        engine::Vector2f toWaypoint = {
            targetWaypoint.x - transform.position.x,
            targetWaypoint.y - transform.position.y
        };
        float distSq = toWaypoint.x * toWaypoint.x + toWaypoint.y * toWaypoint.y;
        
        // Check if reached current waypoint
        if (distSq < patrol->waypointReachThreshold * patrol->waypointReachThreshold) {
            // Advance to next waypoint based on mode
            if (patrol->loopMode) {
                // Loop mode: 0 -> 1 -> 2 -> 0 -> ...
                patrol->currentWaypointIndex = (patrol->currentWaypointIndex + 1) % patrol->waypoints.size();
            } else {
                // Ping-pong mode: 0 -> 1 -> 2 -> 1 -> 0 -> ...
                if (patrol->reversing) {
                    if (patrol->currentWaypointIndex > 0) {
                        patrol->currentWaypointIndex--;
                    }
                    if (patrol->currentWaypointIndex == 0) {
                        patrol->reversing = false; // Reached start, switch direction
                    }
                } else {
                    patrol->currentWaypointIndex++;
                    if (patrol->currentWaypointIndex >= static_cast<int>(patrol->waypoints.size()) - 1) {
                        patrol->currentWaypointIndex = static_cast<int>(patrol->waypoints.size()) - 1;
                        patrol->reversing = true; // Reached end, switch direction
                    }
                }
            }
            
            // Recompute vector to next waypoint
            targetWaypoint = patrol->waypoints[patrol->currentWaypointIndex];
            toWaypoint = {
                targetWaypoint.x - transform.position.x,
                targetWaypoint.y - transform.position.y
            };
            distSq = toWaypoint.x * toWaypoint.x + toWaypoint.y * toWaypoint.y;
        }
        
        // Normalize and scale by moveSpeed
        if (distSq > 0.01f) {
            float dist = std::sqrt(distSq);
            return {
                (toWaypoint.x / dist) * enemy.moveSpeed,
                (toWaypoint.y / dist) * enemy.moveSpeed
            };
        }
        
        return {0.0f, 0.0f};
    }


    // --- CHASE/APPROACH MOVEMENT ---
    // Move toward player's last known position (or real-time if LOS)
    engine::Vector2f EnemyMovementSystem::calculateChaseMovement(entt::entity entity, EnemyComponent& enemy, const engine::TransformComponent& transform) {
        if (enemy.targetEntity == entt::null || !mContext.m_registry->valid(enemy.targetEntity)) {
            return {0.0f, 0.0f}; // No valid target, no movement
        }
        
        // If we have line of sight in Chase state, track player directly
        // Otherwise (Approach state or lost LOS), move to last known position
        engine::Vector2f targetPos;
        if (enemy.currentState == EnemyState::Chase && enemy.hasLineOfSight) {
            targetPos = mContext.m_registry->get<engine::TransformComponent>(enemy.targetEntity).position;
        } else {
            targetPos = enemy.lastKnownPosition;
        }
        
        engine::Vector2f toTarget = {
            targetPos.x - transform.position.x,
            targetPos.y - transform.position.y
        };
        float distSq = toTarget.x * toTarget.x + toTarget.y * toTarget.y;
        
        // Normalize and scale by moveSpeed
        if (distSq > 0.01f) {
            float dist = std::sqrt(distSq);
            return {
                (toTarget.x / dist) * enemy.moveSpeed,
                (toTarget.y / dist) * enemy.moveSpeed
            };
        }
        
        return {0.0f, 0.0f};
    }


    // --- RUSH MOVEMENT ---
    // Fast charge toward player (uses rushSpeed > moveSpeed)
    engine::Vector2f EnemyMovementSystem::calculateRushMovement(entt::entity entity, EnemyComponent& enemy, const engine::TransformComponent& transform) {
        auto* rush = mContext.m_registry->try_get<RushBehaviorComponent>(entity);
        if (!rush) {
            // Fallback to normal chase if RushBehaviorComponent is missing
            return calculateChaseMovement(entity, enemy, transform);
        }
        
        if (enemy.targetEntity == entt::null || !mContext.m_registry->valid(enemy.targetEntity)) {
            return {0.0f, 0.0f};
        }
        
        // Rush directly toward player's current position (real-time, no memory)
        engine::Vector2f targetPos = mContext.m_registry->get<engine::TransformComponent>(enemy.targetEntity).position;
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


    // --- RETREAT MOVEMENT ---
    // Move away from player (or back to lastValidPosition if no target)
    engine::Vector2f EnemyMovementSystem::calculateRetreatMovement(entt::entity entity, EnemyComponent& enemy, const engine::TransformComponent& transform) {
        // If no valid target, retreat to cached safe position
        if (enemy.targetEntity == entt::null || !mContext.m_registry->valid(enemy.targetEntity)) {
            engine::Vector2f safePos = enemy.lastValidPosition;
            if (safePos.x == 0.0f && safePos.y == 0.0f) {
                safePos = transform.position; // Fallback: stay in place if no safe pos
            }

            engine::Vector2f toSafe = {
                safePos.x - transform.position.x,
                safePos.y - transform.position.y
            };
            float distSq = toSafe.x * toSafe.x + toSafe.y * toSafe.y;
            if (distSq > 0.01f) {
                float dist = std::sqrt(distSq);
                // Move faster (1.2x) when retreating to safe pos
                return {
                    (toSafe.x / dist) * enemy.moveSpeed * 1.2f,
                    (toSafe.y / dist) * enemy.moveSpeed * 1.2f
                };
            }
            return {0.0f, 0.0f};
        }
        
        // Otherwise, retreat directly away from player
        engine::Vector2f playerPos = mContext.m_registry->get<engine::TransformComponent>(enemy.targetEntity).position;
        engine::Vector2f awayFromPlayer = {
            transform.position.x - playerPos.x,
            transform.position.y - playerPos.y
        };
        float distSq = awayFromPlayer.x * awayFromPlayer.x + awayFromPlayer.y * awayFromPlayer.y;
        
        if (distSq > 0.01f) {
            float dist = std::sqrt(distSq);
            // 1.2x speed boost when retreating
            return {
                (awayFromPlayer.x / dist) * enemy.moveSpeed * 1.2f,
                (awayFromPlayer.y / dist) * enemy.moveSpeed * 1.2f
            };
        }
        
        return {0.0f, 0.0f};
    }


    // --- WALL SLIDING ---
    // Raycast ahead to detect walls/obstacles and slide along them smoothly
    engine::Vector2f EnemyMovementSystem::applyWallSliding(const engine::Vector2f& position, const engine::Vector2f& desiredVelocity, float dt) {
        float velocityMag = std::sqrt(desiredVelocity.x * desiredVelocity.x + desiredVelocity.y * desiredVelocity.y);
        if (velocityMag < 0.01f) {
            return desiredVelocity; // Not moving, no sliding needed
        }
        
        // Normalize desired direction
        engine::Vector2f direction = {
            desiredVelocity.x / velocityMag,
            desiredVelocity.y / velocityMag
        };
        
        // Raycast ahead: 2.5x the per-frame distance to anticipate collision
        float rayLength = velocityMag * dt * 2.5f; 
        
        // Box2D query filter: check Wall, VisibilityBlocker, LowObstacle
        b2QueryFilter filter = b2DefaultQueryFilter();
        filter.maskBits = engine::PhysicsCategory::Wall | engine::PhysicsCategory::VisibilityBlocker | engine::PhysicsCategory::LowObstacle;
        
        // Offset ray origin slightly forward to avoid self-collision with enemy's own body
        float radiusBuffer = 16.0f; 
        b2Vec2 origin = {
            position.x + direction.x * radiusBuffer, 
            position.y + direction.y * radiusBuffer
        };
        b2Vec2 translation = {direction.x * rayLength, direction.y * rayLength};
        
        // Cast ray (Box2D 3.0 C API)
        b2RayResult result = b2World_CastRayClosest(mContext.m_physicsWorld, origin, translation, filter);
        
        if (!result.hit) {
            return desiredVelocity; // No wall ahead, move freely
        }
        
        // Wall detected: compute slide vector
        engine::Vector2f hitNormal = {result.normal.x, result.normal.y};
        
        // Dot product: how much are we moving "into" the wall?
        float dotProduct = direction.x * hitNormal.x + direction.y * hitNormal.y;
        
        // If moving away from wall (dotProduct > 0), no need to slide
        if (dotProduct > -0.01f) { 
            return desiredVelocity; // Moving away or parallel, no correction needed
        }
        
        // Compute slide direction: remove the component of velocity that's into the wall
        // Formula: slide = direction - normal * dot(direction, normal)
        engine::Vector2f slideDirection = {
            direction.x - hitNormal.x * dotProduct,
            direction.y - hitNormal.y * dotProduct
        };
        
        // Normalize slide direction and scale by original velocity magnitude
        float slideMag = std::sqrt(slideDirection.x * slideDirection.x + slideDirection.y * slideDirection.y);
        if (slideMag > 0.01f) {
            // Boost factor: 1.1x to make sliding feel smoother (helps enemies "hug" walls)
            float boostFactor = 1.1f;
            return {
                (slideDirection.x / slideMag) * velocityMag * boostFactor,
                (slideDirection.y / slideMag) * velocityMag * boostFactor
            };
        }
        
        // Slide direction is zero (hitting wall head-on), stop completely
        return {0.0f, 0.0f};
    }


}
