#include "engine/pch.h"
#include "EnemyStateTransitions.hpp"
#include "game/components/EnemyComponent.hpp"
#include "game/components/HealthComponent.hpp"
#include "game/components/PatrolBehaviorComponent.hpp"
#include "game/components/RushBehaviorComponent.hpp"
#include "game/components/SniperBehaviorComponent.hpp"
#include "engine/components/TransformComponent.hpp"
#include "engine/core/ILoggerManager.hpp"
#include <limits>

namespace game::ai {

    EnemyStateTransitions::EnemyStateTransitions(engine::EngineContext& context)
        : mContext(context)
    {
        if (context.m_logManager) {
            mLogger = context.m_logManager->GetLogger("EnemyStates");
        }
    }

    void EnemyStateTransitions::update(float dt) {
        auto& registry = *mContext.m_registry;
        auto view = registry.view<EnemyComponent, engine::TransformComponent>();
        
        view.each([&](entt::entity entity, EnemyComponent& enemy, const engine::TransformComponent& transform) {
            
            enemy.stateTimer += dt;
            
            EnemyState oldState = enemy.currentState;
            
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
                case EnemyState::Rush:
                    transitionFromRush(entity, enemy, transform);
                    break;
                case EnemyState::Retreat:
                    transitionFromRetreat(entity, enemy, transform);
                    break;
                case EnemyState::Approach:
                    transitionFromApproach(entity, enemy, transform);
                    break;
                case EnemyState::Dead:
                    break;
                default:
                    break;
            }
            
            if (oldState != enemy.currentState) {
                if (oldState == EnemyState::Shoot) {
                    if (auto* sniper = registry.try_get<SniperBehaviorComponent>(entity)) {
                        sniper->isAiming = false;
                        sniper->aimTimer = 0.0f;
                    }
                }

                if (mLogger) {
                    mLogger->debug("Enemy {} transitioned from {} to {}", 
                                   entt::to_integral(entity), 
                                   static_cast<int>(oldState), 
                                   static_cast<int>(enemy.currentState));
                }
            }
        });
    }

    void EnemyStateTransitions::transitionFromIdle(entt::entity entity, EnemyComponent& enemy, const engine::TransformComponent& transform) {
        if (mContext.m_registry->any_of<PatrolBehaviorComponent>(entity)) {
            enemy.currentState = EnemyState::Patrol;
            enemy.stateTimer = 0.0f;
            return;
        }
        
        if (enemy.targetEntity != entt::null && enemy.hasLineOfSight) {
            enemy.currentState = EnemyState::Chase;
            enemy.stateTimer = 0.0f;
            return;
        }
        
        if (enemy.isAlerted && enemy.targetEntity == entt::null) {
            enemy.currentState = EnemyState::Alert;
            enemy.stateTimer = 0.0f;
            return;
        }
    }

    void EnemyStateTransitions::transitionFromPatrol(entt::entity entity, EnemyComponent& enemy, const engine::TransformComponent& transform) {
        if (auto* health = mContext.m_registry->try_get<HealthComponent>(entity)) {
            if (health->currentHp < health->maxHp * enemy.retreatHpPercent) {
                enemy.currentState = EnemyState::Retreat;
                enemy.stateTimer = 0.0f;
                return;
            }
        }
        
        if (enemy.targetEntity != entt::null && enemy.hasLineOfSight) {
            enemy.currentState = EnemyState::Chase;
            enemy.stateTimer = 0.0f;
            return;
        }
        
        if (enemy.isAlerted && enemy.targetEntity == entt::null) {
            enemy.currentState = EnemyState::Alert;
            enemy.stateTimer = 0.0f;
            return;
        }
    }

    void EnemyStateTransitions::transitionFromAlert(entt::entity entity, EnemyComponent& enemy, const engine::TransformComponent& transform) {
        if (enemy.targetEntity != entt::null && enemy.hasLineOfSight) {
            enemy.currentState = EnemyState::Chase;
            enemy.stateTimer = 0.0f;
            return;
        }
        
        if (enemy.targetEntity == entt::null && enemy.isAlerted) {
            enemy.currentState = EnemyState::Approach;
            enemy.stateTimer = 0.0f;
            return;
        }
        
        if (enemy.stateTimer > 10.0f) {
            if (mContext.m_registry->any_of<PatrolBehaviorComponent>(entity)) {
                enemy.currentState = EnemyState::Patrol;
            } else {
                enemy.currentState = EnemyState::Idle;
            }
            enemy.isAlerted = false;
            enemy.stateTimer = 0.0f;
            return;
        }
    }

    void EnemyStateTransitions::transitionFromChase(entt::entity entity, EnemyComponent& enemy, const engine::TransformComponent& transform) {
        auto& registry = *mContext.m_registry;

        if (auto* health = registry.try_get<HealthComponent>(entity)) {
            if (health->currentHp < health->maxHp * enemy.retreatHpPercent) {
                enemy.currentState = EnemyState::Retreat;
                enemy.stateTimer = 0.0f;
                return;
            }
        }
        
        float distSq = std::numeric_limits<float>::max();
        bool targetValid = false;
        
        if (enemy.targetEntity != entt::null && registry.valid(enemy.targetEntity)) {
            const auto& targetTransform = registry.get<engine::TransformComponent>(enemy.targetEntity);
            engine::Vector2f toTarget = targetTransform.position - transform.position;
            distSq = toTarget.x * toTarget.x + toTarget.y * toTarget.y;
            targetValid = true;
        } else {
            enemy.targetEntity = entt::null;
            enemy.currentState = EnemyState::Idle; 
            return;
        }

        if (registry.any_of<RushBehaviorComponent>(entity)) {
            auto& rush = registry.get<RushBehaviorComponent>(entity);
            if (distSq < rush.rushActivationRange * rush.rushActivationRange && rush.rushCooldownTimer <= 0.0f) {
                enemy.currentState = EnemyState::Rush;
                enemy.stateTimer = 0.0f;
                return;
            }
        }
        
        if (enemy.hasLineOfSight) {
            if (distSq < enemy.attackRange * enemy.attackRange) {
                enemy.currentState = EnemyState::Shoot;
                enemy.stateTimer = 0.0f;
                return;
            }
        } else {
            enemy.currentState = EnemyState::Approach;
            enemy.stateTimer = 0.0f;
            return;
        }
    }

    void EnemyStateTransitions::transitionFromShoot(entt::entity entity, EnemyComponent& enemy, const engine::TransformComponent& transform) {
        auto& registry = *mContext.m_registry;

        if (auto* health = registry.try_get<HealthComponent>(entity)) {
            if (health->currentHp < health->maxHp * enemy.retreatHpPercent) {
                enemy.currentState = EnemyState::Retreat;
                enemy.stateTimer = 0.0f;
                return;
            }
        }
        
        if (enemy.targetEntity != entt::null && registry.valid(enemy.targetEntity)) {
            const auto& targetTransform = registry.get<engine::TransformComponent>(enemy.targetEntity);
            engine::Vector2f toTarget = targetTransform.position - transform.position;
            float distSq = toTarget.x * toTarget.x + toTarget.y * toTarget.y;
            
            if (distSq >= enemy.attackRange * enemy.attackRange) {
                enemy.currentState = EnemyState::Chase;
                enemy.stateTimer = 0.0f;
                return;
            }
        }
        
        if (!enemy.hasLineOfSight) {
            enemy.currentState = EnemyState::Approach;
            enemy.stateTimer = 0.0f;
            return;
        }
    }

    void EnemyStateTransitions::transitionFromRush(entt::entity entity, EnemyComponent& enemy, const engine::TransformComponent& transform) {
        auto& registry = *mContext.m_registry;
        auto* rush = registry.try_get<RushBehaviorComponent>(entity);
        if (!rush) {
            enemy.currentState = EnemyState::Chase;
            enemy.stateTimer = 0.0f;
            return;
        }
        
        if (auto* health = registry.try_get<HealthComponent>(entity)) {
            if (health->currentHp < health->maxHp * enemy.retreatHpPercent) {
                enemy.currentState = EnemyState::Retreat;
                enemy.stateTimer = 0.0f;
                rush->rushCooldownTimer = rush->rushCooldown;
                return;
            }
        }
        
        if (enemy.targetEntity == entt::null || !registry.valid(enemy.targetEntity)) {
            enemy.currentState = EnemyState::Idle;
            enemy.stateTimer = 0.0f;
            rush->rushCooldownTimer = rush->rushCooldown;
            return;
        }
        
        const auto& targetTransform = registry.get<engine::TransformComponent>(enemy.targetEntity);
        engine::Vector2f toTarget = targetTransform.position - transform.position;
        float distSq = toTarget.x * toTarget.x + toTarget.y * toTarget.y;
        
        if (distSq < enemy.attackRange * enemy.attackRange && enemy.hasLineOfSight) {
            enemy.currentState = EnemyState::Shoot;
            enemy.stateTimer = 0.0f;
            rush->rushCooldownTimer = rush->rushCooldown;
            return;
        }
        
        if (!enemy.hasLineOfSight || distSq > rush->rushActivationRange * rush->rushActivationRange * 4.0f) {
            enemy.currentState = EnemyState::Chase;
            enemy.stateTimer = 0.0f;
            rush->rushCooldownTimer = rush->rushCooldown;
            return;
        }
    }

    void EnemyStateTransitions::transitionFromRetreat(entt::entity entity, EnemyComponent& enemy, const engine::TransformComponent& transform) {
        if (auto* health = mContext.m_registry->try_get<HealthComponent>(entity)) {
            if (health->currentHp >= health->maxHp * (enemy.retreatHpPercent + 0.2f)) {
                enemy.currentState = EnemyState::Chase;
                enemy.stateTimer = 0.0f;
                return;
            }
        }
        
        if (enemy.targetEntity == entt::null || !mContext.m_registry->valid(enemy.targetEntity)) {
            enemy.currentState = EnemyState::Idle;
            enemy.stateTimer = 0.0f;
            return;
        }
    }

    void EnemyStateTransitions::transitionFromApproach(entt::entity entity, EnemyComponent& enemy, const engine::TransformComponent& transform) {
        if (enemy.targetEntity != entt::null && enemy.hasLineOfSight) {
            enemy.currentState = EnemyState::Chase;
            enemy.stateTimer = 0.0f;
            return;
        }
        
        if (enemy.targetEntity == entt::null || !mContext.m_registry->valid(enemy.targetEntity)) {
            engine::Vector2f toLastKnown = enemy.lastKnownPosition - transform.position;
            float distSq = toLastKnown.x * toLastKnown.x + toLastKnown.y * toLastKnown.y;
            
            if (distSq < 50.0f * 50.0f) {
                enemy.currentState = EnemyState::Idle;
                enemy.stateTimer = 0.0f;
                return;
            }
        }
    }

}
