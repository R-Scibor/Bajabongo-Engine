#include "GameComponentRegistry.hpp"
#include "engine/core/EngineContext.hpp"
#include "engine/ecs/EntityFactory.hpp"

// Game Components
#include "game/components/PlayerComponent.hpp"
#include "game/components/HealthComponent.hpp"
#include "game/components/WeaponComponent.hpp"

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
                
                registry.emplace<WeaponComponent>(entity, weapon);
            }
        );
    }
}