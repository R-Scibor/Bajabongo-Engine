#include "engine/pch.h"

#include "Application.hpp"

#include "engine/core/EngineContext.hpp"
#include "engine/core/IWindow.hpp"
#include "engine/rendering/IRenderer.hpp"
#include "engine/rendering/SFMLRenderer.hpp" // Required for casting to get sf::RenderWindow
#include "engine/core/ILoggerManager.hpp"
#include "engine/core/ILogger.hpp"
#include "engine/core/IInputManager.hpp"
#include "engine/core/GuiService.hpp"

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
        
        // Initialize GuiService
        m_context->m_guiService = std::make_shared<GuiService>();
        m_context->m_guiService->SetLogger(m_context->m_logManager->GetLogger("GuiService"));
        
        // We need the concrete sf::RenderWindow for ImGui-SFML.
        // This assumes we are using SFMLRenderer, which is currently the only implementation.
        auto sfmlRenderer = std::dynamic_pointer_cast<SFMLRenderer>(m_context->m_window);
        if (sfmlRenderer) {
            m_context->m_guiService->Init(sfmlRenderer->getNativeRenderWindow());
        } else {
            m_logger->error("Failed to initialize GuiService: Window is not SFMLRenderer");
        }

        m_logger->info("Application starting up.");
    }

    Application::~Application()
    {
        m_context->m_guiService->Shutdown();
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

            // Prevent spiral of death
            if (m_accumulator > 0.2f) {
                m_accumulator = 0.2f;
            }

            // 1. Process all pending inputs and forward to state manager
            processInput();

            // 2. Start Gui Frame
            // Ensure ImGui is updated once per frame with the current delta time.
            auto sfmlRenderer = std::dynamic_pointer_cast<SFMLRenderer>(m_context->m_window);
            if (sfmlRenderer) {
                m_context->m_guiService->BeginFrame(sfmlRenderer->getNativeRenderWindow(), sf::seconds(deltaTime));
            }

            // 3. Run fixed-step updates for physics and game logic
            while (m_accumulator >= m_fixedTimestep) {
                fixedUpdate();
                m_accumulator -= m_fixedTimestep;
            }

            // 4. Run non-essential updates and process event dispatcher
            update();

            // 5. Process all queued state transitions at a safe point
            m_stateManager->processTransitions();

            if (m_stateManager->isEmpty()) {
                m_context->m_window->close();
            }

            // 6. Render the final state stack
            render();
        }

        m_logger->info("Main loop finished.");
    }

    void Application::processInput()
    {
        m_logger->trace("Processing input.");

        // Poll all SFML events and forward them to the active state
        while (auto event = m_context->m_window->pollEvent()) {
            
            // Pass to GuiService first
            bool captured = false;
            auto sfmlRenderer = std::dynamic_pointer_cast<SFMLRenderer>(m_context->m_window);
            if (sfmlRenderer) {
                 captured = m_context->m_guiService->HandleEvent(sfmlRenderer->getNativeRenderWindow(), *event);
            }

            // If UI captured input, we might want to skip game processing for some events.
            // But we still let the system handle Closed event.
            if (!captured) {
                 m_stateManager->handleEvent(*event);
                 m_context->m_inputManager->processEvent(*event);
            }

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
        
        // Render Game World
        m_stateManager->render();

        // Render GUI on top
        auto sfmlRenderer = std::dynamic_pointer_cast<SFMLRenderer>(m_context->m_window);
        if (sfmlRenderer) {
             m_context->m_guiService->Render(sfmlRenderer->getNativeRenderWindow());
        }

        m_context->m_renderer->endFrame();
    }


} // namespace engine
