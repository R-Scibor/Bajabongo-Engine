#include "engine/pch.h"
#include "DamageSystem.hpp"
#include "game/components/DamageComponent.hpp"
#include "game/components/ProjectileComponent.hpp"
#include "game/components/HealthComponent.hpp"
#include "engine/components/TransformComponent.hpp"
#include "engine/components/RenderableComponent.hpp"
#include "engine/components/AnimationStateComponent.hpp"
#include "engine/components/AnimationComponent.hpp"
#include "engine/components/MetaComponent.hpp"
#include "game/components/DeadBodyComponent.hpp"
#include "game/components/HitFlashComponent.hpp"
#include "game/components/PlayerComponent.hpp"
#include "game/components/EnemyComponent.hpp"
#include "engine/events/AudioEvents.hpp"

#include "engine/core/EngineContext.hpp"
#include "engine/core/ILoggerManager.hpp"
#include "engine/core/ILogger.hpp"

#include <entt/entt.hpp>
#include <cstdlib>

namespace game {

    DamageSystem::DamageSystem(engine::EngineContext& context)
        : m_context(context)
    {
        m_logger = context.m_logManager->GetLogger("DamageSystem");
        
        // Subscribe to events
        context.m_dispatcher->sink<engine::PhysicsSensorBeginEvent>().connect<&DamageSystem::onSensorBegin>(this);
        context.m_dispatcher->sink<engine::PhysicsContactBeginEvent>().connect<&DamageSystem::onContactBegin>(this);
        
        if (m_logger) {
            m_logger->info("DamageSystem initialized.");
        }
    }

    DamageSystem::~DamageSystem() {
        m_context.m_dispatcher->sink<engine::PhysicsSensorBeginEvent>().disconnect(this);
        m_context.m_dispatcher->sink<engine::PhysicsContactBeginEvent>().disconnect(this);
    }

    void DamageSystem::onContactBegin(const engine::PhysicsContactBeginEvent& event) {
        auto& registry = *m_context.m_registry;
        entt::entity entityA = event.entityA;
        entt::entity entityB = event.entityB;

        if (!registry.valid(entityA) || !registry.valid(entityB)) return;

        // Check if A is projectile (and destroy it on contact with solid)
        if (registry.all_of<ProjectileComponent>(entityA)) {
            registry.destroy(entityA);
        }
        
        // Check if B is projectile
        else if (registry.all_of<ProjectileComponent>(entityB)) {
            registry.destroy(entityB);
        }
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

        // Check ownership to prevent self-damage
        if (auto* projComp = registry.try_get<ProjectileComponent>(projectile)) {
            if (projComp->owner == target) {
                return;
            }
        }

        // 1. Attempt to apply damage first (Handles Hurtbox sensors)
        auto* damageComp = registry.try_get<DamageComponent>(projectile);
        auto* healthComp = registry.try_get<HealthComponent>(target);

        if (damageComp && healthComp) {
            healthComp->currentHp -= damageComp->damageValue;
            
            // Trigger Hit Flash
            if (auto* flashComp = registry.try_get<HitFlashComponent>(target)) {
                if (auto* renderable = registry.try_get<engine::RenderableComponent>(target)) {
                    // Only store original color if we are NOT currently flashing.
                    // If we are flashing, renderable->color is modified, so we keep the existing originalColor.
                    if (!flashComp->isFlashing) {
                        flashComp->originalColor = renderable->color;
                    }

                    // Start flash
                    renderable->color = flashComp->flashColor;
                    flashComp->currentTimer = 0.0f;
                    flashComp->isFlashing = true;
                }
            }

            if (m_logger) {
                m_logger->debug("Entity {} hit Entity {}. Damage: {}. Remaining HP: {}",
                    entt::to_integral(projectile), entt::to_integral(target), damageComp->damageValue, healthComp->currentHp);
            }

            if (healthComp->currentHp <= 0.0f) {
                if (m_logger) {
                    m_logger->info("Entity {} destroyed (HP <= 0).", entt::to_integral(target));
                }

                // Play Death Sound
                std::string soundId;

                // Try to get position for spatial audio
                engine::Vector2f pos = {0.0f, 0.0f};
                bool hasPos = false;
                if (auto* tc = registry.try_get<engine::TransformComponent>(target)) {
                    pos = tc->position;
                    hasPos = true;
                }

                if (registry.all_of<PlayerComponent>(target)) {
                    soundId = "death_player";
                    
                    // Also play scav laugh on player death
                    m_context.m_dispatcher->enqueue<engine::PlaySoundEvent>({
                        .id = "death_scav_laugh",
                        .volume = 100.0f,
                        .pitch = 1.0f,
                        .loop = false,
                        .sourceEntity = target,
                        .position = hasPos ? std::make_optional(pos) : std::nullopt,
                        .minDistance = 500.0f,
                        .attenuation = 2.0f
                    });

                } else if (registry.all_of<EnemyComponent>(target)) {
                    int roll = std::rand() % 100;
                    if (roll < 5) {
                        soundId = "death_amogus";
                    } else if (roll < 10) {
                        // 5% chance (5 to 9)
                        soundId = "death_lego_yoda";
                    } else if (roll < 20) {
                        // 10% chance (10 to 19)
                        soundId = "death_scav";
                    } else {
                        // Remaining 80% (20 to 99)
                        soundId = (std::rand() % 2 == 0) ? "death_1" : "death_2";
                    }
                }

                if (!soundId.empty()) {
                    m_context.m_dispatcher->enqueue<engine::PlaySoundEvent>({
                        .id = soundId,
                        .volume = 100.0f,
                        .pitch = 1.0f,
                        .loop = false,
                        .sourceEntity = target,
                        .position = hasPos ? std::make_optional(pos) : std::nullopt,
                        .minDistance = 500.0f,
                        .attenuation = 2.0f
                    });
                }
                
                // Spawn Dead Body
                spawnDeadBody(target, registry);

                // Destroy the original entity
                registry.destroy(target);
            }

            // Destroy the projectile after dealing damage
            registry.destroy(projectile);
            return;
        }

        // 2. If no damage was dealt:
        // If the target is a Sensor (e.g. Pickup, Zone) and we didn't damage it (no health),
        // then the projectile should pass through.
        if (isTargetSensor) {
            return;
        }

        // 3. If target was solid (Wall/Obstacle) and not a sensor, destroy the projectile
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
            newRenderable.color = sf::Color::White; // Reset color (in case of flash)

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