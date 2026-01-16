#include "engine/pch.h"
#include "EnemyMovementSystem.hpp"
#include "game/components/EnemyComponent.hpp"
#include "game/components/PatrolBehaviorComponent.hpp"
#include "game/components/RushBehaviorComponent.hpp"
#include "engine/components/TransformComponent.hpp"
#include "engine/components/PhysicsBodyComponent.hpp"
#include "engine/core/ILoggerManager.hpp"
#include "engine/physics/PhysicsConstants.hpp"
#include <box2d/box2d.h>
#include <cmath>

namespace game::ai {

    EnemyMovementSystem::EnemyMovementSystem(engine::EngineContext& context)
        : mContext(context)
    {
        if (context.m_logManager) {
            mLogger = context.m_logManager->GetLogger("EnemyMovement");
        }
    }

    void EnemyMovementSystem::update(float dt) {
        auto view = mContext.m_registry->view<EnemyComponent, engine::TransformComponent, engine::PhysicsBodyComponent>();
        
        view.each([&](entt::entity entity, EnemyComponent& enemy, const engine::TransformComponent& transform, engine::PhysicsBodyComponent& bodyComp) {
            
            if (!b2Body_IsValid(bodyComp.bodyId)) return;
            
            if (enemy.stunTimer > 0.0f) {
                enemy.stunTimer -= dt;
                if (enemy.stunTimer > 0.0f) {
                    return; 
                }
            }
            
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
                    desiredVelocity = {0.0f, 0.0f};
                    break;
                default:
                    desiredVelocity = {0.0f, 0.0f};
                    break;
            }
            
            engine::Vector2f finalVelocity = applyWallSliding(transform.position, desiredVelocity, dt);
            
            if (enemy.stuckCounter > 0) {
                 float nudgeSpeed = enemy.moveSpeed * 0.5f;
                 
                 float refVelX = (std::abs(finalVelocity.x) > 0.01f || std::abs(finalVelocity.y) > 0.01f) ? finalVelocity.x : desiredVelocity.x;
                 float refVelY = (std::abs(finalVelocity.x) > 0.01f || std::abs(finalVelocity.y) > 0.01f) ? finalVelocity.y : desiredVelocity.y;
                 
                 engine::Vector2f nudgeDir = {-refVelY, refVelX};
                 float nudgeMag = std::sqrt(nudgeDir.x * nudgeDir.x + nudgeDir.y * nudgeDir.y);
                 
                 if (nudgeMag > 0.01f) {
                     nudgeDir = {nudgeDir.x / nudgeMag, nudgeDir.y / nudgeMag};
                     
                     finalVelocity.x += nudgeDir.x * nudgeSpeed;
                     finalVelocity.y += nudgeDir.y * nudgeSpeed;
                 }
            }
            
            b2Body_SetLinearVelocity(bodyComp.bodyId, {finalVelocity.x, finalVelocity.y});
            
            float velMagSq = finalVelocity.x * finalVelocity.x + finalVelocity.y * finalVelocity.y;
            if (velMagSq > 1.0f) { 
                enemy.lastValidPosition = transform.position;
            } else {
                 if (enemy.lastValidPosition.x == 0.0f && enemy.lastValidPosition.y == 0.0f) {
                     enemy.lastValidPosition = transform.position;
                 }
            }
        });
    }

    engine::Vector2f EnemyMovementSystem::calculatePatrolMovement(entt::entity entity, EnemyComponent& enemy, const engine::TransformComponent& transform) {
        auto* patrol = mContext.m_registry->try_get<PatrolBehaviorComponent>(entity);
        if (!patrol || patrol->waypoints.empty()) {
            return {0.0f, 0.0f};
        }
        
        if (patrol->currentWaypointIndex >= patrol->waypoints.size()) {
            patrol->currentWaypointIndex = 0;
        }
        
        engine::Vector2f targetWaypoint = patrol->waypoints[patrol->currentWaypointIndex];
        engine::Vector2f toWaypoint = {
            targetWaypoint.x - transform.position.x,
            targetWaypoint.y - transform.position.y
        };
        float distSq = toWaypoint.x * toWaypoint.x + toWaypoint.y * toWaypoint.y;
        
        if (distSq < patrol->waypointReachThreshold * patrol->waypointReachThreshold) {
            if (patrol->loopMode) {
                patrol->currentWaypointIndex = (patrol->currentWaypointIndex + 1) % patrol->waypoints.size();
            } else {
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
            
            targetWaypoint = patrol->waypoints[patrol->currentWaypointIndex];
            toWaypoint = {
                targetWaypoint.x - transform.position.x,
                targetWaypoint.y - transform.position.y
            };
            distSq = toWaypoint.x * toWaypoint.x + toWaypoint.y * toWaypoint.y;
        }
        
        if (distSq > 0.01f) {
            float dist = std::sqrt(distSq);
            return {
                (toWaypoint.x / dist) * enemy.moveSpeed,
                (toWaypoint.y / dist) * enemy.moveSpeed
            };
        }
        
        return {0.0f, 0.0f};
    }

    engine::Vector2f EnemyMovementSystem::calculateChaseMovement(entt::entity entity, EnemyComponent& enemy, const engine::TransformComponent& transform) {
        if (enemy.targetEntity == entt::null || !mContext.m_registry->valid(enemy.targetEntity)) {
            return {0.0f, 0.0f};
        }
        
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
        
        if (distSq > 0.01f) {
            float dist = std::sqrt(distSq);
            return {
                (toTarget.x / dist) * enemy.moveSpeed,
                (toTarget.y / dist) * enemy.moveSpeed
            };
        }
        
        return {0.0f, 0.0f};
    }

    engine::Vector2f EnemyMovementSystem::calculateRushMovement(entt::entity entity, EnemyComponent& enemy, const engine::TransformComponent& transform) {
        auto* rush = mContext.m_registry->try_get<RushBehaviorComponent>(entity);
        if (!rush) {
            return calculateChaseMovement(entity, enemy, transform);
        }
        
        if (enemy.targetEntity == entt::null || !mContext.m_registry->valid(enemy.targetEntity)) {
            return {0.0f, 0.0f};
        }
        
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

    engine::Vector2f EnemyMovementSystem::calculateRetreatMovement(entt::entity entity, EnemyComponent& enemy, const engine::TransformComponent& transform) {
        if (enemy.targetEntity == entt::null || !mContext.m_registry->valid(enemy.targetEntity)) {
            engine::Vector2f safePos = enemy.lastValidPosition;
            if (safePos.x == 0.0f && safePos.y == 0.0f) {
                safePos = transform.position; 
            }

            engine::Vector2f toSafe = {
                safePos.x - transform.position.x,
                safePos.y - transform.position.y
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
        
        engine::Vector2f playerPos = mContext.m_registry->get<engine::TransformComponent>(enemy.targetEntity).position;
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

    engine::Vector2f EnemyMovementSystem::applyWallSliding(const engine::Vector2f& position, const engine::Vector2f& desiredVelocity, float dt) {
        float velocityMag = std::sqrt(desiredVelocity.x * desiredVelocity.x + desiredVelocity.y * desiredVelocity.y);
        if (velocityMag < 0.01f) {
            return desiredVelocity;
        }
        
        engine::Vector2f direction = {
            desiredVelocity.x / velocityMag,
            desiredVelocity.y / velocityMag
        };
        
        float rayLength = velocityMag * dt * 2.5f; 
        
        b2QueryFilter filter = b2DefaultQueryFilter();
        filter.maskBits = engine::PhysicsCategory::Wall | engine::PhysicsCategory::VisibilityBlocker | engine::PhysicsCategory::LowObstacle;
        
        float radiusBuffer = 16.0f; 
        b2Vec2 origin = {
            position.x + direction.x * radiusBuffer, 
            position.y + direction.y * radiusBuffer
        };
        b2Vec2 translation = {direction.x * rayLength, direction.y * rayLength};
        
        b2RayResult result = b2World_CastRayClosest(mContext.m_physicsWorld, origin, translation, filter);
        
        if (!result.hit) {
            return desiredVelocity;
        }
        
        engine::Vector2f hitNormal = {result.normal.x, result.normal.y};
        
        float dotProduct = direction.x * hitNormal.x + direction.y * hitNormal.y;
        
        if (dotProduct > -0.01f) { 
            return desiredVelocity; 
        }
        
        engine::Vector2f slideDirection = {
            direction.x - hitNormal.x * dotProduct,
            direction.y - hitNormal.y * dotProduct
        };
        
        float slideMag = std::sqrt(slideDirection.x * slideDirection.x + slideDirection.y * slideDirection.y);
        if (slideMag > 0.01f) {
            float boostFactor = 1.1f;
            return {
                (slideDirection.x / slideMag) * velocityMag * boostFactor,
                (slideDirection.y / slideMag) * velocityMag * boostFactor
            };
        }
        
        return {0.0f, 0.0f};
    }

}
