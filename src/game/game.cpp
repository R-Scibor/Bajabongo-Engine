#include "engine/pch.h"

#include "engine/core/Application.hpp"
#include "engine/core/EngineContext.hpp"
#include "engine/rendering/SFMLRenderer.hpp"
#include "engine/rendering/SFMLResourceManager.hpp"
#include "engine/rendering/Sprite.hpp"
#include "engine/rendering/AnimationClip.hpp"
#include "engine/assets/AssetManifestLoader.hpp"
#include "engine/input/SFMLInputManager.hpp"
#include "engine/logging/SpdlogManager.hpp"
#include "engine/core/ILogger.hpp"
#include "game/states/MainMenuState.hpp"
#include "game/states/GameplayState.hpp"
#include "game/states/MapSelectionState.hpp"
#include "game/states/SplashScreenState.hpp"
#include "engine/core/StateManager.hpp"

#include <box2d/box2d.h>
#include <cstdint>
#include <iostream>
#include <limits>
#include <memory>
#include <optional>
#include <string>

namespace {
    std::optional<std::uint32_t> parseSeed(const std::string& text) {
        if (text.empty()) return std::nullopt;
        try {
            std::size_t idx = 0;
            const unsigned long value = std::stoul(text, &idx, 10);
            if (idx != text.size() || value > std::numeric_limits<std::uint32_t>::max()) {
                return std::nullopt;
            }
            return static_cast<std::uint32_t>(value);
        } catch (const std::exception&) {
            return std::nullopt;
        }
    }
} // namespace

