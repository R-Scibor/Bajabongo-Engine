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

        // === Test entities ===

        // Pudełko – dynamic body, startuje na górze ekranu
        auto box = registry.create();
        registry.emplace<engine::PendingPhysicsBodyComponent>(
            box,
            engine::Vector2f{ 100.f, 100.f },   // pozycja (góra ekranu, w pikselach/Twoich jednostkach)
            engine::Vector2f{ 20.f, 20.f },     // rozmiar (używany przez PhysicsBodyCreationSystem)
            false,                              // isStatic = false → dynamic body
            0.5f                                // density
        );
        registry.emplace<engine::TransformComponent>(box);
        registry.emplace<engine::RenderableComponent>(box, 20.0f); // promień kółka

        // Ziemia – statyczne body, na dole ekranu
        auto ground = registry.create();
        registry.emplace<engine::PendingPhysicsBodyComponent>(
            ground,
            engine::Vector2f{ 100.f, 600.f },   // pozycja (dół ekranu)
            engine::Vector2f{ 400.f, 20.f },    // rozmiar: szeroka platforma
            true,                               // isStatic = true → static body
            0.0f                                // density nieistotna dla statycznego
        );
        registry.emplace<engine::TransformComponent>(ground);
        registry.emplace<engine::RenderableComponent>(ground, 200.0f); // duży „promień”, jako pasek




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

