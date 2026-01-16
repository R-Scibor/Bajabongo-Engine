#include "engine/pch.h"
#include "EnemyStuckRecoverySystem.hpp"
#include "AIUtils.hpp"
#include "game/components/EnemyComponent.hpp"
#include "game/components/HealthComponent.hpp"
#include "game/components/PatrolBehaviorComponent.hpp"
#include "engine/components/TransformComponent.hpp"
#include "engine/components/PhysicsBodyComponent.hpp"
#include "engine/components/LifetimeComponent.hpp"
#include "engine/core/ILoggerManager.hpp"
#include "engine/physics/PhysicsConstants.hpp"
#include <box2d/box2d.h>
#include <cmath>
#include <random>

namespace game::ai {

    EnemyStuckRecoverySystem::EnemyStuckRecoverySystem(engine::EngineContext& context)
        : mContext(context)
    {
        if (context.m_logManager) {
            mLogger = context.m_logManager->GetLogger("EnemyStuck");
        }
    }

    void EnemyStuckRecoverySystem::update(float dt) {
        auto view = mContext.m_registry->view<EnemyComponent, engine::TransformComponent>();
        
        view.each([&](entt::entity entity, EnemyComponent& enemy, const engine::TransformComponent& transform) {
            if (enemy.currentState == EnemyState::Idle || 
                enemy.currentState == EnemyState::Dead ||
                enemy.currentState == EnemyState::Shoot ||
                enemy.currentState == EnemyState::Turret) {
                
                if (enemy.stuckCounter > 0) {
                    enemy.stuckCounter = 0;
                    enemy.lastCheckedPosition = transform.position; 
                    enemy.stuckCheckTimer = 0.0f;
                    if (mLogger) {
                        mLogger->debug("Enemy {} stopped moving, reset stuck counter", 
                                       entt::to_integral(entity));
                    }
                }
                return; 
            }
            
            enemy.stuckCheckTimer += dt;
            
            if (enemy.stuckCheckTimer >= enemy.stuckCheckInterval) {
                engine::Vector2f delta = transform.position - enemy.lastCheckedPosition;
                float distanceMovedSq = delta.x * delta.x + delta.y * delta.y;
                float distanceMoved = std::sqrt(distanceMovedSq);
                
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
                    
                    if (enemy.stuckCounter >= 3) {
                        executeUnstuckBehavior(entity, enemy, transform);
                    }
                } else {
                    if (enemy.stuckCounter > 0) {
                        if (mLogger) {
                            mLogger->info("Enemy {} recovered from stuck state after {} checks", 
                                          entt::to_integral(entity), 
                                          enemy.stuckCounter);
                        }
                        enemy.stuckCounter = 0;
                    }
                }
                
                enemy.lastCheckedPosition = transform.position;
                enemy.stuckCheckTimer = 0.0f;
            }
        });
    }

    engine::Vector2f EnemyStuckRecoverySystem::findSafeUnstuckPosition(entt::entity entity, const engine::Vector2f& preferredPos) {
        if (!AIUtils::isPositionVisibleToPlayer(mContext, preferredPos)) {
            return preferredPos;
        }
        
        if (mLogger) {
            mLogger->debug("Enemy {} preferred unstuck position is visible, searching for alternative", 
                           entt::to_integral(entity));
        }
        
        if (auto* patrol = mContext.m_registry->try_get<PatrolBehaviorComponent>(entity)) {
            for (const auto& waypoint : patrol->waypoints) {
                if (!AIUtils::isPositionVisibleToPlayer(mContext, waypoint)) {
                    if (mLogger) {
                        mLogger->debug("Enemy {} found safe waypoint at ({:.1f}, {:.1f})", 
                                       entt::to_integral(entity), waypoint.x, waypoint.y);
                    }
                    return waypoint;
                }
            }
        }
        
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_real_distribution<float> angleDist(0.0f, 6.28318f); 
        std::uniform_real_distribution<float> radiusDist(100.0f, 300.0f);
        
        for (int attempt = 0; attempt < 8; ++attempt) {
            float angle = angleDist(gen);
            float radius = radiusDist(gen);
            
            engine::Vector2f candidate = {
                preferredPos.x + std::cos(angle) * radius,
                preferredPos.y + std::sin(angle) * radius
            };
            
            if (!AIUtils::isPositionVisibleToPlayer(mContext, candidate)) {
                if (mLogger) {
                    mLogger->debug("Enemy {} found safe random position at ({:.1f}, {:.1f}) after {} attempts", 
                                   entt::to_integral(entity), candidate.x, candidate.y, attempt + 1);
                }
                return candidate;
            }
        }
        
        if (mLogger) {
            mLogger->warn("Enemy {} could not find safe unstuck position, using preferred (visible)", 
                          entt::to_integral(entity));
        }
        return preferredPos;
    }

    void EnemyStuckRecoverySystem::executeUnstuckBehavior(entt::entity entity, EnemyComponent& enemy, const engine::TransformComponent& transform) {
        auto* bodyComp = mContext.m_registry->try_get<engine::PhysicsBodyComponent>(entity);
        if (!bodyComp || !b2Body_IsValid(bodyComp->bodyId)) {
            if (mLogger) {
                mLogger->error("Enemy {} has invalid physics body, cannot execute unstuck", 
                               entt::to_integral(entity));
            }
            enemy.currentState = EnemyState::Dead;
            return;
        }
        
        if (enemy.stuckCounter == 3) {
            static std::random_device rd;
            static std::mt19937 gen(rd());
            std::uniform_real_distribution<float> angleDist(0.0f, 6.28318f);
            std::uniform_real_distribution<float> forceDist(100.0f, 200.0f);
            
            float angle = angleDist(gen);
            float force = forceDist(gen);
            b2Vec2 impulse = {std::cos(angle) * force, std::sin(angle) * force};
            
            b2Body_ApplyLinearImpulseToCenter(bodyComp->bodyId, impulse, true);
            
            enemy.stunTimer = 0.2f;

            if (mLogger) {
                mLogger->info("Enemy {} unstuck attempt 1/4: random impulse (angle={:.2f}deg, force={:.1f})", 
                              entt::to_integral(entity), 
                              angle * 180.0f / 3.14159f, 
                              force);
            }
            return; 
        }
        
        else if (enemy.stuckCounter == 4) {
            if (AIUtils::isPositionVisibleToPlayer(mContext, transform.position)) {
                 if (mLogger) {
                    mLogger->warn("Enemy {} is currently visible, skipping teleport to avoid popping", 
                                  entt::to_integral(entity));
                }
                return; 
            }

            if (enemy.lastValidPosition.x != 0.0f || enemy.lastValidPosition.y != 0.0f) {
                engine::Vector2f safePosition = findSafeUnstuckPosition(entity, enemy.lastValidPosition);
                
                b2Body_SetTransform(bodyComp->bodyId, 
                                     {safePosition.x, safePosition.y}, 
                                     b2MakeRot(0.0f));
                b2Body_SetLinearVelocity(bodyComp->bodyId, {0.0f, 0.0f});
                b2Body_SetAngularVelocity(bodyComp->bodyId, 0.0f);
                b2Body_SetAwake(bodyComp->bodyId, true);

                enemy.lastCheckedPosition = safePosition;
                enemy.stuckCheckTimer = 0.0f;
                enemy.stuckCounter = 0; 
                
                if (mLogger) {
                    bool wasVisible = AIUtils::isPositionVisibleToPlayer(mContext, enemy.lastValidPosition);
                    mLogger->info("Enemy {} unstuck attempt 2/4: teleport to last valid position ({:.1f}, {:.1f}) [{}]", 
                                  entt::to_integral(entity), 
                                  safePosition.x, 
                                  safePosition.y,
                                  wasVisible ? "adjusted for visibility" : "safe");
                }
            } else {
                if (mLogger) {
                    mLogger->warn("Enemy {} has no valid lastValidPosition, skipping attempt 2", 
                                  entt::to_integral(entity));
                }
                enemy.stuckCounter = 5; 
            }
            return;
        }
        
        else if (enemy.stuckCounter == 5) {
            if (AIUtils::isPositionVisibleToPlayer(mContext, transform.position)) {
                 if (mLogger) {
                    mLogger->warn("Enemy {} is currently visible, skipping waypoint teleport", 
                                  entt::to_integral(entity));
                }
                return; 
            }

            if (auto* patrol = mContext.m_registry->try_get<PatrolBehaviorComponent>(entity)) {
                if (!patrol->waypoints.empty()) {
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
                    
                    engine::Vector2f safePosition = findSafeUnstuckPosition(entity, nearestWaypoint);
                    
                    b2Body_SetTransform(bodyComp->bodyId, 
                                         {safePosition.x, safePosition.y}, 
                                         b2MakeRot(0.0f));
                    b2Body_SetLinearVelocity(bodyComp->bodyId, {0.0f, 0.0f});
                    b2Body_SetAngularVelocity(bodyComp->bodyId, 0.0f);
                    b2Body_SetAwake(bodyComp->bodyId, true);
                    
                    enemy.lastCheckedPosition = safePosition;
                    enemy.stuckCheckTimer = 0.0f;
                    enemy.stuckCounter = 0;

                    enemy.currentState = EnemyState::Patrol;
                    enemy.stateTimer = 0.0f; 
                    patrol->currentWaypointIndex = nearestIndex;
                    patrol->reversing = false;
                    
                    if (mLogger) {
                        bool wasVisible = AIUtils::isPositionVisibleToPlayer(mContext, nearestWaypoint);
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
            
            if (mLogger) {
                mLogger->warn("Enemy {} has no waypoints for attempt 3, skipping to attempt 4", 
                              entt::to_integral(entity));
            }
            enemy.stuckCounter = 6;
            return;
        }
        
        else if (enemy.stuckCounter >= 6) {
            
            if (AIUtils::isPositionVisibleToPlayer(mContext, transform.position)) {
                 if (mLogger) {
                    mLogger->warn("Enemy {} is visible, applying aggressive shove instead of kill", 
                                  entt::to_integral(entity));
                }
                
                b2Vec2 directions[4] = {
                    {1.0f, 0.0f},  
                    {-1.0f, 0.0f}, 
                    {0.0f, 1.0f},  
                    {0.0f, -1.0f}  
                };
                
                b2Vec2 bestDir = {0.0f, 0.0f};
                float maxDist = -1.0f;
                
                b2QueryFilter filter = b2DefaultQueryFilter();
                filter.maskBits = engine::PhysicsCategory::Wall | engine::PhysicsCategory::LowObstacle; 
                
                for(int i=0; i<4; ++i) {
                     b2Vec2 origin = {transform.position.x, transform.position.y};
                     b2Vec2 translation = {directions[i].x * 100.0f, directions[i].y * 100.0f}; 
                     
                     b2RayResult result = b2World_CastRayClosest(mContext.m_physicsWorld, origin, translation, filter);
                     
                     float dist = 100.0f;
                     if(result.hit) {
                         dist = result.fraction * 100.0f;
                     }
                     
                     if(dist > maxDist) {
                         maxDist = dist;
                         bestDir = directions[i];
                     }
                }
                
                if (maxDist > 10.0f) {
                     float shoveForce = 1000.0f; 
                     b2Vec2 impulse = {bestDir.x * shoveForce, bestDir.y * shoveForce};
                     b2Body_ApplyLinearImpulseToCenter(bodyComp->bodyId, impulse, true);
                     
                     enemy.stunTimer = 0.5f;
                     
                      if (mLogger) {
                        mLogger->info("Enemy {} shoved with force {:.1f} in dir ({:.1f}, {:.1f})", 
                                      entt::to_integral(entity), shoveForce, bestDir.x, bestDir.y);
                    }
                }
                
                return;
            }

            if (mLogger) {
                mLogger->error("Enemy {} PERMANENTLY STUCK after all recovery attempts. Marking as Dead.", 
                               entt::to_integral(entity));
            }
            
            enemy.currentState = EnemyState::Dead;
            
            if (auto* health = mContext.m_registry->try_get<HealthComponent>(entity)) {
                health->currentHp = 0.0f;
            }
            
            if (!mContext.m_registry->any_of<engine::LifetimeComponent>(entity)) {
                mContext.m_registry->emplace<engine::LifetimeComponent>(entity, 3.0f); 
            }
            
            b2Body_SetLinearVelocity(bodyComp->bodyId, {0.0f, 0.0f});
            b2Body_Disable(bodyComp->bodyId);
        }
    }

}
