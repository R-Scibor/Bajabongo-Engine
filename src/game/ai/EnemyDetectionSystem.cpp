#include "engine/pch.h"
#include "EnemyDetectionSystem.hpp"
#include "DetectionMath.hpp"
#include "AIUtils.hpp"
#include "game/components/EnemyComponent.hpp"
#include "game/components/PlayerComponent.hpp"
#include "game/components/HealthComponent.hpp"
#include "engine/components/TransformComponent.hpp"
#include "engine/core/ILogger.hpp"
#include "engine/core/ILoggerManager.hpp"

namespace game::ai {

    EnemyDetectionSystem::EnemyDetectionSystem(engine::EngineContext& context) : mContext(context) {
        if (context.m_logManager) {
            mLogger = context.m_logManager->getLogger("EnemyDetection");
        }
    }

    void EnemyDetectionSystem::update(float dt) {
        auto& registry = *mContext.m_registry;

        auto playerView = registry.view<PlayerComponent, engine::TransformComponent>();
        auto enemyView = registry.view<EnemyComponent, engine::TransformComponent>();

        entt::entity playerEntity = entt::null;
        engine::Vector2f playerPos{0.0f, 0.0f};

        playerView.each([&](entt::entity entity, const PlayerComponent&, const engine::TransformComponent& transform) {
            playerEntity = entity;
            playerPos = transform.position;
        });

        if (playerEntity == entt::null) {
            return;
        }

        enemyView.each([&](entt::entity entity, EnemyComponent& enemy, const engine::TransformComponent& transform) {
            if (isEnemyDefeated(registry.try_get<HealthComponent>(entity))) {
                enemy.currentState = EnemyState::Dead;
                return;
            }

            bool inRange = isWithinRadius(transform.position, playerPos, enemy.detectionRadius);
            bool hasLoS = false;

            if (inRange) {
                hasLoS = AIUtils::hasLineOfSight(mContext, transform.position, playerPos);
            }

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
            } else {
                // Memory decay
                if (enemy.targetEntity != entt::null) {
                    enemy.lastSeenTimer += dt; // Use passed delta time
                    if (hasForgottenTarget(enemy.lastSeenTimer, enemy.memoryDuration)) {
                        enemy.targetEntity = entt::null;
                    }
                }
            }
        });
    }

    void EnemyDetectionSystem::alertNearbyEnemies(entt::entity source, const engine::Vector2f& position) {
        if (mLogger) {
            mLogger->info("Alerting enemies near entity={} at ({:.1f}, {:.1f})", entt::to_integral(source), position.x,
                          position.y);
        }

        auto& registry = *mContext.m_registry;
        auto& sourceEnemy = registry.get<EnemyComponent>(source);
        const auto& sourceTransform = registry.get<engine::TransformComponent>(source);

        int alertCount = 0;

        auto enemyView = registry.view<EnemyComponent, engine::TransformComponent>();
        enemyView.each([&](entt::entity entity, EnemyComponent& enemy, const engine::TransformComponent& transform) {
            if (entity == source) return;

            if (isWithinRadius(transform.position, sourceTransform.position, sourceEnemy.alertRadius)) {
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

} // namespace game::ai
