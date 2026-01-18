#include "engine/pch.h"
#include "WeaponSwitchSystem.hpp"
#include "engine/core/EngineContext.hpp"
#include "engine/core/IInputManager.hpp"
#include "engine/core/ILogger.hpp"
#include "engine/core/ILoggerManager.hpp"
#include "engine/ecs/ArchetypeManager.hpp"
#include "engine/components/AnimationStateComponent.hpp"
#include "engine/components/AnimationComponent.hpp"
#include "game/components/WeaponInventoryComponent.hpp"
#include "game/components/WeaponComponent.hpp"
#include "game/components/PlayerComponent.hpp"
#include <SFML/Window/Keyboard.hpp>
#include <entt/entt.hpp>
#include <nlohmann/json.hpp>

namespace game {

    void WeaponSwitchSystem::update(engine::EngineContext& context, float dt) {
        // We iterate over entities that have an inventory, a weapon component, and an animation state.
        // Usually, this is the player or maybe smart AI.
        // For input-based switching, we specifically care about the one controlled by the player (implied by input check, but good to filter).
        
        auto view = context.m_registry->view<WeaponInventoryComponent, WeaponComponent, engine::AnimationStateComponent, PlayerComponent>();
        
        for (auto [entity, inv, weapon, animState, player] : view.each()) {
            
            // Decay switch cooldown
            if (inv.switchCooldown > 0.0f) {
                inv.switchCooldown -= dt;
                // If we're on cooldown, we can't switch.
                // However, we should continue decay for all entities.
                // But we shouldn't process input for this entity if it's on cooldown.
            }

            if (inv.switchCooldown > 0.0f) {
                continue;
            }
            
            // Check input (1-9 keys)
            int requestedSlot = getRequestedSlot(context);
            
            // Validate request
            if (requestedSlot != -1 && 
                requestedSlot != inv.activeSlot && 
                requestedSlot >= 0 && 
                requestedSlot < static_cast<int>(inv.weaponArchetypes.size())) {
                
                // Perform switch
                inv.activeSlot = requestedSlot;
                inv.switchCooldown = inv.switchDelay;
                
                // Load weapon stats from archetype
                std::string weaponArchetype = inv.weaponArchetypes[requestedSlot];

                loadWeaponStats(context, entity, weaponArchetype);
                
                // Update AnimationSetId (CORE VISUAL CHANGE)
                // e.g., "Rifle" -> uses rifle_idle, rifle_walk, etc. defined in your animation config
                animState.animationSetId = weaponArchetype;
                
                // Reset animation state to avoid glitches (like trying to play a rifle anim on a pistol skeleton if they differed, though here it's sprite based)
                if (auto* animComp = context.m_registry->try_get<engine::AnimationComponent>(entity)) {
                    animComp->reset();
                }
                
                auto logger = context.m_logManager->GetLogger("WeaponSwitchSystem");
                if (logger) {
                    logger->info("Switched to weapon slot {}: {}", requestedSlot, weaponArchetype);
                }
            }
        }
    }

    int WeaponSwitchSystem::getRequestedSlot(engine::EngineContext& context) const {
        // Check keys 1-9
        if (context.m_inputManager->isKeyPressed(engine::KeyCode::Num1)) return 0;
        if (context.m_inputManager->isKeyPressed(engine::KeyCode::Num2)) return 1;
        if (context.m_inputManager->isKeyPressed(engine::KeyCode::Num3)) return 2;
        if (context.m_inputManager->isKeyPressed(engine::KeyCode::Num4)) return 3;
        if (context.m_inputManager->isKeyPressed(engine::KeyCode::Num5)) return 4;
        if (context.m_inputManager->isKeyPressed(engine::KeyCode::Num6)) return 5;
        if (context.m_inputManager->isKeyPressed(engine::KeyCode::Num7)) return 6;
        if (context.m_inputManager->isKeyPressed(engine::KeyCode::Num8)) return 7;
        if (context.m_inputManager->isKeyPressed(engine::KeyCode::Num9)) return 8;
        
        return -1;
    }