int main(int argc, char* argv[]) {
    // Dev shortcuts: --map hideout|hospital skips splash/menu and loads Gameplay
    // with that mission. --seed <n> makes the shared RNG repeatable. With no
    // flags, startup is unchanged (Splash -> Menu) and the RNG uses random_device.
    std::string devMap;
    std::optional<std::uint32_t> rngSeed;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            std::cerr << "Usage: game [--map hideout|hospital] [--seed <uint>]\n"
                      << "  --map <name>  Skip splash/menu and load that map directly (dev).\n"
                      << "  --seed <n>    Seed the engine RNG (dev; repeatable smokes and sessions).\n";
            return 0;
        }
        if (arg == "--map") {
            if (i + 1 >= argc) {
                std::cerr << "game: --map requires hideout or hospital\n";
                return 2;
            }
            devMap = argv[++i];
        } else if (arg.rfind("--map=", 0) == 0) {
            devMap = arg.substr(6);
        } else if (arg == "--seed") {
            if (i + 1 >= argc) {
                std::cerr << "game: --seed requires an unsigned integer\n";
                return 2;
            }
            rngSeed = parseSeed(argv[++i]);
            if (!rngSeed) {
                std::cerr << "game: --seed must be an unsigned 32-bit integer\n";
                return 2;
            }
        } else if (arg.rfind("--seed=", 0) == 0) {
            rngSeed = parseSeed(arg.substr(7));
            if (!rngSeed) {
                std::cerr << "game: --seed must be an unsigned 32-bit integer\n";
                return 2;
            }
        } else {
            std::cerr << "game: unknown argument: " << arg << "\n";
            return 2;
        }
    }
    if (!devMap.empty() && devMap != "hideout" && devMap != "hospital") {
        std::cerr << "game: --map must be hideout or hospital\n";
        return 2;
    }

    try {
        // Logging backend (shared so it can be stored in EngineContext)
        auto logManager = std::make_shared<engine::SpdlogManager>("config/logging.ini");
        auto gameLogger = logManager->getLogger("Game");

        gameLogger->info("Game is initializing...");

        // === COMPOSITION ROOT: Create core services ===

        // ECS registry
        auto registry = std::make_shared<entt::registry>();

        // Box2D 3.0: Create world using C API
        b2WorldDef worldDef = b2DefaultWorldDef();
        worldDef.gravity = {0.0f, 0.0f}; // Top-down, no gravity
        b2WorldId physicsWorldId = b2CreateWorld(&worldDef);

        gameLogger->info("EnTT registry and Box2D world created.");

        // Game-owned asset locations consumed by engine services (no engine hardcodes).
        engine::EngineContext::AssetPathConfig assetPaths;
        assetPaths.missingTexturePath = "../../assets/textures/eg.png";
        assetPaths.fontPaths = {"assets/fonts/default.ttf", "../../assets/fonts/default.ttf"};

        // Create rendering and input
        auto renderer = std::make_shared<engine::SFMLRenderer>();
        renderer->setLogger(logManager->getLogger("SFMLRenderer"));
        renderer->loadFonts(assetPaths.fontPaths);
        renderer->create("Game Window", 1920, 1080, true);
        renderer->setFramerateLimit(120);
        gameLogger->info("Window created successfully.");

        // Phase 5A: Create Resource Manager
        auto resourceManager = std::make_shared<engine::SFMLResourceManager>(logManager);
        resourceManager->setMissingTexturePath(assetPaths.missingTexturePath);
        renderer->setResourceManager(resourceManager);
        gameLogger->info("Resource Manager created.");

        auto spriteManager = std::make_shared<engine::SpriteManager>();
        gameLogger->info("Sprite Manager created.");

        auto animationLibrary = std::make_shared<engine::AnimationLibrary>();
        gameLogger->info("Animation Library created.");

        auto assetLoader = std::make_shared<engine::AssetManifestLoader>(
            resourceManager, spriteManager, animationLibrary, logManager->getLogger("AssetLoader"));
        gameLogger->info("Asset Manifest Loader created.");

        auto inputManager = std::make_shared<engine::SFMLInputManager>(*logManager, renderer->getNativeRenderWindow());

        // Create the engine context object (DI container)
        auto context = std::make_shared<engine::EngineContext>();
        context->assetPaths = assetPaths;
        context->m_logManager = logManager;
        context->m_registry = registry;
        context->m_physicsWorld = physicsWorldId;
        context->m_renderer = renderer;
        context->m_inputManager = inputManager;
        context->m_window = renderer;
        context->m_resourceManager = resourceManager;
        context->m_spriteManager = spriteManager;
        context->m_animationLibrary = animationLibrary;
        context->m_assetLoader = assetLoader;
        context->m_dispatcher = std::make_shared<entt::dispatcher>();
        if (rngSeed) {
            context->m_random.seed(*rngSeed);
        }
        gameLogger->info("RNG seed: {}", context->m_random.currentSeed());

        // Create the state manager, which requires a reference to the context
        auto stateManager = std::make_unique<engine::StateManager>(*context);

        // Inject all dependencies into Application
        // === State Registration & Initial State ===
        // Register states directly with the local manager before it's moved.
        stateManager->registerState<game::SplashScreenState>("Splash");
        stateManager->registerState<game::MainMenuState>("MainMenu");
        stateManager->registerState<game::GameplayState>("Gameplay");
        stateManager->registerState<game::MapSelectionState>("MapSelection");

        // Push the first state and process transitions to ensure it's active
        // before the main loop begins.
        if (!devMap.empty()) {
            // Splash normally loads the asset manifest; do that here when skipping it.
            if (!assetLoader->load("../../assets/data/resources.json")) {
                gameLogger->error("Dev --map: failed to load ../../assets/data/resources.json");
                return 1;
            }
            const std::string display = (devMap == "hospital") ? "Hospital" : "Hideout";
            context->currentMission = engine::EngineContext::MissionConfig{
                .mapFile = "../../assets/data/map_" + devMap + ".json", .displayName = display};
            gameLogger->info("Dev --map {}: skipping splash/menu, loading Gameplay directly.", devMap);
            stateManager->requestPush("Gameplay");
        } else {
            stateManager->requestPush("Splash");
        }
        stateManager->processTransitions();

        // Inject all dependencies into Application.
        // After this, `stateManager` is a nullptr; use `context->m_stateManager`.
        engine::Application app(context, std::move(stateManager));

        // Run the game loop
        app.run();

        // Cleanup Box2D world
        b2DestroyWorld(physicsWorldId);
        gameLogger->info("Game shutting down.");
    } catch (const std::exception& e) {
        std::cerr << "An unhandled exception occurred: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
