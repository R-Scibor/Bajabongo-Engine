#include "HeadlessScene.hpp"
#include "engine/assets/MapLoader.hpp"
#include "engine/components/HalfCollisionComponent.hpp"
#include "engine/components/MetaComponent.hpp"
#include "engine/components/PendingPhysicsBodyComponent.hpp"
#include "engine/components/RenderableComponent.hpp"
#include "engine/components/TransformComponent.hpp"
#include "engine/components/VisibleToPlayerComponent.hpp"
#include "engine/components/WorldBoundsComponent.hpp"
#include "engine/ecs/ArchetypeManager.hpp"
#include "engine/ecs/EntityFactory.hpp"
#include "engine/physics/PhysicsConstants.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <filesystem>
#include <fstream>
#include <memory>

namespace {

    std::filesystem::path writeTemp(const std::string& name, const std::string& body) {
        auto path = std::filesystem::temp_directory_path() / name;
        std::ofstream out(path);
        out << body;
        out.close();
        return path;
    }

} // namespace

TEST_CASE("MapLoader returns false for a missing file and invalid JSON") {
    baja::test::HeadlessScene scene;
    engine::MapLoader loader(scene.context);
    CHECK_FALSE(loader.load("/tmp/does-not-exist-bajabongo-map.json"));

    auto bad = writeTemp("baja_map_bad.json", "{ not json");
    CHECK_FALSE(loader.load(bad.string()));
}

TEST_CASE("MapLoader applies mapScale, image layer, collisions, and entity spawn") {
    auto archetypes = writeTemp("baja_map_archetypes.json", R"({
        "player": { "Transform": { "scale": [1.0, 1.0] } },
        "crate": { "Transform": {} }
    })");

    baja::test::HeadlessScene scene;
    auto manager = std::make_shared<engine::ArchetypeManager>(scene.logManager);
    REQUIRE(manager->loadArchetypes(archetypes.string()));
    auto factory = std::make_shared<engine::EntityFactory>(scene.context, manager);
    scene.context.m_entityFactory = factory;

    auto mapPath = writeTemp("baja_map_ok.json", R"({
        "mapScale": 2.0,
        "layers": [
            {
                "type": "imagelayer",
                "name": "Image Layer",
                "visible": true,
                "opacity": 0.5,
                "image": "../textures/map_hideout.png",
                "imagewidth": 100,
                "imageheight": 50
            },
            {
                "type": "imagelayer",
                "name": "Hidden",
                "visible": false,
                "image": "unused.png"
            },
            {
                "type": "objectgroup",
                "name": "Collision",
                "objects": [
                    { "x": 10, "y": 20, "width": 30, "height": 40, "rotation": 90 }
                ]
            },
            {
                "type": "objectgroup",
                "name": "HalfCollision",
                "objects": [
                    { "x": 0, "y": 0, "width": 8, "height": 8 }
                ]
            },
            {
                "type": "objectgroup",
                "name": "Entities",
                "objects": [
                    { "type": "player", "x": 5, "y": 7, "width": 0, "height": 0 },
                    { "class": "crate", "x": 1, "y": 2, "width": 4, "height": 6 },
                    { "name": "crate", "x": 0, "y": 0 },
                    { "x": 9, "y": 9 },
                    { "type": "missing", "x": 3, "y": 3 }
                ]
            },
            {
                "type": "objectgroup",
                "name": "Ignored",
                "objects": [ { "x": 1, "y": 1, "width": 1, "height": 1 } ]
            }
        ]
    })");

    engine::MapLoader loader(scene.context);
    REQUIRE(loader.load(mapPath.string()));
    CHECK(scene.context.mapScale == Catch::Approx(2.0f));

    auto backgrounds =
        scene.registry
            ->view<engine::WorldBoundsComponent, engine::RenderableComponent, engine::VisibleToPlayerComponent>();
    int backgroundCount = 0;
    for (auto entity : backgrounds) {
        ++backgroundCount;
        const auto& bounds = backgrounds.get<engine::WorldBoundsComponent>(entity);
        CHECK(bounds.width == Catch::Approx(200.0f));
        CHECK(bounds.height == Catch::Approx(100.0f));
        const auto& renderable = backgrounds.get<engine::RenderableComponent>(entity);
        CHECK(renderable.spriteId == "map_hideout_sprite");
        CHECK(renderable.color.a == 127);
        CHECK(backgrounds.get<engine::VisibleToPlayerComponent>(entity).alwaysVisible);
        const auto& transform = scene.registry->get<engine::TransformComponent>(entity);
        CHECK(transform.scale.x == Catch::Approx(2.0f));
        CHECK(transform.scale.y == Catch::Approx(2.0f));
    }
    CHECK(backgroundCount == 1);

    int wallCount = 0;
    int halfCount = 0;
    for (auto entity : scene.registry->view<engine::PendingPhysicsBodyComponent>()) {
        const auto& pending = scene.registry->get<engine::PendingPhysicsBodyComponent>(entity);
        if (scene.registry->all_of<engine::HalfCollisionComponent>(entity)) {
            ++halfCount;
            CHECK(pending.fixtures[0].categoryBits == engine::PhysicsCategory::LowObstacle);
            CHECK(pending.position.x == Catch::Approx(8.0f));
            CHECK(pending.position.y == Catch::Approx(8.0f));
        } else {
            ++wallCount;
            CHECK((pending.fixtures[0].categoryBits & engine::PhysicsCategory::Wall) != 0);
            CHECK(pending.position.x == Catch::Approx(50.0f));
            CHECK(pending.position.y == Catch::Approx(80.0f));
            CHECK(pending.rotation == Catch::Approx(90.0f * (3.14159f / 180.0f)));
        }
    }
    CHECK(wallCount == 1);
    CHECK(halfCount == 1);

    int players = 0;
    int crates = 0;
    for (auto entity : scene.registry->view<engine::MetaComponent>()) {
        const auto& name = scene.registry->get<engine::MetaComponent>(entity).name;
        if (name == "player") {
            ++players;
            const auto& pos = scene.registry->get<engine::TransformComponent>(entity).position;
            CHECK(pos.x == Catch::Approx(10.0f));
            CHECK(pos.y == Catch::Approx(14.0f));
        } else if (name == "crate") {
            ++crates;
        }
    }
    CHECK(players == 1);
    CHECK(crates == 2);
}

TEST_CASE("MapLoader skips entity objects when no factory is attached") {
    baja::test::HeadlessScene scene;
    auto mapPath = writeTemp("baja_map_nofactory.json", R"({
        "layers": [
            {
                "type": "objectgroup",
                "name": "Entities",
                "objects": [ { "type": "player", "x": 1, "y": 2 } ]
            }
        ]
    })");
    engine::MapLoader loader(scene.context);
    REQUIRE(loader.load(mapPath.string()));
    CHECK(scene.context.mapScale == Catch::Approx(engine::EngineContext::MAP_SCALE));
    CHECK(scene.registry->view<engine::TransformComponent>().begin() ==
          scene.registry->view<engine::TransformComponent>().end());
}

TEST_CASE("MapLoader still succeeds when layers are omitted") {
    baja::test::HeadlessScene scene;
    auto mapPath = writeTemp("baja_map_nolayers.json", R"({ "mapScale": 1.5 })");
    engine::MapLoader loader(scene.context);
    REQUIRE(loader.load(mapPath.string()));
    CHECK(scene.context.mapScale == Catch::Approx(1.5f));
}
