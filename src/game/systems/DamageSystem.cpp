#include "engine/pch.h"
#include "DamageSystem.hpp"
#include "game/components/DamageComponent.hpp"
#include "game/components/HealthComponent.hpp"
#include "engine/components/TransformComponent.hpp"
#include "engine/components/RenderableComponent.hpp"
#include "engine/components/AnimationStateComponent.hpp"
#include "engine/components/AnimationComponent.hpp"
#include "engine/components/MetaComponent.hpp"
#include "game/components/DeadBodyComponent.hpp"

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
                
                // Spawn Dead Body
                spawnDeadBody(target, registry);

                // Destroy the original entity
                registry.destroy(target);
            }
        }

        // 3. Destroy the projectile (because it hit something solid)
        registry.destroy(projectile);
    }

    void DamageSystem::spawnDeadBody(entt::entity originalEntity, entt::registry& registry) {
        auto* transform = registry.try_get<engine::TransformComponent>(originalEntity);
        auto* renderable = registry.try_get<engine::RenderableComponent>(originalEntity);
        auto* animState = registry.try_get<engine::AnimationStateComponent>(originalEntity);

        if (transform && renderable && animState) {
            auto deadBody = registry.create();

            // Copy Transform
            registry.emplace<engine::TransformComponent>(deadBody, *transform);

            // Copy Renderable
            auto& newRenderable = registry.emplace<engine::RenderableComponent>(deadBody, *renderable);
            // Ensure dead bodies are rendered below living entities if needed, or same layer
            newRenderable.layer = 0; // Move to background layer? Or keep same? Let's try layer 0 for floor.

            // Setup Animation State
            engine::AnimationStateComponent newAnimState;
            newAnimState.animationSetId = animState->animationSetId;
            newAnimState.state = engine::AnimationState::Dead;
            newAnimState.facing = animState->facing;
            // newAnimState.commitState(); // REMOVED: Do not commit state, let the system detect the change!
            registry.emplace<engine::AnimationStateComponent>(deadBody, newAnimState);

            // Setup Animation Component (needed for the system to process it)
            registry.emplace<engine::AnimationComponent>(deadBody);

            // Copy MetaComponent if it exists (for fallback logic in AnimationStateMachine)
            if (auto* meta = registry.try_get<engine::MetaComponent>(originalEntity)) {
                registry.emplace<engine::MetaComponent>(deadBody, *meta);
            }
            
            // --- DEBUG: Verify AnimationSetId ---
            if (m_logger) {
                m_logger->info("Spawned DeadBody {}. AnimationSetId: '{}'. Original ID: '{}'", 
                    entt::to_integral(deadBody), newAnimState.animationSetId, animState->animationSetId);
            }

            // Mark as DeadBody
            registry.emplace<DeadBodyComponent>(deadBody);

             if (m_logger) {
                m_logger->debug("Spawned DeadBody entity {} for original entity {}", entt::to_integral(deadBody), entt::to_integral(originalEntity));
            }
        }
    }

}