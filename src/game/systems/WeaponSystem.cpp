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

#include <entt/entt.hpp>
#include <box2d/box2d.h>
#include <cmath>

namespace game {

    WeaponSystem::WeaponSystem(engine::EngineContext& context)
        : m_context(context)
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
                    float angle = weapon.aimAngle;

                    // Calculate spawn position (offset from center)
                    float spawnOffset = 30.0f; // Adjust based on entity size?
                    float cosA = std::cos(angle);
                    float sinA = std::sin(angle);
                    
                    engine::Vector2f spawnPos = {
                        bodyPos.x + cosA * spawnOffset,
                        bodyPos.y + sinA * spawnOffset
                    };

                    // Create Projectile Entity
                    auto projectile = registry.create();

                    // Physics
                    registry.emplace<engine::PendingPhysicsBodyComponent>(
                        projectile,
                        spawnPos,
                        engine::Vector2f{ 10.0f, 10.0f }, // Size
                        false, // Dynamic
                        1.0f,  // Density
                        true,  // isSensor
                        false, // fixedRotation
                        0.0f,   // damping
                        true,   // isBullet
                        engine::Vector2f{ cosA * weapon.projectileSpeed, sinA * weapon.projectileSpeed }, // initialVelocity
                        angle // rotation
                    );

                    // Visuals
                    registry.emplace<engine::TransformComponent>(projectile, spawnPos, angle);
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