#include "engine/pch.h"
#include "EnemyCombatSystem.hpp"
#include "game/components/EnemyComponent.hpp"
#include "game/components/WeaponComponent.hpp"
#include "game/components/TurretBehaviorComponent.hpp"
#include "game/components/SniperBehaviorComponent.hpp"
#include "game/components/CampBehaviorComponent.hpp"
#include "engine/components/TransformComponent.hpp"
#include "engine/components/AnimationStateComponent.hpp"
#include "engine/components/PhysicsBodyComponent.hpp"
#include "engine/physics/PhysicsConstants.hpp"
#include "engine/core/ILoggerManager.hpp"
#include <box2d/box2d.h>
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
            
            // --- Update Animation State for Combat ---
            auto* animState = registry.try_get<engine::AnimationStateComponent>(entity);
            
            if (enemy.currentState != EnemyState::Shoot && 
                enemy.currentState != EnemyState::Camp && 
                enemy.currentState != EnemyState::Turret) {
                // Not in combat mode
                return;
            }
            
            if (enemy.targetEntity == entt::null || !registry.valid(enemy.targetEntity)) {
                return;
            }
            
            if (!enemy.hasLineOfSight) {
                return;
            }
            
            // Calculate Muzzle Position
            engine::Vector2f muzzlePos = { transform.position.x, transform.position.y + WEAPON_MUZZLE_HEIGHT_OFFSET };
            
            // Calculate Target Position (Dynamic Chest Detection)
            engine::Vector2f targetPos;
            bool targetFound = false;

            // 1. Try to find Hurtbox Center
            if (auto* targetBody = registry.try_get<engine::PhysicsBodyComponent>(enemy.targetEntity)) {
                if (b2Body_IsValid(targetBody->bodyId)) {
                    b2ShapeId shapes[8];
                    int shapeCount = b2Body_GetShapes(targetBody->bodyId, shapes, 8);
                    
                    for (int i = 0; i < shapeCount; ++i) {
                        b2Filter filter = b2Shape_GetFilter(shapes[i]);
                        if (filter.categoryBits & engine::PhysicsCategory::Hurtbox) {
                            b2AABB aabb = b2Shape_GetAABB(shapes[i]);
                            targetPos.x = (aabb.lowerBound.x + aabb.upperBound.x) * 0.5f;
                            targetPos.y = (aabb.lowerBound.y + aabb.upperBound.y) * 0.5f;
                            targetFound = true;
                            break;
                        }
                    }
                }
            }

            // 2. Fallback to constant offset if no Hurtbox found
            if (!targetFound) {
                 const auto& targetTransform = registry.get<engine::TransformComponent>(enemy.targetEntity);
                 targetPos = { targetTransform.position.x, targetTransform.position.y + TARGET_CHEST_HEIGHT_OFFSET };
            }

            engine::Vector2f toTarget = targetPos - muzzlePos;
            float distanceSq = toTarget.x * toTarget.x + toTarget.y * toTarget.y;
            
            float desiredAimAngle = std::atan2(toTarget.y, toTarget.x);
            
            // Default to aiming at target
            weapon.aimAngle = desiredAimAngle;

            // Handle Aim Animation State
            if (animState) {
                // Update Facing based on aim angle
                float normalizedAim = desiredAimAngle;
                while (normalizedAim > M_PI) normalizedAim -= 2.0f * M_PI; // TODO: Extract to wrapAngle() helper
                while (normalizedAim < -M_PI) normalizedAim += 2.0f * M_PI;
                
                if (std::abs(normalizedAim) > M_PI / 2.0f) {
                    animState->facing = engine::FacingDirection::Left;
                } else {
                    animState->facing = engine::FacingDirection::Right;
                }
            }

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
                    if (animState) animState->state = engine::AnimationState::Aim;
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
                
                // While aiming, show Aim animation
                if (animState) animState->state = engine::AnimationState::Aim;

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
                // Regular shooter
                weapon.aimAngle = desiredAimAngle;
                
                if (distanceSq < enemy.attackRange * enemy.attackRange) {
                    weapon.wantsToShoot = true;
                    if (animState) animState->state = engine::AnimationState::Aim;
                }
            }
            
            if (weapon.currentAmmo == 0 && weapon.totalAmmo > 0) {
                weapon.wantsToReload = true;
            }
        });
    }

}