    void WeaponSwitchSystem::loadWeaponStats(engine::EngineContext& context, entt::entity entity, const std::string& archetypeId) {
        if (!context.m_archetypeManager) return;

        const auto* archJson = context.m_archetypeManager->getArchetype(archetypeId);
        if (!archJson) {
             auto logger = context.m_logManager->GetLogger("WeaponSwitchSystem");
             if (logger) logger->error("Failed to find weapon archetype: {}", archetypeId);
             return;
        }

        // We need to extract the WeaponComponent part from the archetype JSON
        // The archetype JSON usually looks like: { "components": { "WeaponComponent": { ... } } }
        // BUT some archetypes might be defined as just the component if they are not full entities, 
        // OR the structure might differ. 
        // Based on archetypes.json, "TestRifle": { "Weapon": { ... } } 
        // Note: The key in the JSON is "Weapon", not "WeaponComponent" (based on archetypes.json inspection).
        // Let's check both or adjust based on archetypes.json.
        
        const nlohmann::json* weaponDataPtr = nullptr;

        if (archJson->contains("Weapon")) {
            weaponDataPtr = &(*archJson)["Weapon"];
        } else if (archJson->contains("components") && (*archJson)["components"].contains("Weapon")) {
            weaponDataPtr = &(*archJson)["components"]["Weapon"];
        }

        if (weaponDataPtr) {
            const auto& weaponData = *weaponDataPtr;
            
            auto& weapon = context.m_registry->get<WeaponComponent>(entity);
            
            // Copy stats manually or via reflection if you had it. 
            // Since we don't have full reflection deserialization here in this snippet, we do it manually based on fields.
            // Based on WeaponComponent.hpp:
            
            if (weaponData.contains("fireRate")) weapon.fireRate = weaponData["fireRate"];
            if (weaponData.contains("projectileSpeed")) weapon.projectileSpeed = weaponData["projectileSpeed"];
            if (weaponData.contains("projectileLifetime")) weapon.projectileLifetime = weaponData["projectileLifetime"];
            if (weaponData.contains("damage")) weapon.damage = weaponData["damage"];
            
            if (weaponData.contains("baseSpreadDeg")) weapon.baseSpreadDeg = weaponData["baseSpreadDeg"];
            if (weaponData.contains("spreadPerShotDeg")) weapon.spreadPerShotDeg = weaponData["spreadPerShotDeg"];
            if (weaponData.contains("spreadDecayDegPerSec")) weapon.spreadDecayDegPerSec = weaponData["spreadDecayDegPerSec"];
            if (weaponData.contains("maxSpreadDeg")) weapon.maxSpreadDeg = weaponData["maxSpreadDeg"];
            
            if (weaponData.contains("recoilKickDeg")) weapon.recoilKickDeg = weaponData["recoilKickDeg"];
            if (weaponData.contains("recoilReturnDegPerSec")) weapon.recoilReturnDegPerSec = weaponData["recoilReturnDegPerSec"];
            
            if (weaponData.contains("projectilesPerShot")) weapon.projectilesPerShot = weaponData["projectilesPerShot"];
            
            if (weaponData.contains("magSize")) {
                weapon.magSize = weaponData["magSize"];
                // Optionally reset current ammo on switch? Or keep it? 
                // Usually in simple games, getting a "new" gun fills it, but if it's inventory based, we might want to store state.
                // For now, let's reset the mag to full.
                weapon.currentAmmo = weapon.magSize; 
            }
            if (weaponData.contains("totalAmmo")) weapon.totalAmmo = weaponData["totalAmmo"];
            if (weaponData.contains("reloadDuration")) weapon.reloadDuration = weaponData["reloadDuration"];
            
            // Reset runtime states
            weapon.currentSpreadDeg = weapon.baseSpreadDeg;
            weapon.recoilAngleOffset = 0.0f;
            weapon.isReloading = false;
            weapon.reloadTimer = 0.0f;
            weapon.cooldownTimer = 0.0f;
            
        } else {
            auto logger = context.m_logManager->GetLogger("WeaponSwitchSystem");
            if (logger) logger->warn("Archetype {} has no WeaponComponent data!", archetypeId);
        }
    }

} // namespace game
