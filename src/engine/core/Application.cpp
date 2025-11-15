#include "Application.hpp"
#include "engine/core/IWindow.hpp"
#include "engine/rendering/IRenderer.hpp"
#include "engine/core/ILoggerManager.hpp"
#include "engine/core/ILogger.hpp"
#include "engine/core/IInputManager.hpp"
#include "engine/components/PhysicsBodyComponent.hpp"


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
        , m_physicsBodyCreationSystem(registry, physicsWorld, logManager)
        , m_physicsSyncSystem(registry, logManager)
        , m_renderSystem(registry, renderer)
    {
        m_logger = m_logManager.GetLogger("Core");
        m_logger->info("Application starting up.");

        // === HOOK: lifecycle cleanup for physics bodies ===
        auto physicsLogger = m_logManager.GetLogger("Physics");

        m_registry.on_destroy<engine::PhysicsBodyComponent>()
            .connect<&Application::onPhysicsBodyDestroyed>(this);

        physicsLogger->info("on_destroy hook for PhysicsBodyComponent registered.");
    }

    Application::~Application() {
        m_logger->info("Application shutting down.");
    }

    void Application::run() {
        m_logger->info("Starting main loop.");
        auto lastTime = std::chrono::high_resolution_clock::now();

        // Main game loop
        while (m_window.isOpen()) {
            auto currentTime = std::chrono::high_resolution_clock::now();
            float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastTime).count();
            lastTime = currentTime;

            m_accumulator += deltaTime;

            processInput();

            while (m_accumulator >= m_physicsTimeStep)
            {
                fixedUpdate();
                m_accumulator -= m_physicsTimeStep;
            }

            update();
            render();
        }
        m_logger->info("Main loop finished.");
    }

    void Application::processInput() {
        m_logger->trace("Processing input.");
        m_inputManager.processEvents();
    }

    void Application::update() {
        m_logger->trace("Updating game state.");
        // Non-physics game logic goes here
    }

    void Application::fixedUpdate()
    {
        m_logger->trace("Performing fixed update (physics).");

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
        b2World_Step(m_physicsWorld, m_physicsTimeStep, 8);

        m_physicsSyncSystem.update();
    }

    void Application::render() {
        m_logger->trace("Rendering frame.");
        m_renderer.beginFrame();
        m_renderer.clear({ 0, 0, 25, 255 }); // Dark blue background

        // Nowy, ECS‑owy render
        m_renderSystem.update();

        m_renderer.endFrame();
    }

    void Application::onPhysicsBodyDestroyed(entt::registry& registry, entt::entity entity)
    {
        // Komponent w on_destroy jest jeszcze dostępny
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

        // Box2D 3.0: niszczymy ciało po samym bodyId, BEZ worldId
        b2DestroyBody(physicsBody.bodyId);

        if (m_logger) {
            m_logger->trace(
                "Destroyed Box2D body for entity {}",
                entt::to_integral(entity)
            );
        }
    }

}
