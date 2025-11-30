#include "engine/pch.h"
#include "DamageSystem.hpp"
#include "game/components/DamageComponent.hpp"
#include "game/components/HealthComponent.hpp"

#include "engine/core/EngineContext.hpp"
#include "engine/core/ILoggerManager.hpp"
#include "engine/core/ILogger.hpp"

#include <entt/entt.hpp>

namespace game {

    DamageSystem::DamageSystem(engine::EngineContext& context)
        : m_context(context)
    {
        m_logger = context.m_logManager->GetLogger("DamageSystem");
        
        // Subscribe to events
        context.m_dispatcher->sink<engine::PhysicsSensorBeginEvent>().connect<&DamageSystem::onSensorBegin>(this);
        
        if (m_logger) {
            m_logger->info("DamageSystem initialized.");
        }
    }

    DamageSystem::~DamageSystem() {
        m_context.m_dispatcher->sink<engine::PhysicsSensorBeginEvent>().disconnect(this);
    }

    void DamageSystem::onSensorBegin(const engine::PhysicsSensorBeginEvent& event) {
        auto& registry = *m_context.m_registry;

        entt::entity sensor = event.sensorEntity;
        entt::entity visitor = event.visitorEntity;
        bool isVisitorSensor = event.isVisitorSensor;

        // Check if sensor is Projectile
        if (registry.all_of<DamageComponent>(sensor)) {
            // Sensor is Projectile, Visitor is Target
            // We know 'isVisitorSensor' tells us if the target is a sensor.
            handleCollision(sensor, visitor, isVisitorSensor, registry);
        }
        
        // Check if visitor is Projectile
        if (registry.all_of<DamageComponent>(visitor)) {
            // Visitor is Projectile, Sensor is Target.
            // Since this event is "SensorBegin", the 'sensor' entity definitely has a sensor shape.
            // So isTargetSensor = true.
            handleCollision(visitor, sensor, true, registry);
        }
    }

    void DamageSystem::handleCollision(entt::entity projectile, entt::entity target, bool isTargetSensor, entt::registry& registry) {
        if (!registry.valid(projectile) || !registry.valid(target)) return;

        // 1. If the target is a Sensor (Trigger), do not collide/destroy.
        if (isTargetSensor) {
            return;
        }

        // 2. Apply Damage (if target has Health)
        auto* damageComp = registry.try_get<DamageComponent>(projectile);
        auto* healthComp = registry.try_get<HealthComponent>(target);

        if (damageComp && healthComp) {
            healthComp->currentHp -= damageComp->damageValue;
            
            if (m_logger) {
                m_logger->debug("Entity {} hit Entity {}. Damage: {}. Remaining HP: {}",
                    entt::to_integral(projectile), entt::to_integral(target), damageComp->damageValue, healthComp->currentHp);
            }

            if (healthComp->currentHp <= 0.0f) {
                if (m_logger) {
                    m_logger->info("Entity {} destroyed (HP <= 0).", entt::to_integral(target));
                }
                registry.destroy(target);
            }
        }

        // 3. Destroy the projectile (because it hit something solid)
        registry.destroy(projectile);
    }

}