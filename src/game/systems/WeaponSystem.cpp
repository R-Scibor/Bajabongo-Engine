#include "engine/pch.h"
#include "WeaponSystem.hpp"
#include "game/components/WeaponComponent.hpp"
#include "engine/core/EngineContext.hpp"
#include "engine/core/ILogger.hpp"
#include "engine/core/ILoggerManager.hpp"
#include "engine/components/PhysicsBodyComponent.hpp"
#include "engine/components/PendingPhysicsBodyComponent.hpp"
#include "engine/components/TransformComponent.hpp"
#include "engine/components/RenderableComponent.hpp"
#include "engine/components/LifetimeComponent.hpp"
#include "game/components/ProjectileComponent.hpp"
#include "game/components/DamageComponent.hpp"
#include "engine/physics/PhysicsConstants.hpp"

#include <entt/entt.hpp>
#include <box2d/box2d.h>
#include <cmath>
#include <numbers>

#ifndef PI
#define PI 3.14159265359f
#endif

namespace game {

    WeaponSystem::WeaponSystem(engine::EngineContext& context)
        : m_context(context)
        , m_rng(std::random_device{}())
    {
    }

    void WeaponSystem::update(float fixedDeltaTime)
    {
        auto& registry = *m_context.m_registry;
        // We need PhysicsBodyComponent to know where to spawn the bullet (origin of the shooter)
        auto view = registry.view<WeaponComponent, engine::PhysicsBodyComponent>();

        view.each([&](entt::entity entity, WeaponComponent& weapon, engine::PhysicsBodyComponent& bodyComp) {
            
            // Ensure physics body is valid
            if (!b2Body_IsValid(bodyComp.bodyId)) return;

            // Spread Decay
            if (weapon.currentSpreadDeg > weapon.baseSpreadDeg) {
                weapon.currentSpreadDeg -= weapon.spreadDecayDegPerSec * fixedDeltaTime;
                weapon.currentSpreadDeg = std::max(weapon.currentSpreadDeg, weapon.baseSpreadDeg);
            }

            // Recoil Return (decay toward zero)
            if (std::abs(weapon.recoilAngleOffset) > 0.01f) {
                float sign = weapon.recoilAngleOffset > 0 ? -1.0f : 1.0f;
                weapon.recoilAngleOffset += sign * weapon.recoilReturnDegPerSec * fixedDeltaTime * (PI / 180.0f);

                // Clamp to zero when close
                if (std::abs(weapon.recoilAngleOffset) < 0.01f) {
                    weapon.recoilAngleOffset = 0.0f;
                }
            }

            // Cooldown Management
            if (weapon.cooldownTimer > 0.0f) {
                weapon.cooldownTimer -= fixedDeltaTime;
            }

            // Reload Management
            if (weapon.isReloading) {
                weapon.reloadTimer -= fixedDeltaTime;

                if (weapon.reloadTimer <= 0.0f) {
                    // Calculate how much ammo we need
                    int ammoNeeded = weapon.magSize - weapon.currentAmmo;
                    
                    int ammoToLoad = ammoNeeded;
                    if (weapon.totalAmmo < ammoToLoad) {
                        ammoToLoad = weapon.totalAmmo;
                    }
                    
                    weapon.currentAmmo += ammoToLoad;
                    weapon.totalAmmo -= ammoToLoad;
                    
                    weapon.isReloading = false;
                    weapon.wantsToReload = false; // Reset flag
                    weapon.isBursting = false;
                    weapon.burstShotsFired = 0;
                    
                    if (m_context.m_logManager) {
                         auto logger = m_context.m_logManager->GetLogger("WeaponSystem");
                         if (logger) logger->debug("Reload complete. Ammo: {}/{}", weapon.currentAmmo, weapon.totalAmmo);
                    }
                }
                // If reloading, we can't do anything else (shoot/start reload)
                return; 
            }

            // Handle Reload Request
            if (weapon.wantsToReload) {
                if (weapon.currentAmmo < weapon.magSize && weapon.totalAmmo > 0) {
                     weapon.isReloading = true;
                     weapon.reloadTimer = weapon.reloadDuration;
                     // TODO: Play reload sound
                     return;
                }
            }

            // Handle Shooting
            if (canFire(weapon)) {
                // Start burst if applicable
                if (!weapon.isBursting && weapon.shotsPerBurst > 1) {
                    weapon.isBursting = true;
                    weapon.burstShotsFired = 0;
                }

                applyFiringEffects(weapon, fixedDeltaTime);

                spawnProjectiles(m_context, entity, weapon, bodyComp);

                weapon.currentAmmo--;
                
                if (weapon.isBursting) {
                    weapon.burstShotsFired++;
                    if (weapon.burstShotsFired >= weapon.shotsPerBurst) {
                        // Burst finished
                        weapon.isBursting = false;
                        weapon.burstShotsFired = 0;
                        weapon.cooldownTimer = weapon.burstCooldown;
                    } else {
                        // Continue burst
                        weapon.cooldownTimer = weapon.fireRate;
                    }
                } else {
                    // Normal fire
                    weapon.cooldownTimer = weapon.fireRate;
                }

            } else {
                 // Out of ammo logic (simplified auto-reload check if desired)
                 if (weapon.currentAmmo <= 0) {
                     weapon.isBursting = false;
                     weapon.burstShotsFired = 0;

                     if (weapon.wantsToShoot && weapon.cooldownTimer <= 0.0f && weapon.totalAmmo > 0) {
                         weapon.isReloading = true;
                         weapon.reloadTimer = weapon.reloadDuration;
                     }
                 }
            }
        });
    }

