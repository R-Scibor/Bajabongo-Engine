#include "GameComponentRegistry.hpp"
#include "engine/core/EngineContext.hpp"
#include "engine/ecs/EntityFactory.hpp"

// Game Components
#include "game/components/PlayerComponent.hpp"
#include "game/components/HealthComponent.hpp"
#include "game/components/DamageComponent.hpp"
#include "game/components/WeaponComponent.hpp"
#include "game/components/PortalComponent.hpp"
#include "game/components/VisibilityComponent.hpp"
#include "game/components/VisibleToPlayerComponent.hpp"
#include "engine/components/WorldBoundsComponent.hpp"

// AI Components
#include "game/components/EnemyComponent.hpp"
#include "game/components/PatrolBehaviorComponent.hpp"
#include "game/components/RushBehaviorComponent.hpp"
#include "game/components/SniperBehaviorComponent.hpp"
#include "game/components/CampBehaviorComponent.hpp"
#include "game/components/TurretBehaviorComponent.hpp"

#include "engine/components/TransformComponent.hpp"
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

        // --- 6. Rejestracja VisibilityComponent ---
        factory.registerComponentLoader("Visibility",
            [](entt::registry& registry, entt::entity entity, const nlohmann::json& data) {
                VisibilityComponent visibility;
                visibility.viewRadius = data.value("viewRadius", 500.0f);
                visibility.minViewRadius = data.value("minViewRadius", 50.0f);
                visibility.viewAngle = data.value("viewAngle", 90.0f);
                visibility.viewDirection = data.value("viewDirection", 0.0f);
                
                if (data.contains("viewOffset")) {
                    auto offsetArr = data["viewOffset"];
                    if (offsetArr.is_array() && offsetArr.size() >= 2) {
                        visibility.offset.x = offsetArr[0];
                        visibility.offset.y = offsetArr[1];
                    }
                }

                registry.emplace<VisibilityComponent>(entity, visibility);
            }
        );

        // --- VisibleToPlayerComponent (Tag) ---
        // Doesn't strictly need a loader if it's only added at runtime, 
        // but useful if we want to force-make something visible in JSON
        factory.registerComponentLoader("VisibleToPlayer",
            [](entt::registry& registry, entt::entity entity, const nlohmann::json& data) {
                 registry.emplace<VisibleToPlayerComponent>(entity);
            }
        );

        // --- 7. Rejestracja WorldBoundsComponent ---
        factory.registerComponentLoader("WorldBounds",
            [](entt::registry& registry, entt::entity entity, const nlohmann::json& data) {
                float w = data.value("width", 0.0f);
                float h = data.value("height", 0.0f);
                
                registry.emplace<engine::WorldBoundsComponent>(entity, w, h);
            }
        );

        // --- 8. Rejestracja EnemyComponent ---
        factory.registerComponentLoader("Enemy", [](entt::registry& registry, entt::entity entity, const nlohmann::json& data) {
            EnemyComponent enemy;
            enemy.detectionRadius = data.value("detectionRadius", 400.0f);
            enemy.alertRadius = data.value("alertRadius", 500.0f);
            enemy.attackRange = data.value("attackRange", 300.0f);
            enemy.moveSpeed = data.value("moveSpeed", 80.0f);
            enemy.retreatHpPercent = data.value("retreatHpPercent", 0.3f);
            
            // Initialize stuck detection tracking
            if (engine::TransformComponent* transform = registry.try_get<engine::TransformComponent>(entity)) {
                enemy.lastValidPosition = transform->position;
                enemy.lastCheckedPosition = transform->position;
            }
            
            enemy.stunTimer = 0.0f; // Ensure initialized
            
            registry.emplace<EnemyComponent>(entity, enemy);
        });

        // --- 9. Rejestracja PatrolBehaviorComponent ---
        factory.registerComponentLoader("PatrolBehavior", [](entt::registry& registry, entt::entity entity, const nlohmann::json& data) {
            PatrolBehaviorComponent patrol;
            
            if (data.contains("waypoints") && data["waypoints"].is_array()) {
                for (const auto& wp : data["waypoints"]) {
                    if (wp.is_array() && wp.size() >= 2) {
                        patrol.waypoints.push_back({wp[0], wp[1]});
                    }
                }
            }
            
            patrol.waypointReachThreshold = data.value("waypointReachThreshold", 20.0f);
            patrol.loopMode = data.value("loopMode", "loop") == "loop";
            
            registry.emplace<PatrolBehaviorComponent>(entity, patrol);
        });

        // --- 10. Rejestracja RushBehaviorComponent ---
        factory.registerComponentLoader("RushBehavior", [](entt::registry& registry, entt::entity entity, const nlohmann::json& data) {
            RushBehaviorComponent rush;
            rush.rushSpeed = data.value("rushSpeed", 250.0f);
            rush.rushActivationRange = data.value("rushActivationRange", 150.0f);
            registry.emplace<RushBehaviorComponent>(entity, rush);
        });

        // --- 11. Rejestracja SniperBehaviorComponent ---
        factory.registerComponentLoader("SniperBehavior", [](entt::registry& registry, entt::entity entity, const nlohmann::json& data) {
            SniperBehaviorComponent sniper;
            sniper.aimDuration = data.value("aimDuration", 1.5f);
            sniper.maxRange = data.value("maxRange", 700.0f);
            registry.emplace<SniperBehaviorComponent>(entity, sniper);
        });

        // --- 12. Rejestracja CampBehaviorComponent ---
        factory.registerComponentLoader("CampBehavior", [](entt::registry& registry, entt::entity entity, const nlohmann::json& data) {
            CampBehaviorComponent camp;
            camp.campDuration = data.value("campDuration", 15.0f);
            camp.campZoneRadius = data.value("campZoneRadius", 200.0f);
            registry.emplace<CampBehaviorComponent>(entity, camp);
        });

        // --- 13. Rejestracja TurretBehaviorComponent ---
        factory.registerComponentLoader("TurretBehavior", [](entt::registry& registry, entt::entity entity, const nlohmann::json& data) {
            TurretBehaviorComponent turret;
            turret.rotationSpeed = data.value("rotationSpeed", 3.0f);
            turret.shootAngleTolerance = data.value("shootAngleTolerance", 0.1f);
            registry.emplace<TurretBehaviorComponent>(entity, turret);
        });
    }
}