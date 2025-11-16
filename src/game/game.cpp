#include "engine/pch.h"

#include "engine/core/Application.hpp"
#include "engine/core/EngineContext.hpp"
#include "engine/rendering/SFMLRenderer.hpp"
#include "engine/input/SFMLInputManager.hpp"
#include "engine/logging/SpdlogManager.hpp"
#include "engine/core/ILogger.hpp"
#include "engine/components/PendingPhysicsBodyComponent.hpp"
#include "engine/core/math/Vector2.hpp"
#include "engine/components/TransformComponent.hpp"
#include "engine/components/RenderableComponent.hpp"

#include <box2d/box2d.h>
#include <iostream>
#include <memory>

int main() {
    try {
        // Logging backend (shared so it can be stored in EngineContext)
        auto logManager = std::make_shared<engine::SpdlogManager>("config/logging.ini");
        auto gameLogger = logManager->GetLogger("Game");

        gameLogger->info("Game is initializing...");

        // === COMPOSITION ROOT: Create core services ===

        // ECS registry
        auto registry = std::make_shared<entt::registry>();

        // Box2D 3.0: Create world using C API
        b2WorldDef worldDef = b2DefaultWorldDef();
        worldDef.gravity = { 0.0f, 9.8f }; // Positive Y = down
        b2WorldId physicsWorldId = b2CreateWorld(&worldDef);

        gameLogger->info("EnTT registry and Box2D world created.");

        // Create rendering and input
        auto renderer = std::make_shared<engine::SFMLRenderer>();
        renderer->create("Game Window", 1280, 720);
        gameLogger->info("Window created successfully.");

        auto inputManager = std::make_shared<engine::SFMLInputManager>(
            *logManager,
            renderer->getNativeRenderWindow()
        );

        // Create the engine context object (DI container)
        engine::EngineContext context;
        context.m_logManager = logManager;
        context.m_registry = registry;
        context.m_physicsWorld = physicsWorldId;
        context.m_renderer = renderer;
        context.m_inputManager = inputManager;
        context.m_window = renderer;

        // Phase 4 services (not used yet)
        // context.m_dispatcher   = std::make_shared<entt::dispatcher>();
        // context.m_stateManager = nullptr;

        // Inject all dependencies into Application via the context
        engine::Application app(context);

        // === Test entities ===

        // Box – dynamic body, starts at the top of the screen
        auto box = registry->create();
        registry->emplace<engine::PendingPhysicsBodyComponent>(
            box,
            engine::Vector2f{ 100.f, 100.f },   // position (top of the screen)
            engine::Vector2f{ 20.f, 20.f },     // size (used by PhysicsBodyCreationSystem)
            false,                              // isStatic = false → dynamic body
            0.5f                                // density
        );
        registry->emplace<engine::TransformComponent>(box);
        registry->emplace<engine::RenderableComponent>(box, 20.0f); // circle radius

        // Ground – static body at the bottom of the screen
        auto ground = registry->create();
        registry->emplace<engine::PendingPhysicsBodyComponent>(
            ground,
            engine::Vector2f{ 100.f, 600.f },   // position (bottom of the screen)
            engine::Vector2f{ 400.f, 20.f },    // size: wide platform
            true,                               // isStatic = true → static body
            0.0f                                // density not relevant for static body
        );
        registry->emplace<engine::TransformComponent>(ground);
        registry->emplace<engine::RenderableComponent>(ground, 200.0f); // large "radius", as a bar

        // Run the game loop
        app.run();

        // Cleanup Box2D world
        b2DestroyWorld(physicsWorldId);
        gameLogger->info("Game shutting down.");
    }
    catch (const std::exception& e) {
        std::cerr << "An unhandled exception occurred: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
