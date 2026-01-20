#include "engine/pch.h"
#include "PlayerControllerSystem.hpp"

#include <entt/entt.hpp>
#include "engine/core/EngineContext.hpp"
#include "engine/core/IInputManager.hpp"
#include "engine/core/input/MouseCode.hpp"
#include "engine/rendering/IRenderer.hpp"
#include "engine/components/PhysicsBodyComponent.hpp"
#include "engine/components/TransformComponent.hpp"
#include "engine/components/RenderableComponent.hpp"
#include "engine/components/LifetimeComponent.hpp"
#include "game/components/PlayerComponent.hpp"
#include "game/components/HealthComponent.hpp"
#include "game/components/WeaponComponent.hpp"
#include "game/components/VisibilityComponent.hpp"
#include "game/components/InteractableComponent.hpp"
#include "engine/events/StateEvents.hpp"
#include "engine/components/AnimationComponent.hpp"
#include "engine/components/AnimationStateComponent.hpp"
#include "engine/core/math/Vector2.hpp"

#include <box2d/box2d.h>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

constexpr float PI_F = static_cast<float>(M_PI);

namespace game {

    PlayerControllerSystem::PlayerControllerSystem(engine::EngineContext& context)
        : m_context(context)
    {
    }

    void PlayerControllerSystem::update(float fixedDeltaTime)
    {
        auto& registry = *m_context.m_registry;
        auto input = m_context.m_inputManager;
        auto renderer = m_context.m_renderer;

        auto view = registry.view<PlayerComponent, engine::PhysicsBodyComponent, engine::TransformComponent, VisibilityComponent>();

        view.each([&](entt::entity entity, PlayerComponent& player, engine::PhysicsBodyComponent& bodyComp, engine::TransformComponent& transform, VisibilityComponent& visibility) {
            if (!b2Body_IsValid(bodyComp.bodyId)) return;

            auto* weapon = registry.try_get<WeaponComponent>(entity);
            auto* anim = registry.try_get<engine::AnimationComponent>(entity);
            auto* health = registry.try_get<HealthComponent>(entity);

            // --- Healing Logic ---
            if (input->isKeyPressed(engine::KeyCode::H) && !player.isHealing && health && player.medkits > 0) {
                if (health->currentHp > 0 && health->currentHp < health->maxHp) {
                    player.isHealing = true;
                    player.healTimer = 6.0f;
                    player.medkits--;
                }
            }

            // --- Interaction Logic (E) ---
            if (input->isKeyPressed(engine::KeyCode::E)) {
                auto interactables = registry.view<InteractableComponent, engine::TransformComponent>();
                for (auto [targetEntity, interactable, targetTransform] : interactables.each()) {
                    float dx = transform.position.x - targetTransform.position.x;
                    float dy = transform.position.y - targetTransform.position.y;
                    float distSq = dx * dx + dy * dy;

                    if (distSq <= interactable.interactionRadius * interactable.interactionRadius) {
                        if (interactable.type == InteractableComponent::Type::MissionTable) {
                             // Swap to MapSelectionState to ensure current scene is cleared
                             m_context.m_dispatcher->enqueue<engine::RequestStateSwapEvent>("MapSelection");
                             break; // Only interact with one thing at a time
                        }
                    }
                }
            }

            if (player.isHealing) {
                player.healTimer -= fixedDeltaTime;
                if (player.healTimer <= 0.0f) {
                    player.isHealing = false;
                    if (health) {
                        health->currentHp = std::min(health->currentHp + 25.0f, health->maxHp);
                    }
                }
            }

            // --- Movement (WASD) ---
            engine::Vector2f moveDir{ 0.0f, 0.0f };

            if (!player.isHealing) {
                if (input->isKeyPressed(engine::KeyCode::W)) moveDir.y -= 1.0f;
                if (input->isKeyPressed(engine::KeyCode::S)) moveDir.y += 1.0f;
                if (input->isKeyPressed(engine::KeyCode::A)) moveDir.x -= 1.0f;
                if (input->isKeyPressed(engine::KeyCode::D)) moveDir.x += 1.0f;
            }

            // Normalize vector if moving diagonally
            float lengthSq = moveDir.x * moveDir.x + moveDir.y * moveDir.y;
            if (lengthSq > 0.0f)
            {
                float length = std::sqrt(lengthSq);
                moveDir.x /= length;
                moveDir.y /= length;
            }

            // --- Animation State Machine ---
            if (anim) {
                // We rely on AnimationStateComponent now.
                // If it's missing, the entity won't animate correctly (which is expected if we want to enforce the new system).
                if (auto* animState = registry.try_get<engine::AnimationStateComponent>(entity)) {
                    if (player.isHealing) {
                        animState->state = engine::AnimationState::Heal;
                    } else {
                        // 1. Determine State: Idle vs Moving vs Aiming
                        if (lengthSq > 0.0f) {
                            animState->state = engine::AnimationState::Walk;
                        } else if (weapon && weapon->wantsToShoot) {
                            animState->state = engine::AnimationState::Aim;
                        } else {
                            animState->state = engine::AnimationState::Idle;
                        }
                    }
                    
                    // 2. Determine Facing Direction (Left/Right only)
                    if (moveDir.x < 0.0f) {
                        animState->facing = engine::FacingDirection::Left;
                    } else if (moveDir.x > 0.0f) {
                        animState->facing = engine::FacingDirection::Right;
                    } else if ((lengthSq < 0.001f || player.isHealing) && weapon) {
                        // If standing still (or healing), face the aim direction
                        // Ideally we should do this if we are aiming
                        float aimAngle = weapon->aimAngle;
                        // Normalize angle to -PI to PI
                        while (aimAngle > PI_F) aimAngle -= 2.0f * PI_F;
                        while (aimAngle < -PI_F) aimAngle += 2.0f * PI_F;

                        if (std::abs(aimAngle) > PI_F / 2.0f) {
                            animState->facing = engine::FacingDirection::Left;
                        } else {
                            animState->facing = engine::FacingDirection::Right;
                        }
                    }
                }
            }

            // Apply velocity
            if (player.isHealing) {
                 b2Body_SetLinearVelocity(bodyComp.bodyId, { 0.0f, 0.0f });
            } else {
                b2Vec2 velocity = { moveDir.x * player.moveSpeed, moveDir.y * player.moveSpeed };

                // Wake up the body if we are trying to move it, otherwise it might sleep
                if (lengthSq > 0.0f) {
                    b2Body_SetAwake(bodyComp.bodyId, true);
                    b2Body_SetLinearVelocity(bodyComp.bodyId, velocity);
                }
            }


            // --- Rotation (Mouse) ---
            if (renderer)
            {
                engine::Vector2i mouseScreenPos = input->getMousePosition();

                // Need to cast to float for screenToWorld
                engine::Vector2f mouseWorldPos = renderer->screenToWorld(
                    engine::Vector2f{ static_cast<float>(mouseScreenPos.x), static_cast<float>(mouseScreenPos.y) }
                );

                b2Vec2 bodyPos = b2Body_GetPosition(bodyComp.bodyId);

                float dx = mouseWorldPos.x - bodyPos.x;
                // Adjust aim origin to match weapon muzzle height to ensure accuracy
                float dy = mouseWorldPos.y - (bodyPos.y + WEAPON_MUZZLE_HEIGHT_OFFSET);

                // atan2 returns angle in radians
                float angle = std::atan2(dy, dx);

                // Set rotation directly
                // b2Body_SetTransform(bodyComp.bodyId, bodyPos, b2MakeRot(angle));

                // NEW: Update visibility direction with smooth interpolation
                visibility.targetViewDirection = angle;
                
                // Shortest path interpolation for angle
                float deltaAngle = angle - visibility.viewDirection;
                if (deltaAngle > PI_F) deltaAngle -= 2.0f * PI_F;
                if (deltaAngle < -PI_F) deltaAngle += 2.0f * PI_F;
                
                visibility.viewDirection += deltaAngle * visibility.viewInterpSpeed * fixedDeltaTime;

                // Wrap angle to keep it clean (optional)
                if (visibility.viewDirection > PI_F) visibility.viewDirection -= 2.0f * PI_F;
                if (visibility.viewDirection < -PI_F) visibility.viewDirection += 2.0f * PI_F;

                // --- Update Weapon State ---
                if (weapon)
                {
                    weapon->aimAngle = angle;
                    if (player.isHealing) {
                        weapon->wantsToShoot = false;
                        weapon->wantsToReload = false;
                    } else {
                        weapon->wantsToShoot = input->isMouseButtonPressed(engine::MouseCode::Left);
                        weapon->wantsToReload = input->isKeyPressed(engine::KeyCode::R);
                    }
                }
            }
        });
    }

} // namespace game