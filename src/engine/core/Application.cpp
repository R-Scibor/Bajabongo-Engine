#include "engine/pch.h"

#include "Application.hpp"

#include "engine/core/EngineContext.hpp"
#include "engine/core/IWindow.hpp"
#include "engine/rendering/IRenderer.hpp"
#include "engine/core/ILoggerManager.hpp"
#include "engine/core/ILogger.hpp"
#include "engine/core/IInputManager.hpp"
#include "engine/components/PhysicsBodyComponent.hpp"

#include <SFML/Window/Event.hpp>
#include <box2d/box2d.h>
#include <chrono>

namespace engine {

    Application::Application(EngineContext& context)
        : m_context{ context }
        , m_window{ context.m_window }
        , m_renderer{ context.m_renderer }
        , m_logManager{ context.m_logManager }
        , m_inputManager{ context.m_inputManager }
        , m_stateManager{ context } // StateManager is constructed here
        , m_registry{ context.m_registry }
        , m_physicsWorld{ context.m_physicsWorld }
    {
        // The Application owns the StateManager, so it sets the context's non-owning pointer.
        m_context.m_stateManager = &m_stateManager;
        m_logger = m_logManager->GetLogger("Core");
        m_logger->info("Application starting up.");

        // === HOOK: lifecycle cleanup for physics bodies ===
        auto physicsLogger = m_logManager->GetLogger("Physics");

        m_registry->on_destroy<engine::PhysicsBodyComponent>()
            .connect<&Application::onPhysicsBodyDestroyed>(this);

        physicsLogger->info("on_destroy hook for PhysicsBodyComponent registered.");
    }

    Application::~Application()
    {
        // Clear the context's pointer to the StateManager, as it's about to be destroyed.
        m_context.m_stateManager = nullptr;

        if (m_logger) {
            m_logger->info("Application shutting down.");
        }
    }

    void Application::run()
    {
        m_logger->info("Starting main loop.");

        auto lastTime = std::chrono::high_resolution_clock::now();

        // Main game loop
        while (m_window->isOpen()) {
            auto currentTime = std::chrono::high_resolution_clock::now();
            float deltaTime =
                std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastTime).count();
            lastTime = currentTime;

            m_accumulator += deltaTime;

            // 1. Process all pending inputs and forward to state manager
            processInput();

            // 2. Run fixed-step updates for physics and game logic
            while (m_accumulator >= m_physicsTimeStep) {
                fixedUpdate();
                m_accumulator -= m_physicsTimeStep;
            }

            // 3. Run non-essential updates and process event dispatcher
            update();

            // 4. Process all queued state transitions at a safe point
            m_stateManager.processTransitions();

            if (m_stateManager.isEmpty()) {
                m_window->close();
            }

            // 5. Render the final state stack
            render();
        }

        m_logger->info("Main loop finished.");
    }

    void Application::processInput()
    {
        m_logger->trace("Processing input.");

        // Poll all SFML events and forward them to the active state
        while (auto event = m_window->pollEvent()) {
            // Give the state manager first dibs on the event
            m_stateManager.handleEvent(*event);

            // Also update the input manager for direct polling (e.g., isKeyPressed)
            m_inputManager->processEvent(*event);

            if (event->is<sf::Event::Closed>()) {
                m_window->close();
            }
        }
    }

    void Application::update()
    {
        m_logger->trace("Updating game state and dispatcher.");
        // Process all enqueued events (e.g., RequestStateSwapEvent)
        m_context.m_dispatcher->update();
    }

    void Application::fixedUpdate()
    {
        m_logger->trace("Performing fixed update.");
        // Delegate the fixed update to the active game state.
        // The state itself will be responsible for updating physics, ECS systems, etc.
        m_stateManager.update(m_physicsTimeStep);
    }

    void Application::render()
    {
        m_logger->trace("Rendering frame.");

        m_renderer->beginFrame();
        m_renderer->clear({ 0, 0, 25, 255 }); // Dark blue background

        // Delegate rendering to the state manager, which renders all states on the stack
        m_stateManager.render();

        m_renderer->endFrame();
    }

    void Application::onPhysicsBodyDestroyed(entt::registry& registry, entt::entity entity)
    {
        // Component is still valid in on_destroy
        auto& physicsBody = registry.get<PhysicsBodyComponent>(entity);

        if (!b2Body_IsValid(physicsBody.bodyId)) {
            if (m_logger) {
                m_logger->warn(
                    "on_destroy: physics body already invalid for entity {}",
                    entt::to_integral(entity)
                );
            }
            return;
        }

        // Box2D 3.0: destroy body using only bodyId (no worldId needed)
        b2DestroyBody(physicsBody.bodyId);

        if (m_logger) {
            m_logger->trace(
                "Destroyed Box2D body for entity {}",
                entt::to_integral(entity)
            );
        }
    }

} // namespace engine
