#include "HeadlessScene.hpp"
#include "engine/assets/MapLoader.hpp"
#include "engine/components/HalfCollisionComponent.hpp"
#include "engine/components/MetaComponent.hpp"
#include "engine/components/PendingPhysicsBodyComponent.hpp"
#include "engine/components/PhysicsBodyComponent.hpp"
#include "engine/components/TransformComponent.hpp"
#include "engine/components/WorldBoundsComponent.hpp"
#include "engine/ecs/ArchetypeManager.hpp"
#include "engine/ecs/EntityFactory.hpp"
#include "engine/physics/PhysicsBodyCreationSystem.hpp"

#include <box2d/box2d.h>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <filesystem>
#include <nlohmann/json.hpp>
#include <string>

#ifndef BAJA_ASSETS_DIR
#define BAJA_ASSETS_DIR ""
#endif

namespace {

    std::filesystem::path assetsDir() {
        const std::filesystem::path configured{BAJA_ASSETS_DIR};
        if (!configured.empty() && std::filesystem::is_directory(configured)) {
            return configured;
        }
        const std::filesystem::path fallbacks[] = {
            std::filesystem::current_path() / "assets",
            std::filesystem::current_path() / ".." / ".." / "assets",
            std::filesystem::current_path() / ".." / ".." / ".." / ".." / "assets",
        };
        for (const auto& path : fallbacks) {
            if (std::filesystem::is_directory(path)) {
                return path;
            }
        }
        return configured;
    }

    struct MapExpectations {
        float mapScale = 0.0f;
        int imageLayers = 0;
        int collisions = 0;
        int halfCollisions = 0;
        int entityObjects = 0;
        int skippedEntities = 0;
        int players = 0;
        float playerX = 0.0f;
        float playerY = 0.0f;
        float boundsWidth = 0.0f;
        float boundsHeight = 0.0f;
    };

    MapExpectations expectationsFromJson(const nlohmann::json& root, const engine::ArchetypeManager& archetypes) {
        MapExpectations expected;
        expected.mapScale = root.value("mapScale", engine::EngineContext::MAP_SCALE);

        if (!root.contains("layers") || !root["layers"].is_array()) {
            return expected;
        }

        for (const auto& layer : root["layers"]) {
            const std::string type = layer.value("type", "");
            const std::string name = layer.value("name", "");
            if (type == "imagelayer") {
                if (layer.value("visible", true)) {
                    ++expected.imageLayers;
                    const float width = layer.value("imagewidth", 0.0f);
                    const float height = layer.value("imageheight", 0.0f);
                    if (width > 0.0f && height > 0.0f) {
                        expected.boundsWidth = width * expected.mapScale;
                        expected.boundsHeight = height * expected.mapScale;
                    }
                }
                continue;
            }
            if (type != "objectgroup" || !layer.contains("objects") || !layer["objects"].is_array()) {
                continue;
            }
            if (name == "Collision") {
                expected.collisions = static_cast<int>(layer["objects"].size());
            } else if (name == "HalfCollision") {
                expected.halfCollisions = static_cast<int>(layer["objects"].size());
            } else if (name == "Entities") {
                for (const auto& obj : layer["objects"]) {
                    std::string archetype = obj.value("type", "");
                    if (archetype.empty()) {
                        archetype = obj.value("class", "");
                    }
                    if (archetype.empty()) {
                        archetype = obj.value("name", "");
                    }
                    if (archetype.empty()) {
                        ++expected.skippedEntities;
                        continue;
                    }
                    if (archetypes.getArchetype(archetype) == nullptr) {
                        ++expected.skippedEntities;
                        continue;
                    }
                    ++expected.entityObjects;
                    if (archetype == "player") {
                        ++expected.players;
                        const float scale = expected.mapScale;
                        const float x = obj.value("x", 0.0f) * scale;
                        const float y = obj.value("y", 0.0f) * scale;
                        const float w = obj.value("width", 0.0f) * scale;
                        const float h = obj.value("height", 0.0f) * scale;
                        expected.playerX = x + w * 0.5f;
                        expected.playerY = y + h * 0.5f;
                    }
                }
            }
        }
        return expected;
    }

    int count(entt::registry& registry) {
        int n = 0;
        for (auto entity : registry.storage<entt::entity>()) {
            if (registry.valid(entity)) {
                ++n;
            }
        }
        return n;
    }

