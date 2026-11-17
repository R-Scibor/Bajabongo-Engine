#include "HeadlessSim.hpp"
#include "engine/assets/MapLoader.hpp"
#include "engine/ecs/ArchetypeManager.hpp"
#include "engine/ecs/EntityFactory.hpp"

#include <chrono>
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>

#ifndef BAJA_ASSETS_DIR
#define BAJA_ASSETS_DIR ""
#endif

int main() {
    constexpr float kDt = 1.0f / 60.0f;
    constexpr int kFrames = 600;

    const std::filesystem::path assets{BAJA_ASSETS_DIR};
    const auto mapPath = assets / "data" / "map_hideout.json";
    const auto archetypesPath = assets / "data" / "archetypes.json";
    if (!std::filesystem::is_regular_file(mapPath) || !std::filesystem::is_regular_file(archetypesPath)) {
        std::cerr << "baja_bench: assets not found under " << assets << "\n";
        return 1;
    }

    baja::test::HeadlessScene scene;
    auto archetypes = std::make_shared<engine::ArchetypeManager>(scene.logManager);
    if (!archetypes->loadArchetypes(archetypesPath.string())) {
        std::cerr << "baja_bench: failed to load archetypes\n";
        return 1;
    }
    auto factory = std::make_shared<engine::EntityFactory>(scene.context, archetypes);
    scene.context.m_entityFactory = factory;

    engine::MapLoader loader(scene.context);
    if (!loader.load(mapPath.string())) {
        std::cerr << "baja_bench: failed to load hideout map\n";
        return 1;
    }

    baja::test::HeadlessGameplayLoop loop(scene);
    loop.bodyCreation.update();

    // Warm-up so the timed run is not dominated by first-touch costs.
    loop.step(kDt);

    const auto start = std::chrono::steady_clock::now();
    for (int frame = 0; frame < kFrames; ++frame) {
        loop.step(kDt);
    }
    const auto elapsed = std::chrono::steady_clock::now() - start;
    const double ms = std::chrono::duration<double, std::milli>(elapsed).count();
    const double msPerFrame = ms / static_cast<double>(kFrames);
    const double fps = (ms > 0.0) ? (static_cast<double>(kFrames) * 1000.0 / ms) : 0.0;

    std::cout << "baja_bench: " << kFrames << " frames in " << ms << " ms (" << msPerFrame << " ms/frame, " << fps
              << " fps), entities=" << loop.entityCount() << ", finite=" << (loop.transformsFinite() ? "yes" : "no")
              << "\n";
    return loop.transformsFinite() ? 0 : 2;
}
