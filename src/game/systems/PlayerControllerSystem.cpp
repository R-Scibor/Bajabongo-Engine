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
#include "game/components/WeaponComponent.hpp"
#include "engine/components/AnimationComponent.hpp"
#include "engine/core/math/Vector2.hpp"

#include <box2d/box2d.h>
#include <cmath>

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

        auto view = registry.view<PlayerComponent, engine::PhysicsBodyComponent, engine::TransformComponent>();

        view.each([&](entt::entity entity, PlayerComponent& player, engine::PhysicsBodyComponent& bodyComp, engine::TransformComponent& transform) {
            if (!b2Body_IsValid(bodyComp.bodyId)) return;

            auto* weapon = registry.try_get<WeaponComponent>(entity);
            auto* anim = registry.try_get<engine::AnimationComponent>(entity);

            // --- Movement (WASD) ---
            engine::Vector2f moveDir{ 0.0f, 0.0f };

            if (input->isKeyPressed(engine::KeyCode::W)) moveDir.y -= 1.0f;
            if (input->isKeyPressed(engine::KeyCode::S)) moveDir.y += 1.0f;
            if (input->isKeyPressed(engine::KeyCode::A)) moveDir.x -= 1.0f;
            if (input->isKeyPressed(engine::KeyCode::D)) moveDir.x += 1.0f;

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
                // 1. Determine State: Idle vs Moving
                if (lengthSq > 0.0f) {
                    // Moving
                    std::string newClip = anim->currentClipId;
                    float scaleX = std::abs(transform.scale.x); // Preserve magnitude

                    // 2. Determine Direction
                    // Prioritize vertical movement for sprite selection if moving diagonally,
                    // or prioritize horizontal? Let's prioritize based on larger component.
                    if (std::abs(moveDir.y) > std::abs(moveDir.x)) {
                         if (moveDir.y < 0.0f) {
                            newClip = "player_walk_up";
                        } else {
                            newClip = "player_walk_down";
                        }
                    } else {
                        // Horizontal or equal
                        newClip = "player_walk_right";
                        if (moveDir.x < 0.0f) {
                            transform.scale.x = -scaleX; // Face Left
                        } else {
                            transform.scale.x = scaleX; // Face Right
                        }
                    }

                    // Switch clip if changed
                    if (anim->currentClipId != newClip) {
                        anim->currentClipId = newClip;
                        anim->reset();
                    }
                }
                else {
                    // Idle
                    if (anim->currentClipId != "player_idle") {
                        anim->currentClipId = "player_idle";
                        anim->reset();
                    }
                }
            }

            // Apply velocity
            b2Vec2 velocity = { moveDir.x * player.moveSpeed, moveDir.y * player.moveSpeed };

            // Wake up the body if we are trying to move it, otherwise it might sleep
            if (lengthSq > 0.0f) {
                b2Body_SetAwake(bodyComp.bodyId, true);
                b2Body_SetLinearVelocity(bodyComp.bodyId, velocity);
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
                float dy = mouseWorldPos.y - bodyPos.y;

                // atan2 returns angle in radians
                float angle = std::atan2(dy, dx);

                // Set rotation directly
                // b2Body_SetTransform(bodyComp.bodyId, bodyPos, b2MakeRot(angle));

                // --- Update Weapon State ---
                if (weapon)
                {
                    weapon->aimAngle = angle;
                    weapon->wantsToShoot = input->isMouseButtonPressed(engine::MouseCode::Left);
                    weapon->wantsToReload = input->isKeyPressed(engine::KeyCode::R);
                }
            }
        });
    }

} // namespace game