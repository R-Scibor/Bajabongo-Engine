#include "engine/pch.h"
#include "GameplayState.hpp"
#include "engine/core/EngineContext.hpp"
#include "engine/core/ILogger.hpp"
#include "engine/core/ILoggerManager.hpp"
#include "engine/components/PendingPhysicsBodyComponent.hpp"
#include "engine/components/PhysicsBodyComponent.hpp"
#include "engine/core/math/Vector2.hpp"
#include "engine/events/StateEvents.hpp"
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
        m_logger = context.m_logManager->GetLogger("GameplayState");
    }

    void GameplayState::onEnter(engine::EngineContext& context) {
        if (m_logger) m_logger->info("Entering GameplayState.");
        
        auto& registry = *context.m_registry;

        // Box – dynamic body, starts at the top of the screen
        auto box = registry.create();
    }

    void GameplayState::onExit(engine::EngineContext& context) {
        if (m_logger) m_logger->info("Exiting GameplayState.");
        
        if (m_physicsCleanupHook) {
            context.m_registry->on_destroy<engine::PhysicsBodyComponent>().disconnect(this);
            if (m_logger) m_logger->info("Physics cleanup hook disconnected.");
        }
        // Optional: context.m_registry->clear();
    }

    void GameplayState::handleEvent(engine::EngineContext& context, const sf::Event& event) {
        if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
            if (keyPressed->code == sf::Keyboard::Key::Escape) {
                if (m_logger) m_logger->info("Escape key pressed. Requesting pop to previous state.");
                context.m_dispatcher->enqueue<engine::RequestStatePopEvent>();
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

    void GameplayState::onPhysicsBodyDestroyed(entt::registry& registry, entt::entity entity) {
        if (!m_logger) return;

        const auto& bodyComp = registry.get<engine::PhysicsBodyComponent>(entity);
        if (b2Body_IsValid(bodyComp.bodyId)) {
            m_logger->info("Destroying physics body for entity {}", static_cast<uint32_t>(entity));
            b2DestroyBody(bodyComp.bodyId);
        } else {
            m_logger->warn("Attempted to destroy an invalid physics body for entity {}", static_cast<uint32_t>(entity));
        }
    }

} // namespace game