    template<typename... Components>
    int countWith(entt::registry& registry) {
        int n = 0;
        for (auto entity : registry.view<Components...>()) {
            (void)entity;
            ++n;
        }
        return n;
    }

} // namespace

TEST_CASE("MapLoader loads the shipped Tiled maps into a headless registry") {
    const auto assets = assetsDir();
    const auto archetypesPath = assets / "data" / "archetypes.json";
    if (!std::filesystem::is_regular_file(archetypesPath)) {
        SKIP("assets/data/archetypes.json is not available");
    }

    const std::string mapName = GENERATE(std::string{"map_hideout.json"}, std::string{"map_hospital.json"});
    const auto mapPath = assets / "data" / mapName;
    if (!std::filesystem::is_regular_file(mapPath)) {
        SKIP("map JSON is not available: " + mapName);
    }

    nlohmann::json mapJson;
    std::string error;
    REQUIRE(engine::readMapJsonFile(mapPath.string(), mapJson, &error));
    REQUIRE(engine::isTiledMapJson(mapJson));

    baja::test::HeadlessScene scene;
    auto archetypes = std::make_shared<engine::ArchetypeManager>(scene.logManager);
    REQUIRE(archetypes->loadArchetypes(archetypesPath.string()));
    auto factory = std::make_shared<engine::EntityFactory>(scene.context, archetypes);
    scene.context.m_entityFactory = factory;

    const auto expected = expectationsFromJson(mapJson, *archetypes);
    REQUIRE(expected.players == 1);
    REQUIRE(expected.collisions > 0);
    REQUIRE(expected.halfCollisions > 0);
    REQUIRE(expected.entityObjects > 0);

    engine::MapLoader loader(scene.context);
    REQUIRE(loader.load(mapPath.string()));
    CHECK(scene.context.mapScale == Catch::Approx(expected.mapScale));

    CHECK(countWith<engine::WorldBoundsComponent>(*scene.registry) == expected.imageLayers);
    if (expected.boundsWidth > 0.0f) {
        auto boundsView = scene.registry->view<engine::WorldBoundsComponent>();
        auto entity = *boundsView.begin();
        const auto& bounds = boundsView.get<engine::WorldBoundsComponent>(entity);
        CHECK(bounds.width == Catch::Approx(expected.boundsWidth));
        CHECK(bounds.height == Catch::Approx(expected.boundsHeight));
    }

    CHECK(countWith<engine::HalfCollisionComponent>(*scene.registry) == expected.halfCollisions);

    int wallCount = 0;
    int pendingCount = 0;
    for (auto entity : scene.registry->view<engine::PendingPhysicsBodyComponent>()) {
        ++pendingCount;
        if (!scene.registry->all_of<engine::HalfCollisionComponent>(entity) &&
            !scene.registry->all_of<engine::MetaComponent>(entity)) {
            ++wallCount;
        }
    }
    CHECK(wallCount == expected.collisions);

    int players = 0;
    int namedEntities = 0;
    for (auto entity : scene.registry->view<engine::MetaComponent, engine::TransformComponent>()) {
        ++namedEntities;
        const auto& meta = scene.registry->get<engine::MetaComponent>(entity);
        if (meta.name == "player") {
            ++players;
            const auto& pos = scene.registry->get<engine::TransformComponent>(entity).position;
            CHECK(pos.x == Catch::Approx(expected.playerX));
            CHECK(pos.y == Catch::Approx(expected.playerY));
        }
    }
    CHECK(players == 1);
    CHECK(namedEntities == expected.entityObjects);

    const int totalBeforeBodies = count(*scene.registry);
    CHECK(totalBeforeBodies ==
          expected.imageLayers + expected.collisions + expected.halfCollisions + expected.entityObjects);

    engine::PhysicsBodyCreationSystem physics(scene.context);
    physics.update();

    CHECK(countWith<engine::PendingPhysicsBodyComponent>(*scene.registry) == 0);
    CHECK(countWith<engine::PhysicsBodyComponent>(*scene.registry) == pendingCount);
    CHECK(count(*scene.registry) == totalBeforeBodies);

    int validBodies = 0;
    for (auto entity : scene.registry->view<engine::PhysicsBodyComponent>()) {
        const auto body = scene.registry->get<engine::PhysicsBodyComponent>(entity).bodyId;
        if (b2Body_IsValid(body)) {
            ++validBodies;
        }
    }
    CHECK(validBodies == pendingCount);
}
