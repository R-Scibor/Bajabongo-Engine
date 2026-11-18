#include "HeadlessScene.hpp"
#include "engine/assets/MapLoader.hpp"
#include "engine/components/MetaComponent.hpp"
#include "engine/components/TransformComponent.hpp"
#include "engine/ecs/ArchetypeManager.hpp"
#include "engine/ecs/EntityFactory.hpp"
#include "game/components/PatrolBehaviorComponent.hpp"
#include "game/setup/GameComponentRegistry.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <cmath>
#include <filesystem>
#include <string>

#ifndef BAJA_ASSETS_DIR
#define BAJA_ASSETS_DIR ""
#endif

TEST_CASE("Hospital patrol waypoints stay near each enemy spawn") {
    const std::filesystem::path assets{BAJA_ASSETS_DIR};
    const auto mapPath = assets / "data" / "map_hospital.json";
    const auto archetypesPath = assets / "data" / "archetypes.json";
    if (!std::filesystem::is_regular_file(mapPath) || !std::filesystem::is_regular_file(archetypesPath)) {
        SKIP("hospital map or archetypes.json is not available");
    }

    baja::test::HeadlessScene scene;
    auto archetypes = std::make_shared<engine::ArchetypeManager>(scene.logManager);
    REQUIRE(archetypes->loadArchetypes(archetypesPath.string()));
    auto factory = std::make_shared<engine::EntityFactory>(scene.context, archetypes);
    scene.context.m_entityFactory = factory;
    game::RegisterGameComponents(scene.context);

    engine::MapLoader loader(scene.context);
    REQUIRE(loader.load(mapPath.string()));

    int patrols = 0;
    auto view = scene.registry->view<game::PatrolBehaviorComponent, engine::TransformComponent>();
    for (auto entity : view) {
        const auto& patrol = view.get<game::PatrolBehaviorComponent>(entity);
        const auto& transform = view.get<engine::TransformComponent>(entity);
        REQUIRE_FALSE(patrol.waypoints.empty());
        ++patrols;

        const auto& first = patrol.waypoints.front();
        const float dx = first.x - transform.position.x;
        const float dy = first.y - transform.position.y;
        const float dist = std::sqrt(dx * dx + dy * dy);
        // Shared absolute archetype points (100, 200) would be thousands of units away.
        CHECK(dist < 800.0f);
        CHECK_FALSE((std::abs(first.x - 100.0f) < 1.0f && std::abs(first.y - 200.0f) < 1.0f));
    }
    CHECK(patrols == 16);
}
