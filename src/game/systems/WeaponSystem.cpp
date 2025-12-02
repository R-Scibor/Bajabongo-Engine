#include "engine/pch.h"
#include "WeaponSystem.hpp"
#include "game/components/WeaponComponent.hpp"
#include "engine/core/EngineContext.hpp"
#include <entt/entt.hpp>
#include "engine/core/ILogger.hpp"
#include "engine/core/ILoggerManager.hpp"

namespace game {

    WeaponSystem::WeaponSystem(engine::EngineContext& context)
        : m_context(context)
    {
    }

    void WeaponSystem::update(float fixedDeltaTime)
    {
        auto& registry = *m_context.m_registry;
        auto view = registry.view<WeaponComponent>();

        view.each([&](WeaponComponent& weapon) {
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
                    
                    if (m_context.m_logManager) {
                         auto logger = m_context.m_logManager->GetLogger("WeaponSystem");
                         if (logger) logger->debug("Reload complete. Ammo: {}/{}", weapon.currentAmmo, weapon.totalAmmo);
                    }
                }
            }
        });
    }

} // namespace game