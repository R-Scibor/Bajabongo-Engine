#include "engine/pch.h"
#include "EnemyCombatSystem.hpp"
#include "game/components/EnemyComponent.hpp"
#include "game/components/WeaponComponent.hpp"
#include "game/components/TurretBehaviorComponent.hpp"
#include "game/components/SniperBehaviorComponent.hpp"
#include "game/components/CampBehaviorComponent.hpp"
#include "engine/components/TransformComponent.hpp"
#include "engine/core/ILoggerManager.hpp"
#include <cmath>
#include <numbers>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

namespace game::ai {

    EnemyCombatSystem::EnemyCombatSystem(engine::EngineContext& context)
        : mContext(context)
    {
        if (context.m_logManager) {
            mLogger = context.m_logManager->GetLogger("EnemyCombat");
        }
    }

    void EnemyCombatSystem::update(float dt) {
        auto& registry = *mContext.m_registry;
        auto view = registry.view<EnemyComponent, engine::TransformComponent, WeaponComponent>();
        
        view.each([&](entt::entity entity, EnemyComponent& enemy, engine::TransformComponent& transform, WeaponComponent& weapon) {
            weapon.wantsToShoot = false;
            weapon.wantsToReload = false;
            
            if (enemy.currentState != EnemyState::Shoot && 
                enemy.currentState != EnemyState::Camp && 
                enemy.currentState != EnemyState::Turret) {
                return;
            }
            
            if (enemy.targetEntity == entt::null || !registry.valid(enemy.targetEntity)) {
                return;
            }
            
            if (!enemy.hasLineOfSight) {
                return;
            }
            
            const auto& targetTransform = registry.get<engine::TransformComponent>(enemy.targetEntity);
            engine::Vector2f toTarget = targetTransform.position - transform.position;
            float distanceSq = toTarget.x * toTarget.x + toTarget.y * toTarget.y;
            
            float desiredAimAngle = std::atan2(toTarget.y, toTarget.x);
            
            if (auto* turret = registry.try_get<TurretBehaviorComponent>(entity)) {
                float angleDiff = desiredAimAngle - weapon.aimAngle;
                
                while (angleDiff > M_PI) angleDiff -= 2.0f * M_PI;
                while (angleDiff < -M_PI) angleDiff += 2.0f * M_PI;
                
                float maxRotation = turret->rotationSpeed * dt;
                if (std::abs(angleDiff) < maxRotation) {
                    weapon.aimAngle = desiredAimAngle;
                } else {
                    weapon.aimAngle += (angleDiff > 0.0f ? maxRotation : -maxRotation);
                }
                
                float scaleX = std::abs(transform.scale.x);
                if (weapon.aimAngle > M_PI / 2.0f || weapon.aimAngle < -M_PI / 2.0f) {
                    transform.scale.x = -scaleX;
                } else {
                    transform.scale.x = scaleX;
                }
                
                if (std::abs(angleDiff) < turret->shootAngleTolerance) {
                    weapon.wantsToShoot = true;
                }
            }
            else if (auto* sniper = registry.try_get<SniperBehaviorComponent>(entity)) {
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
            else {
                weapon.aimAngle = desiredAimAngle;
                
                if (distanceSq < enemy.attackRange * enemy.attackRange) {
                    weapon.wantsToShoot = true;
                }
            }
            
            if (weapon.currentAmmo == 0 && weapon.totalAmmo > 0) {
                weapon.wantsToReload = true;
            }
        });
    }

}
