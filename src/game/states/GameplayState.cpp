#include "engine/pch.h"
#include "GameplayState.hpp"
#include "engine/core/EngineContext.hpp"
#include "engine/core/ILogger.hpp"
#include "engine/core/ILoggerManager.hpp"
#include "engine/components/PendingPhysicsBodyComponent.hpp"
#include "engine/core/math/Vector2.hpp"
#include "engine/physics/PhysicsBodyCreationSystem.hpp"
#include "engine/physics/PhysicsSyncSystem.hpp"
#include "engine/rendering/RenderSystem.hpp"
#include <box2d/box2d.h>
#include "engine/components/TransformComponent.hpp"
#include "engine/components/RenderableComponent.hpp"
#include <entt/entt.hpp>
#include <SFML/Window/Event.hpp>

namespace game {

    GameplayState::GameplayState(engine::EngineContext& context)
        : m_physicsBodyCreationSystem(context)
        , m_physicsSyncSystem(context)
        , m_renderSystem(context)
    {
    }

    void GameplayState::onEnter(engine::EngineContext& context) {
        context.m_logManager->GetLogger("Game")->info("Entering GameplayState.");
        
        auto& registry = *context.m_registry;

        // Box – dynamic body, starts at the top of the screen
        auto box = registry.create();
        registry.emplace<engine::PendingPhysicsBodyComponent>(
            box,
            engine::Vector2f{ 100.f, 100.f },   // position (top of the screen)
            engine::Vector2f{ 20.f, 20.f },     // size (used by PhysicsBodyCreationSystem)
            false,                              // isStatic = false → dynamic body
            0.5f                                // density
        );
        registry.emplace<engine::TransformComponent>(box);
        registry.emplace<engine::RenderableComponent>(box, 20.0f); // circle radius

        // Ground – static body at the bottom of the screen
        auto ground = registry.create();
        registry.emplace<engine::PendingPhysicsBodyComponent>(
            ground,
            engine::Vector2f{ 100.f, 600.f },   // position (bottom of the screen)
            engine::Vector2f{ 400.f, 20.f },    // size: wide platform
            true,                               // isStatic = true → static body
            0.0f                                // density not relevant for static body
        );
        registry.emplace<engine::TransformComponent>(ground);
        registry.emplace<engine::RenderableComponent>(ground, 200.0f); // large "radius", as a bar
    }

    void GameplayState::onExit(engine::EngineContext& context) {
        context.m_logManager->GetLogger("Game")->info("Exiting GameplayState.");
        // In the future, game entities should be cleaned up here.
    }

    void GameplayState::handleEvent(engine::EngineContext& context, const sf::Event& event) {
        // Handle gameplay-specific input, e.g., pausing the game.
        if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
            if (keyPressed->code == sf::Keyboard::Key::Escape) {
                // Placeholder for pushing a "Pause" state
                context.m_logManager->GetLogger("Game")->info("Escape key pressed, requesting to push 'Pause' state (not implemented yet).");
                // context.m_stateManager->requestPush("Pause");
            }
        }
    }

    void GameplayState::update(engine::EngineContext& context, float fixedDeltaTime) {
        m_physicsBodyCreationSystem.update();

        b2World_Step(context.m_physicsWorld, fixedDeltaTime, 8);

        m_physicsSyncSystem.update();
    }

    void GameplayState::render(engine::EngineContext& context) {
        m_renderSystem.update();
    }

} // namespace game