#include "Application.hpp"
#include "engine/core/IWindow.hpp"
#include "engine/rendering/IRenderer.hpp"
#include "engine/core/ILoggerManager.hpp"
#include "engine/core/ILogger.hpp"
#include "engine/core/IInputManager.hpp"
#include <chrono> // For high-resolution clock and delta time

namespace engine
{
    Application::Application(IWindow& window, IRenderer& renderer, ILoggerManager& logManager, IInputManager& inputManager)
        : m_window(window)
        , m_renderer(renderer)
        , m_logManager(logManager)
        , m_inputManager(inputManager)
    {
        m_logger = m_logManager.GetLogger("Core");
        m_logger->info("Application starting up.");
    }

    Application::~Application() {
        m_logger->info("Application shutting down.");
    }

    void Application::run() {
        m_logger->info("Starting main loop.");
        auto lastTime = std::chrono::high_resolution_clock::now();

        // The main game loop
        while (m_window.isOpen()) {
            // Calculate delta time
            auto currentTime = std::chrono::high_resolution_clock::now();
            float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastTime).count();
            lastTime = currentTime;

            // Execute frame logic
            processInput();
            update(deltaTime);
            render();
        }
        m_logger->info("Main loop finished.");
    }

    void Application::processInput() {
        m_logger->trace("Processing input.");
        // Delegate event processing to the input manager
        m_inputManager.processEvents();

    }

    void Application::update(float deltaTime) {
        m_logger->trace("Updating game state.");
        // (void)deltaTime; // Suppress unused parameter warning
        // In Phase 1, there is no game state to update.
        // This will be filled in later (e.g., physics, AI).
    }

    void Application::render() {
        m_logger->trace("Rendering frame.");
        // Orchestrate the rendering process via the abstract interface
        m_renderer.beginFrame();
        m_renderer.clear({ 0, 0, 25, 255 }); // Dark blue background
        m_renderer.drawShape({100.0f, 100.0f}, 50.0f); // The Phase 1 milestone
        m_renderer.endFrame();
    }
}