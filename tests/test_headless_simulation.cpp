#include "HeadlessSim.hpp"
#include "engine/assets/MapLoader.hpp"
#include "engine/components/MetaComponent.hpp"
#include "engine/ecs/ArchetypeManager.hpp"
#include "engine/ecs/EntityFactory.hpp"
#include "engine/physics/PhysicsConstants.hpp"
#include "game/ai/PatrolWaypointValidation.hpp"
#include "game/setup/GameComponentRegistry.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <memory>

#ifndef BAJA_ASSETS_DIR
#define BAJA_ASSETS_DIR ""
#endif

namespace {

    constexpr float kDt = 1.0f / 60.0f;
    constexpr int kFrames = 300;

    std::filesystem::path assetsDir() {
        const std::filesystem::path configured{BAJA_ASSETS_DIR};
        if (!configured.empty() && std::filesystem::is_directory(configured)) {
            return configured;
        }
        return {};
    }

} // namespace

TEST_CASE("Headless simulation stays finite and does not leak entities") {
    baja::test::HeadlessScene scene;
    baja::test::HeadlessGameplayLoop loop(scene);

    scene.addStaticBox(0.0f, 40.0f, 80.0f, 4.0f, engine::PhysicsCategory::Wall);

    auto mover = scene.registry->create();
    scene.registry->emplace<engine::TransformComponent>(mover, engine::Vector2f{0.0f, 0.0f}, 0.0f,
                                                        engine::Vector2f{1.0f, 1.0f});
    engine::PendingPhysicsBodyComponent pending{.position = {0.0f, 0.0f},
                                                .isStatic = false,
                                                .fixedRotation = true,
                                                .linearDamping = 0.0f,
                                                .isBullet = false,
                                                .initialVelocity = {30.0f, 0.0f},
                                                .rotation = 0.0f,
                                                .fixtures = {}};
    engine::FixtureDef fixture;
    fixture.size = {8.0f, 8.0f};
    fixture.density = 1.0f;
    fixture.categoryBits = engine::PhysicsCategory::Enemy;
    fixture.maskBits = engine::PhysicsCategory::Wall;
    pending.fixtures.push_back(fixture);
    scene.registry->emplace<engine::PendingPhysicsBodyComponent>(mover, pending);

    auto puff = scene.registry->create();
    scene.registry->emplace<engine::TransformComponent>(puff, engine::Vector2f{1.0f, 1.0f}, 0.0f,
                                                        engine::Vector2f{1.0f, 1.0f});
    scene.registry->emplace<engine::LifetimeComponent>(puff, 10.0f * kDt);

    const int startCount = loop.entityCount();
    REQUIRE(startCount >= 2);

    for (int frame = 0; frame < kFrames; ++frame) {
        loop.step(kDt);
        REQUIRE(loop.transformsFinite());
        if (frame == 0) {
            CHECK(loop.entityCount() == startCount);
        }
        if (frame >= 10) {
            CHECK(loop.entityCount() == startCount - 1);
            CHECK_FALSE(scene.registry->valid(puff));
        }
    }

    CHECK(loop.transformsFinite());
    CHECK(loop.entityCount() == startCount - 1);
    REQUIRE(scene.registry->valid(mover));
    const auto& moved = scene.registry->get<engine::TransformComponent>(mover);
    CHECK(moved.position.x == Catch::Approx(30.0f * kDt * kFrames).margin(1.0f));
    CHECK(moved.position.y == Catch::Approx(0.0f).margin(0.1f));
}

TEST_CASE("Headless hideout simulation keeps the player stable") {
    const auto mapPath = assetsDir() / "data" / "map_hideout.json";
    const auto archetypesPath = assetsDir() / "data" / "archetypes.json";
    if (!std::filesystem::is_regular_file(mapPath) || !std::filesystem::is_regular_file(archetypesPath)) {
        SKIP("hideout map or archetypes.json is not available");
    }

    baja::test::HeadlessScene scene;
    auto archetypes = std::make_shared<engine::ArchetypeManager>(scene.logManager);
    REQUIRE(archetypes->loadArchetypes(archetypesPath.string()));
    auto factory = std::make_shared<engine::EntityFactory>(scene.context, archetypes);
    scene.context.m_entityFactory = factory;

    engine::MapLoader loader(scene.context);
    REQUIRE(loader.load(mapPath.string()));

    baja::test::HeadlessGameplayLoop loop(scene);
    loop.bodyCreation.update();

    entt::entity player = entt::null;
    for (auto entity : scene.registry->view<engine::MetaComponent, engine::TransformComponent>()) {
        if (scene.registry->get<engine::MetaComponent>(entity).name == "player") {
            player = entity;
            break;
        }
    }
    REQUIRE(player != entt::null);
    const auto startPos = scene.registry->get<engine::TransformComponent>(player).position;
    const int startCount = loop.entityCount();
    REQUIRE(startCount > 1);

    for (int frame = 0; frame < kFrames; ++frame) {
        loop.step(kDt);
        REQUIRE(loop.transformsFinite());
        CHECK(loop.entityCount() == startCount);
    }

    const auto& endPos = scene.registry->get<engine::TransformComponent>(player).position;
    CHECK(endPos.x == Catch::Approx(startPos.x).margin(0.5f));
    CHECK(endPos.y == Catch::Approx(startPos.y).margin(0.5f));
}

TEST_CASE("Headless hospital simulation stays finite with movement and stuck recovery") {
    const auto mapPath = assetsDir() / "data" / "map_hospital.json";
    const auto archetypesPath = assetsDir() / "data" / "archetypes.json";
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

    baja::test::HeadlessGameplayLoop loop(scene);
    loop.bodyCreation.update();
    game::validateLoadedPatrolWaypoints(scene.context);

    const int startCount = loop.entityCount();
    REQUIRE(startCount > 1);
    REQUIRE(loop.transformsFinite());

    for (int frame = 0; frame < kFrames; ++frame) {
        loop.step(kDt);
        REQUIRE(loop.transformsFinite());
    }

    CHECK(loop.transformsFinite());
    CHECK(loop.entityCount() >= startCount - 2);
}
