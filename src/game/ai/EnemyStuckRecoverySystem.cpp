#include "engine/pch.h"
#include "EnemyStuckRecoverySystem.hpp"
#include "AIUtils.hpp"
#include "EnemyNavigation.hpp"
#include "game/components/EnemyComponent.hpp"
#include "game/components/HealthComponent.hpp"
#include "game/components/PatrolBehaviorComponent.hpp"
#include "game/ai/PatrolWaypoints.hpp"
#include "engine/components/TransformComponent.hpp"
#include "engine/components/PhysicsBodyComponent.hpp"
#include "engine/components/LifetimeComponent.hpp"
#include "engine/core/ILoggerManager.hpp"
#include "engine/physics/PhysicsConstants.hpp"
#include <box2d/box2d.h>
#include <cmath>

namespace game::ai {

    EnemyStuckRecoverySystem::EnemyStuckRecoverySystem(engine::EngineContext& context) : mContext(context) {
        if (context.m_logManager) {
            mLogger = context.m_logManager->getLogger("EnemyStuck");
        }
    }

    void EnemyStuckRecoverySystem::update(float dt) {
        auto view = mContext.m_registry->view<EnemyComponent, engine::TransformComponent>();

        view.each([&](entt::entity entity, EnemyComponent& enemy, const engine::TransformComponent& transform) {
            if (enemy.currentState == EnemyState::Idle || enemy.currentState == EnemyState::Dead ||
                enemy.currentState == EnemyState::Shoot || enemy.currentState == EnemyState::Turret) {

                if (enemy.stuckCounter > 0) {
                    enemy.stuckCounter = 0;
                    enemy.lastCheckedPosition = transform.position;
                    enemy.stuckCheckTimer = 0.0f;
                    if (mLogger) {
                        mLogger->debug("Enemy {} stopped moving, reset stuck counter", entt::to_integral(entity));
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
                                      entt::to_integral(entity), enemy.stuckCounter, distanceMoved,
                                      enemy.stuckCheckInterval);
                    }

                    if (enemy.currentState == EnemyState::Patrol && enemy.stuckCounter >= 2) {
                        if (auto* patrol = mContext.m_registry->try_get<PatrolBehaviorComponent>(entity)) {
                            const int previous = patrol->currentWaypointIndex;
                            if (skipToNextPatrolWaypoint(*patrol)) {
                                enemy.stuckCounter = 0;
                                enemy.stuckCheckTimer = 0.0f;
                                if (mLogger) {
                                    mLogger->info("Enemy {} skipped blocked patrol waypoint #{} -> #{}",
                                                  entt::to_integral(entity), previous, patrol->currentWaypointIndex);
                                }
                            }
                        }
                    }

                    if ((enemy.currentState == EnemyState::Chase || enemy.currentState == EnemyState::Approach ||
                         enemy.currentState == EnemyState::Rush) &&
                        enemy.stuckCounter >= 2) {
                        auto* bodyComp = mContext.m_registry->try_get<engine::PhysicsBodyComponent>(entity);
                        engine::Vector2f dir{0.0f, 0.0f};
                        if (bodyComp && b2Body_IsValid(bodyComp->bodyId)) {
                            const SolidFootprint foot = solidFootprint(bodyComp->bodyId, transform.position);
                            dir = mostOpenDirection(mContext.m_physicsWorld, foot, 80.0f);
                        }
                        if (dir.x == 0.0f && dir.y == 0.0f) {
                            engine::Vector2f toTarget = enemy.lastKnownPosition - transform.position;
                            dir = {-toTarget.y, toTarget.x};
                            const float mag = std::sqrt(dir.x * dir.x + dir.y * dir.y);
                            if (mag > 0.01f) {
                                dir = {dir.x / mag, dir.y / mag};
                            } else {
                                dir = {1.0f, 0.0f};
                            }
                        }
                        enemy.sidestepDirection = dir;
                        enemy.sidestepTimer = 0.75f;
                        enemy.stuckCounter = 0;
                        enemy.stuckCheckTimer = 0.0f;
                        if (mLogger) {
                            mLogger->info("Enemy {} chase sidestep ({:.2f}, {:.2f})", entt::to_integral(entity), dir.x,
                                          dir.y);
                        }
                    }

                    if (enemy.stuckCounter >= 3) {
                        executeUnstuckBehavior(entity, enemy, transform);
                    }
                } else {
                    if (enemy.stuckCounter > 0) {
                        if (mLogger) {
                            mLogger->info("Enemy {} recovered from stuck state after {} checks",
                                          entt::to_integral(entity), enemy.stuckCounter);
                        }
                        enemy.stuckCounter = 0;
                    }
                }

                enemy.lastCheckedPosition = transform.position;
                enemy.stuckCheckTimer = 0.0f;
            }
        });
    }

    engine::Vector2f EnemyStuckRecoverySystem::findSafeUnstuckPosition(entt::entity entity,
                                                                       const engine::Vector2f& preferredPos) {
        auto* bodyComp = mContext.m_registry->try_get<engine::PhysicsBodyComponent>(entity);
        SolidFootprint foot;
        if (bodyComp && b2Body_IsValid(bodyComp->bodyId)) {
            foot = solidFootprint(bodyComp->bodyId, preferredPos);
        }

        auto isClear = [&](const engine::Vector2f& pos) {
            return !areaBlockedByWalls(mContext.m_physicsWorld, pos, foot.halfWidth, foot.halfHeight);
        };

        if (isClear(preferredPos) && !AIUtils::isPositionVisibleToPlayer(mContext, preferredPos)) {
            return preferredPos;
        }

        if (mLogger && !isClear(preferredPos)) {
            mLogger->debug("Enemy {} preferred unstuck position ({:.1f}, {:.1f}) overlaps a wall",
                           entt::to_integral(entity), preferredPos.x, preferredPos.y);
        } else if (mLogger) {
            mLogger->debug("Enemy {} preferred unstuck position is visible, searching for alternative",
                           entt::to_integral(entity));
        }

        auto firstClearWaypoint = [&](bool requireHidden) -> const engine::Vector2f* {
            if (auto* patrol = mContext.m_registry->try_get<PatrolBehaviorComponent>(entity)) {
                for (const auto& waypoint : patrol->waypoints) {
                    if (!isClear(waypoint)) continue;
                    if (requireHidden && AIUtils::isPositionVisibleToPlayer(mContext, waypoint)) continue;
                    return &waypoint;
                }
            }
            return nullptr;
        };

        if (const auto* hidden = firstClearWaypoint(true)) {
            if (mLogger) {
                mLogger->debug("Enemy {} found safe waypoint at ({:.1f}, {:.1f})", entt::to_integral(entity), hidden->x,
                               hidden->y);
            }
            return *hidden;
        }
        if (const auto* clearWp = firstClearWaypoint(false)) {
            return *clearWp;
        }

        engine::Vector2f firstClearRandom{0.0f, 0.0f};
        bool haveClearRandom = false;
        for (int attempt = 0; attempt < 8; ++attempt) {
            float angle = mContext.m_random.real(0.0f, 6.28318f);
            float radius = mContext.m_random.real(100.0f, 300.0f);

            engine::Vector2f candidate = {preferredPos.x + std::cos(angle) * radius,
                                          preferredPos.y + std::sin(angle) * radius};

            if (!isClear(candidate)) continue;
            if (!haveClearRandom) {
                firstClearRandom = candidate;
                haveClearRandom = true;
            }
            if (!AIUtils::isPositionVisibleToPlayer(mContext, candidate)) {
                if (mLogger) {
                    mLogger->debug("Enemy {} found safe random position at ({:.1f}, {:.1f}) after {} attempts",
                                   entt::to_integral(entity), candidate.x, candidate.y, attempt + 1);
                }
                return candidate;
            }
        }

        if (haveClearRandom) return firstClearRandom;
        if (isClear(preferredPos)) return preferredPos;

        if (mLogger) {
            mLogger->warn("Enemy {} could not find a clear unstuck position, using preferred",
                          entt::to_integral(entity));
        }
        return preferredPos;
    }

    void EnemyStuckRecoverySystem::executeUnstuckBehavior(entt::entity entity, EnemyComponent& enemy,
                                                          const engine::TransformComponent& transform) {
        auto* bodyComp = mContext.m_registry->try_get<engine::PhysicsBodyComponent>(entity);
        if (!bodyComp || !b2Body_IsValid(bodyComp->bodyId)) {
            if (mLogger) {
                mLogger->error("Enemy {} has invalid physics body, cannot execute unstuck", entt::to_integral(entity));
            }
            enemy.currentState = EnemyState::Dead;
            return;
        }

        const SolidFootprint foot = solidFootprint(bodyComp->bodyId, transform.position);

        auto shoveOpen = [&](float force, float stun) -> engine::Vector2f {
            engine::Vector2f dir = mostOpenDirection(mContext.m_physicsWorld, foot, 80.0f);
            if (dir.x == 0.0f && dir.y == 0.0f) {
                const float angle = mContext.m_random.real(0.0f, 6.28318f);
                dir = {std::cos(angle), std::sin(angle)};
            }
            b2Body_ApplyLinearImpulseToCenter(bodyComp->bodyId, {dir.x * force, dir.y * force}, true);
            enemy.stunTimer = stun;
            return dir;
        };

        auto teleportTo = [&](const engine::Vector2f& dest) {
            b2Body_SetTransform(bodyComp->bodyId, {dest.x, dest.y}, b2MakeRot(0.0f));
            b2Body_SetLinearVelocity(bodyComp->bodyId, {0.0f, 0.0f});
            b2Body_SetAngularVelocity(bodyComp->bodyId, 0.0f);
            b2Body_SetAwake(bodyComp->bodyId, true);
            enemy.lastCheckedPosition = dest;
            enemy.lastValidPosition = dest;
            enemy.previousPosition = dest;
            enemy.stuckCheckTimer = 0.0f;
            enemy.stuckCounter = 0;
        };

        const bool visible = AIUtils::isPositionVisibleToPlayer(mContext, transform.position);

        // Phase 1: shove toward an open side (fall back to a random direction)
        if (enemy.stuckCounter == 3) {
            const engine::Vector2f dir = shoveOpen(mContext.m_random.real(120.0f, 200.0f), 0.15f);
            if (mLogger) {
                mLogger->info("Enemy {} unstuck attempt 1/4: open-direction impulse ({:.2f}, {:.2f})",
                              entt::to_integral(entity), dir.x, dir.y);
            }
            return;
        }

        // Phase 2: last-valid teleport if hidden; if the player can see the enemy, shove instead
        else if (enemy.stuckCounter == 4) {
            if (visible) {
                const engine::Vector2f dir = shoveOpen(200.0f, 0.2f);
                if (mLogger) {
                    mLogger->warn("Enemy {} is visible, impulse instead of last-valid teleport ({:.2f}, {:.2f})",
                                  entt::to_integral(entity), dir.x, dir.y);
                }
                return;
            }

            if (enemy.lastValidPosition.x != 0.0f || enemy.lastValidPosition.y != 0.0f) {
                engine::Vector2f safePosition = findSafeUnstuckPosition(entity, enemy.lastValidPosition);
                if (areaBlockedByWalls(mContext.m_physicsWorld, safePosition, foot.halfWidth, foot.halfHeight)) {
                    const engine::Vector2f dir = shoveOpen(200.0f, 0.2f);
                    if (mLogger) {
                        mLogger->warn("Enemy {} last valid is blocked, impulse instead ({:.2f}, {:.2f})",
                                      entt::to_integral(entity), dir.x, dir.y);
                    }
                    return;
                }
                teleportTo(safePosition);
                if (mLogger) {
                    bool wasVisible = AIUtils::isPositionVisibleToPlayer(mContext, enemy.lastValidPosition);
                    mLogger->info("Enemy {} unstuck attempt 2/4: teleport to last valid position ({:.1f}, {:.1f}) [{}]",
                                  entt::to_integral(entity), safePosition.x, safePosition.y,
                                  wasVisible ? "adjusted for visibility" : "safe");
                }
            } else {
                const engine::Vector2f dir = shoveOpen(200.0f, 0.2f);
                if (mLogger) {
                    mLogger->warn("Enemy {} has no last valid position, impulse instead ({:.2f}, {:.2f})",
                                  entt::to_integral(entity), dir.x, dir.y);
                }
            }
            return;
        }

        // Phase 3: waypoint teleport if hidden; shove when visible
        else if (enemy.stuckCounter == 5) {
            if (visible) {
                const engine::Vector2f dir = shoveOpen(250.0f, 0.25f);
                if (mLogger) {
                    mLogger->warn("Enemy {} is visible, impulse instead of waypoint teleport ({:.2f}, {:.2f})",
                                  entt::to_integral(entity), dir.x, dir.y);
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
                            nearestIndex = static_cast<int>(i);
                        }
                    }

                    engine::Vector2f safePosition = findSafeUnstuckPosition(entity, nearestWaypoint);
                    if (areaBlockedByWalls(mContext.m_physicsWorld, safePosition, foot.halfWidth, foot.halfHeight)) {
                        const engine::Vector2f dir = shoveOpen(250.0f, 0.25f);
                        if (mLogger) {
                            mLogger->warn("Enemy {} waypoint is blocked, impulse instead ({:.2f}, {:.2f})",
                                          entt::to_integral(entity), dir.x, dir.y);
                        }
                        return;
                    }

                    teleportTo(safePosition);
                    enemy.currentState = EnemyState::Patrol;
                    enemy.stateTimer = 0.0f;
                    patrol->currentWaypointIndex = nearestIndex;
                    patrol->reversing = false;

                    if (mLogger) {
                        bool wasVisible = AIUtils::isPositionVisibleToPlayer(mContext, nearestWaypoint);
                        mLogger->info("Enemy {} unstuck attempt 3/4: teleport to waypoint #{} ({:.1f}, {:.1f}) [{}]",
                                      entt::to_integral(entity), nearestIndex, safePosition.x, safePosition.y,
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

        // Phase 4: Aggressive Shove or Kill
        else if (enemy.stuckCounter >= 6) {

            if (visible) {
                if (mLogger) mLogger->warn("Enemy {} is visible, attempting shove", entt::to_integral(entity));

                engine::Vector2f bestDir = mostOpenDirection(mContext.m_physicsWorld, foot, 100.0f);
                if (bestDir.x != 0.0f || bestDir.y != 0.0f) {
                    const float shoveForce = 1000.0f;
                    b2Body_ApplyLinearImpulseToCenter(bodyComp->bodyId,
                                                      {bestDir.x * shoveForce, bestDir.y * shoveForce}, true);
                    enemy.stunTimer = 0.5f;
                    if (mLogger) {
                        mLogger->info("Enemy {} shoved with force {:.1f} in dir ({:.1f}, {:.1f})",
                                      entt::to_integral(entity), shoveForce, bestDir.x, bestDir.y);
                    }
                }

                return;
            }

            if (mLogger) mLogger->error("Enemy {} PERMANENTLY STUCK. Killing.", entt::to_integral(entity));

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

} // namespace game::ai
