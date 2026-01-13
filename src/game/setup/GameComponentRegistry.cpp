#include "GameComponentRegistry.hpp"
#include "engine/core/EngineContext.hpp"
#include "engine/ecs/EntityFactory.hpp"

// Game Components
#include "game/components/PlayerComponent.hpp"
#include "game/components/HealthComponent.hpp"
#include "game/components/DamageComponent.hpp"
#include "game/components/WeaponComponent.hpp"
#include "game/components/PortalComponent.hpp"

#include <entt/entt.hpp>
#include <nlohmann/json.hpp>

namespace game {

    void RegisterGameComponents(engine::EngineContext& context) {
        if (!context.m_entityFactory) return;

        auto& factory = *context.m_entityFactory;

        // --- 1. Rejestracja PlayerComponent ---
        factory.registerComponentLoader("Player", 
            [](entt::registry& registry, entt::entity entity, const nlohmann::json& data) {
                PlayerComponent playerComp;
                playerComp.moveSpeed = data.value("moveSpeed", 5.0f);
                playerComp.rotSpeed = data.value("rotSpeed", 10.0f);
                
                registry.emplace<PlayerComponent>(entity, playerComp);
            }
        );

        // --- 2. Rejestracja HealthComponent ---
        factory.registerComponentLoader("Health", 
            [](entt::registry& registry, entt::entity entity, const nlohmann::json& data) {
                float hp = data.value("maxHp", 100.0f);
                
                HealthComponent healthComp;
                healthComp.maxHp = hp;
                healthComp.currentHp = hp;

                registry.emplace<HealthComponent>(entity, healthComp);
            }
        );

        // --- 3. Rejestracja WeaponComponent ---
        factory.registerComponentLoader("Weapon",
            [](entt::registry& registry, entt::entity entity, const nlohmann::json& data) {
                WeaponComponent weapon;
                weapon.fireRate = data.value("fireRate", 0.2f);
                weapon.damage = data.value("damage", 10.0f);
                weapon.projectileSpeed = data.value("projectileSpeed", 300.0f);
                weapon.projectileLifetime = data.value("projectileLifetime", 2.0f);
                weapon.cooldownTimer = data.value("cooldownTimer", 0.0f);
                
                // Ammo & Reload
                weapon.magSize = data.value("magSize", 30);
                weapon.currentAmmo = data.value("currentAmmo", weapon.magSize); // Default to full mag
                weapon.totalAmmo = data.value("totalAmmo", 90);
                weapon.reloadDuration = data.value("reloadDuration", 2.0f);
                weapon.isReloading = false;
                weapon.reloadTimer = 0.0f;

                registry.emplace<WeaponComponent>(entity, weapon);
            }
        );

        // --- 4. Rejestracja DamageComponent ---
        factory.registerComponentLoader("Damage",
            [](entt::registry& registry, entt::entity entity, const nlohmann::json& data) {
                DamageComponent damage;
                damage.damageValue = data.value("damageValue", 10.0f);
                
                registry.emplace<DamageComponent>(entity, damage);
            }
        );

        // --- 5. Rejestracja PortalComponent ---
        factory.registerComponentLoader("Portal",
            [](entt::registry& registry, entt::entity entity, const nlohmann::json& data) {
                PortalComponent portal;
                portal.isOpen = data.value("isOpen", false);
                portal.isLocked = data.value("isLocked", false);
                
                registry.emplace<PortalComponent>(entity, portal);
            }
        );
    }
}