    bool WeaponSystem::canFire(const WeaponComponent& weapon) const {
        bool trigger = weapon.wantsToShoot || weapon.isBursting;
        return trigger &&
               weapon.cooldownTimer <= 0.0f &&
               weapon.currentAmmo > 0;
    }

    void WeaponSystem::applyFiringEffects(WeaponComponent& weapon, float dt) {
        // Accumulate Spread (bloom)
        weapon.currentSpreadDeg = std::min(
            weapon.currentSpreadDeg + weapon.spreadPerShotDeg,
            weapon.maxSpreadDeg
        );

        // Add Recoil Kick (Upwards direction relative to aim)
        // "Up" in world space is (0, -1) which is -PI/2 radians
        // We want to kick the angle towards -PI/2

        float currentAim = weapon.aimAngle;
        float targetUp = -PI / 2.0f;

        // Calculate shortest angular distance to "up"
        float diff = targetUp - currentAim;

        // Wrap to [-PI, PI]
        while (diff <= -PI) diff += 2 * PI;
        while (diff > PI) diff -= 2 * PI;

        float kickSign = 0.0f;
        if (std::abs(diff) > 0.001f) {
            // Standard case: kick towards up
            kickSign = (diff > 0) ? 1.0f : -1.0f;

            // Handle 180 degree edge case (aiming straight down)
            // If aim is close to PI/2 (down), diff is close to -PI or PI
            if (std::abs(std::abs(diff) - PI) < 0.1f) {
                 // Tie-breaker using aim vector x-component
                 float aimX = std::cos(currentAim);
                 if (aimX > 0) kickSign = -1.0f; 
                 else kickSign = 1.0f;
            }
        } else {
            // Already looking up, small jitter
             kickSign = (m_rng() % 2 == 0) ? 1.0f : -1.0f;
        }

        weapon.recoilAngleOffset += kickSign * weapon.recoilKickDeg * (PI / 180.0f);
    }

    void WeaponSystem::spawnProjectiles(engine::EngineContext& context, entt::entity entity, const WeaponComponent& weapon, const engine::PhysicsBodyComponent& bodyComp) {
        auto& registry = *context.m_registry;
        b2Vec2 bodyPos = b2Body_GetPosition(bodyComp.bodyId);
        
        // Loop for multiple projectiles
        for (int i = 0; i < weapon.projectilesPerShot; ++i) {
            // Apply Spread
            std::uniform_real_distribution<float> spreadDist(-weapon.currentSpreadDeg, weapon.currentSpreadDeg);
            
            float spreadOffsetDeg = spreadDist(m_rng); 
            float spreadOffsetRad = spreadOffsetDeg * (PI / 180.0f);
            float finalAngle = weapon.aimAngle + weapon.recoilAngleOffset + spreadOffsetRad;

            // Calculate spawn position (offset from center)
            float spawnOffset = 45.0f; // Increased from 30.0f to be further out
            float heightOffset = WEAPON_MUZZLE_HEIGHT_OFFSET; // Lift up to chest/gun level
            float cosA = std::cos(finalAngle);
            float sinA = std::sin(finalAngle);
            
            engine::Vector2f spawnPos = {
                bodyPos.x + cosA * spawnOffset,
                bodyPos.y + sinA * spawnOffset + heightOffset
            };

            // Create Projectile Entity
            auto projectile = registry.create();

            // Physics
            engine::PendingPhysicsBodyComponent pending{
                 .position = spawnPos,
                 .isStatic = false,
                 .isBullet = true,
                 .initialVelocity = { cosA * weapon.projectileSpeed, sinA * weapon.projectileSpeed },
                 .rotation = finalAngle
            };

            engine::FixtureDef fixDef;
            fixDef.size = { 10.0f, 10.0f };
            fixDef.density = 1.0f;
            fixDef.isSensor = false;
            fixDef.categoryBits = engine::PhysicsCategory::Projectile;
            // Collide with Walls and Hurtboxes (Player/Enemy sensors).
            // Exclude Enemy/Player hitboxes to rely solely on Hurtbox for damage.
            // Exclude LowObstacle to allow shooting over them.
            fixDef.maskBits = engine::PhysicsCategory::Wall | engine::PhysicsCategory::Hurtbox;
            
            pending.fixtures.push_back(fixDef);

            registry.emplace<engine::PendingPhysicsBodyComponent>(projectile, pending);

            // Visuals
            registry.emplace<engine::TransformComponent>(projectile, spawnPos, finalAngle);
            registry.emplace<engine::RenderableComponent>(projectile, "bullet_sprite", 2, sf::Color::White);

            // Game Logic
            registry.emplace<ProjectileComponent>(projectile, weapon.damage, entity);
            registry.emplace<DamageComponent>(projectile, weapon.damage);
            registry.emplace<engine::LifetimeComponent>(projectile, weapon.projectileLifetime);
        }
    }

} // namespace game
