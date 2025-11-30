#include "engine/pch.h"
#include "PlayerControllerSystem.hpp"

#include <entt/entt.hpp>
#include "engine/core/EngineContext.hpp"
#include "engine/core/IInputManager.hpp"
#include "engine/core/input/MouseCode.hpp"
#include "engine/rendering/IRenderer.hpp"
#include "engine/components/PhysicsBodyComponent.hpp"
#include "engine/components/PendingPhysicsBodyComponent.hpp"
#include "engine/components/TransformComponent.hpp"
#include "engine/components/RenderableComponent.hpp"
#include "engine/components/LifetimeComponent.hpp"
#include "game/components/PlayerComponent.hpp"
#include "game/components/WeaponComponent.hpp"
#include "game/components/DamageComponent.hpp"
#include "game/components/ProjectileComponent.hpp"
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

        auto view = registry.view<PlayerComponent, engine::PhysicsBodyComponent>();

        view.each([&](entt::entity entity, PlayerComponent& player, engine::PhysicsBodyComponent& bodyComp) {
            if (!b2Body_IsValid(bodyComp.bodyId)) return;

            // Update Weapon Cooldown
            auto* weapon = registry.try_get<WeaponComponent>(entity);
            if (weapon && weapon->cooldownTimer > 0.0f) {
                weapon->cooldownTimer -= fixedDeltaTime;
            }

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
                b2Body_SetTransform(bodyComp.bodyId, bodyPos, b2MakeRot(angle));

                // --- Shooting (Left Click) ---
                if (weapon && input->isMouseButtonPressed(engine::MouseCode::Left) && weapon->cooldownTimer <= 0.0f)
                {
                    // Reset Cooldown
                    weapon->cooldownTimer = weapon->fireRate;

                    // Calculate spawn position (offset from center to avoid self-collision or just look better)
                    // For now, spawn at center + small offset in direction of aim
                    float spawnOffset = 30.0f; // Adjust based on player size
                    float cosA = std::cos(angle);
                    float sinA = std::sin(angle);
                    
                    engine::Vector2f spawnPos = {
                        bodyPos.x + cosA * spawnOffset,
                        bodyPos.y + sinA * spawnOffset
                    };

                    // Create Projectile Entity
                    auto projectile = registry.create();

                    // Physics
                    registry.emplace<engine::PendingPhysicsBodyComponent>(
                        projectile,
                        spawnPos,
                        engine::Vector2f{ 10.0f, 10.0f }, // Size
                        false, // Dynamic
                        1.0f,  // Density
                        true,  // isSensor
                        false, // fixedRotation
                        0.0f,   // damping
                        true,   // isBullet
                        engine::Vector2f{ cosA * weapon->projectileSpeed, sinA * weapon->projectileSpeed } // initialVelocity
                    );

                    // Visuals
                    registry.emplace<engine::TransformComponent>(projectile, spawnPos);
                    registry.emplace<engine::RenderableComponent>(projectile, "bullet_sprite", 2, sf::Color::White);

                    // Game Logic
                    registry.emplace<ProjectileComponent>(projectile, weapon->damage);
                    registry.emplace<DamageComponent>(projectile, weapon->damage); // Add DamageComponent!
                    registry.emplace<engine::LifetimeComponent>(projectile, weapon->projectileLifetime);

                }
            }
        });
    }

} // namespace game