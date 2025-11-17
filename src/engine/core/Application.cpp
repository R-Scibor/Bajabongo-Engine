#include "engine/pch.h"

#include "Application.hpp"

#include "engine/core/EngineContext.hpp"
#include "engine/core/IWindow.hpp"
#include "engine/rendering/IRenderer.hpp"
#include "engine/core/ILoggerManager.hpp"
#include "engine/core/ILogger.hpp"
#include "engine/core/IInputManager.hpp"

#include <SFML/Window/Event.hpp>
#include <box2d/box2d.h>
#include <chrono>

namespace engine {

    Application::Application(std::shared_ptr<EngineContext> context,
                             std::unique_ptr<StateManager> stateManager)
        : m_context{ std::move(context) }
        , m_stateManager{ std::move(stateManager) }
    {
        m_context->m_stateManager = m_stateManager.get();
        m_logger = m_context->m_logManager->GetLogger("Core");
        m_logger->info("Application starting up.");
    }

    Application::~Application()
    {
        m_context->m_stateManager = nullptr;
        if (m_logger) {
            m_logger->info("Application shutting down.");
        }
    }

    void Application::run()
    {
        m_logger->info("Starting main loop.");

        auto lastTime = std::chrono::high_resolution_clock::now();

        // Main game loop
        while (m_context->m_window->isOpen()) {
            auto currentTime = std::chrono::high_resolution_clock::now();
            float deltaTime =
                std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastTime).count();
            lastTime = currentTime;

            m_accumulator += deltaTime;

            // 1. Process all pending inputs and forward to state manager
            processInput();

            // 2. Run fixed-step updates for physics and game logic
            while (m_accumulator >= m_fixedTimestep) {
                fixedUpdate();
                m_accumulator -= m_fixedTimestep;
            }

            // 3. Run non-essential updates and process event dispatcher
            update();

            // 4. Process all queued state transitions at a safe point
            m_stateManager->processTransitions();

            if (m_stateManager->isEmpty()) {
                m_context->m_window->close();
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
        while (auto event = m_context->m_window->pollEvent()) {
            m_stateManager->handleEvent(*event);
            m_context->m_inputManager->processEvent(*event);
            if (event->is<sf::Event::Closed>()) {
                m_context->m_window->close();
            }
        }
    }

    void Application::update()
    {
        m_logger->trace("Updating game state and dispatcher.");
        // Process all enqueued events (e.g., RequestStateSwapEvent)
        m_context->m_dispatcher->update();
    }

    void Application::fixedUpdate()
    {
        m_logger->trace("Performing fixed update.");
        // Delegate the fixed update to the active game state.
        // The state itself will be responsible for updating physics, ECS systems, etc.
        m_stateManager->update(m_fixedTimestep);
    }

    void Application::render()
    {
        m_logger->trace("Rendering frame.");

        m_context->m_renderer->beginFrame();
        m_context->m_renderer->clear({0, 0, 25, 255});
        m_stateManager->render();
        m_context->m_renderer->endFrame();
    }


} // namespace engine
