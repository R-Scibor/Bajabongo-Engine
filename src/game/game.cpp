#include "engine/core/Application.hpp"
#include "engine/rendering/SFMLRenderer.hpp"
#include "engine/input/SFMLInputManager.hpp"
#include "engine/logging/SpdlogManager.hpp"
#include "engine/core/ILogger.hpp"
#include "engine/components/PendingPhysicsBodyComponent.hpp"
#include "engine/core/math/Vector2.hpp"
#include "engine/components/TransformComponent.hpp"
#include "engine/components/RenderableComponent.hpp"

#include <entt/entt.hpp>
#include <box2d/box2d.h>
#include <iostream>

int main() {
    try {
        engine::SpdlogManager logManager("config/logging.ini");
        auto gameLogger = logManager.GetLogger("Game");

        gameLogger->info("Game is initializing...");

        // === COMPOSITION ROOT: Create core services ===
        entt::registry registry;

        // Box2D 3.0: Create world using C API
        b2WorldDef worldDef = b2DefaultWorldDef();
        worldDef.gravity = { 0.0f, 9.8f }; // Positive Y = down
        b2WorldId physicsWorldId = b2CreateWorld(&worldDef);

        gameLogger->info("EnTT registry and Box2D world created.");

        // Create rendering and input
        engine::SFMLRenderer renderer;
        renderer.create("Game Window", 1280, 720);
        gameLogger->info("Window created successfully.");

        engine::SFMLInputManager inputManager(logManager, renderer.getNativeRenderWindow());

        // Inject all dependencies into Application
        engine::Application app(renderer, renderer, logManager, inputManager, registry, physicsWorldId);

        // Test entity: box
        auto box = registry.create();
        registry.emplace<engine::PendingPhysicsBodyComponent>(
            box,
            engine::Vector2f{ 100.f, 100.f },
            engine::Vector2f{ 20.f, 20.f },
            false,
            0.5f
        );
        registry.emplace<engine::TransformComponent>(box);
        registry.emplace<engine::RenderableComponent>(box, 20.0f); // prosty promień


        // Run the game loop (we'll add timed destruction next)
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

