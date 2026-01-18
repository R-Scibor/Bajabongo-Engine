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
            if (weapon.wantsToShoot && weapon.cooldownTimer <= 0.0f) {
                if (weapon.currentAmmo > 0) {
                    // Fire!
                    weapon.cooldownTimer = weapon.fireRate;
                    weapon.currentAmmo--;

                    b2Vec2 bodyPos = b2Body_GetPosition(bodyComp.bodyId);
                    
                    // Apply Spread
                    std::uniform_real_distribution<float> spreadDist(-weapon.currentSpreadDeg, weapon.currentSpreadDeg);
                    float spreadOffsetDeg = spreadDist(m_rng);
                    float spreadOffsetRad = spreadOffsetDeg * (PI / 180.0f);
                    float finalAngle = weapon.aimAngle + spreadOffsetRad;

                    // Accumulate Spread (bloom)
                    weapon.currentSpreadDeg = std::min(
                        weapon.currentSpreadDeg + weapon.spreadPerShotDeg,
                        weapon.maxSpreadDeg
                    );

                    // Debug Log Spread
                    // if (m_context.m_logManager) {
                    //     auto logger = m_context.m_logManager->GetLogger("WeaponSystem");
                    //     if (logger) logger->debug("Fired. Spread: {:.2f}", weapon.currentSpreadDeg);
                    // }

                    // Calculate spawn position (offset from center)
                    float spawnOffset = 30.0f; // Adjust based on entity size?
                    float cosA = std::cos(finalAngle);
                    float sinA = std::sin(finalAngle);
                    
                    engine::Vector2f spawnPos = {
                        bodyPos.x + cosA * spawnOffset,
                        bodyPos.y + sinA * spawnOffset
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
                    fixDef.isSensor = true;
                    fixDef.categoryBits = engine::PhysicsCategory::Projectile;
                    // Collide with Walls, Enemies, and PlayerHurtbox (if needed, but usually enemies shoot players)
                    fixDef.maskBits = engine::PhysicsCategory::Wall | engine::PhysicsCategory::Enemy | engine::PhysicsCategory::Hurtbox;
                    
                    pending.fixtures.push_back(fixDef);

                    registry.emplace<engine::PendingPhysicsBodyComponent>(projectile, pending);

                    // Visuals
                    registry.emplace<engine::TransformComponent>(projectile, spawnPos, finalAngle);
                    registry.emplace<engine::RenderableComponent>(projectile, "bullet_sprite", 2, sf::Color::White);

                    // Game Logic
                    registry.emplace<ProjectileComponent>(projectile, weapon.damage);
                    registry.emplace<DamageComponent>(projectile, weapon.damage);
                    registry.emplace<engine::LifetimeComponent>(projectile, weapon.projectileLifetime);

                } else {
                     // Out of ammo
                     // If auto-reload logic is desired, it could go here. 
                     // For now, just ensuring we don't fire.
                     
                     // Simple auto-reload trigger if empty and trying to shoot
                     if (weapon.totalAmmo > 0) {
                         weapon.isReloading = true;
                         weapon.reloadTimer = weapon.reloadDuration;
                     }
                }
            }
        });
    }

} // namespace game