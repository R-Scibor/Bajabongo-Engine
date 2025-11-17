#include "engine/pch.h"

#include "engine/core/Application.hpp"
#include "engine/core/EngineContext.hpp"
#include "engine/rendering/SFMLRenderer.hpp"
#include "engine/input/SFMLInputManager.hpp"
#include "engine/logging/SpdlogManager.hpp"
#include "engine/core/ILogger.hpp"
#include "game/states/MainMenuState.hpp"
#include "game/states/GameplayState.hpp"
#include "engine/core/StateManager.hpp"

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
        auto context = std::make_shared<engine::EngineContext>();
        context->m_logManager = logManager;
        context->m_registry = registry;
        context->m_physicsWorld = physicsWorldId;
        context->m_renderer = renderer;
        context->m_inputManager = inputManager;
        context->m_window = renderer;
        context->m_dispatcher   = std::make_shared<entt::dispatcher>();

        // Create the state manager, which requires a reference to the context
        auto stateManager = std::make_unique<engine::StateManager>(*context);

        // Inject all dependencies into Application
        // === State Registration & Initial State ===
        // Register states directly with the local manager before it's moved.
        stateManager->registerState<game::MainMenuState>("MainMenu");
        stateManager->registerState<game::GameplayState>("Gameplay");

        // Push the first state and process transitions to ensure it's active
        // before the main loop begins.
        stateManager->requestPush("MainMenu");
        stateManager->processTransitions();

        // Inject all dependencies into Application.
        // After this, `stateManager` is a nullptr; use `context->m_stateManager`.
        engine::Application app(context, std::move(stateManager));

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
