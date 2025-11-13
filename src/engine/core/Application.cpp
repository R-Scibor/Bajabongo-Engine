#include "Application.hpp"
#include "engine/core/IWindow.hpp"
#include "engine/rendering/IRenderer.hpp"
#include "engine/core/ILoggerManager.hpp"
#include "engine/core/ILogger.hpp"
#include "engine/core/IInputManager.hpp"

#include <entt/entt.hpp>
#include <box2d/box2d.h>
#include <chrono>

namespace engine
{
    Application::Application(
        IWindow& window,
        IRenderer& renderer,
        ILoggerManager& logManager,
        IInputManager& inputManager,
        entt::registry& registry,
        b2WorldId physicsWorld
    )
        : m_window(window)
        , m_renderer(renderer)
        , m_logManager(logManager)
        , m_inputManager(inputManager)
        , m_registry(registry)
        , m_physicsWorld(physicsWorld)
        , m_physicsBodyCreationSystem(registry, physicsWorld)
        , m_physicsSyncSystem(registry)
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

        // Main game loop
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
        m_inputManager.processEvents();
    }

    void Application::update(float deltaTime) {
        m_logger->trace("Updating game state.");

        m_physicsBodyCreationSystem.update();

        // === PHYSICS INTEGRATION ===
        // Box2D 3.0: Step the world using the C API
        // Parameters:
        //   - worldId: The world to simulate
        //   - timeStep: Time delta (in seconds)
        //   - subStepCount: Number of sub-steps (default: 4, we use 8 for more precision)
        //
        // NOTE: Box2D 3.0 removed velocity/position iterations - now it uses subStepCount
        // See: https://box2d.org/posts/2024/02/solver/
        b2World_Step(m_physicsWorld, deltaTime, 8);

        m_physicsSyncSystem.update();
    }

    void Application::render() {
        m_logger->trace("Rendering frame.");
        m_renderer.beginFrame();
        m_renderer.clear({ 0, 0, 25, 255 }); // Dark blue background
        m_renderer.drawShape({ 100.0f, 100.0f }, 50.0f); // Phase 1 milestone
        m_renderer.endFrame();
    }
}
