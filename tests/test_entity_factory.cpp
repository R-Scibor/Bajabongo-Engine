#include "HeadlessScene.hpp"
#include "engine/components/AnimationComponent.hpp"
#include "engine/components/AnimationStateComponent.hpp"
#include "engine/components/CameraFocusComponent.hpp"
#include "engine/components/ChildComponent.hpp"
#include "engine/components/MetaComponent.hpp"
#include "engine/components/ParentComponent.hpp"
#include "engine/components/PendingPhysicsBodyComponent.hpp"
#include "engine/components/RenderableComponent.hpp"
#include "engine/components/TransformComponent.hpp"
#include "engine/ecs/ArchetypeManager.hpp"
#include "engine/ecs/EntityFactory.hpp"
#include "engine/physics/PhysicsConstants.hpp"

#include <catch2/catch_test_macros.hpp>

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

    struct FactoryScene : baja::test::HeadlessScene {
        std::shared_ptr<engine::ArchetypeManager> archetypes = std::make_shared<engine::ArchetypeManager>(logManager);
        std::shared_ptr<engine::EntityFactory> factory;

        explicit FactoryScene(const std::filesystem::path& archetypePath) {
            REQUIRE(archetypes->loadArchetypes(archetypePath.string()));
            factory = std::make_shared<engine::EntityFactory>(context, archetypes);
            context.m_entityFactory = factory;
        }
    };

} // namespace

TEST_CASE("EntityFactory returns null for an unknown archetype") {
    auto path = writeTemp("baja_factory_empty.json", "{}");
    FactoryScene scene(path);
    CHECK(scene.factory->spawn("missing", {1.0f, 2.0f}) == entt::null);
    CHECK(scene.registry->view<engine::TransformComponent>().begin() ==
          scene.registry->view<engine::TransformComponent>().end());
}

TEST_CASE("EntityFactory applies transform, renderable, animation, and camera data") {
    auto path = writeTemp("baja_factory_visual.json", R"({
        "unit": {
            "Transform": { "rotation": 0.5, "scale": [2.0, 3.0] },
            "Renderable": { "spriteId": "unit_sprite", "layer": 4, "color": [10, 20, 30, 40] },
            "Animation": { "defaultClip": "unit_idle", "playing": true },
            "AnimationState": { "animationSetId": "unit", "initialState": "Walk", "initialFacing": "Left" },
            "CameraFocus": { "viewHeight": 720.0, "smoothness": 0.25 }
        }
    })");
    FactoryScene scene(path);
    auto entity = scene.factory->spawn("unit", {8.0f, 12.0f});
    REQUIRE(entity != entt::null);

    CHECK(scene.registry->get<engine::MetaComponent>(entity).name == "unit");
    const auto& transform = scene.registry->get<engine::TransformComponent>(entity);
    CHECK(transform.position.x == 8.0f);
    CHECK(transform.position.y == 12.0f);
    CHECK(transform.rotation == 0.5f);
    CHECK(transform.scale.x == 2.0f);
    CHECK(transform.scale.y == 3.0f);

    const auto& renderable = scene.registry->get<engine::RenderableComponent>(entity);
    CHECK(renderable.spriteId == "unit_sprite");
    CHECK(renderable.layer == 4);
    CHECK(renderable.color.r == 10);
    CHECK(renderable.color.g == 20);
    CHECK(renderable.color.b == 30);
    CHECK(renderable.color.a == 40);

    CHECK(scene.registry->get<engine::AnimationComponent>(entity).currentClipId == "unit_idle");
    CHECK(scene.registry->get<engine::AnimationComponent>(entity).isPlaying);
    CHECK(scene.registry->get<engine::AnimationStateComponent>(entity).animationSetId == "unit");
    CHECK(scene.registry->get<engine::AnimationStateComponent>(entity).state == engine::AnimationState::Walk);
    CHECK(scene.registry->get<engine::AnimationStateComponent>(entity).facing == engine::FacingDirection::Left);
    CHECK(scene.registry->get<engine::CameraFocusComponent>(entity).viewHeight == 720.0f);
    CHECK(scene.registry->get<engine::CameraFocusComponent>(entity).smoothness == 0.25f);
}

TEST_CASE("EntityFactory skips an empty sprite id and keeps a bad component from aborting spawn") {
    auto path = writeTemp("baja_factory_partial.json", R"({
        "partial": {
            "Renderable": { "spriteId": "", "layer": 1 },
            "CameraFocus": "not-an-object",
            "UnknownThing": { "ok": true }
        }
    })");
    FactoryScene scene(path);
    auto entity = scene.factory->spawn("partial", {0.0f, 0.0f});
    REQUIRE(entity != entt::null);
    CHECK_FALSE(scene.registry->all_of<engine::RenderableComponent>(entity));
    CHECK_FALSE(scene.registry->all_of<engine::CameraFocusComponent>(entity));
    CHECK(scene.registry->all_of<engine::TransformComponent>(entity));
}

TEST_CASE("EntityFactory uses a fallback physics size and a single fixture when fixtures are omitted") {
    auto path = writeTemp("baja_factory_phys_simple.json", R"({
        "crate": {
            "Physics": {
                "type": "static",
                "size": [0.0, 0.0],
                "isSensor": true,
                "density": 2.0,
                "fixedRotation": true,
                "linearDamping": 1.5,
                "category": "Wall",
                "blocksVision": true
            }
        }
    })");
    FactoryScene scene(path);
    auto entity = scene.factory->spawn("crate", {4.0f, 6.0f});
    REQUIRE(entity != entt::null);
    const auto& pending = scene.registry->get<engine::PendingPhysicsBodyComponent>(entity);
    CHECK(pending.position.x == 4.0f);
    CHECK(pending.position.y == 6.0f);
    CHECK(pending.isStatic);
    CHECK(pending.fixedRotation);
    CHECK(pending.linearDamping == 1.5f);
    REQUIRE(pending.fixtures.size() == 1);
    CHECK(pending.fixtures[0].size.x == 32.0f);
    CHECK(pending.fixtures[0].size.y == 32.0f);
    CHECK(pending.fixtures[0].isSensor);
    CHECK(pending.fixtures[0].density == 2.0f);
    CHECK((pending.fixtures[0].categoryBits & engine::PhysicsCategory::Wall) != 0);
    CHECK((pending.fixtures[0].categoryBits & engine::PhysicsCategory::VisibilityBlocker) != 0);
}

TEST_CASE("EntityFactory maps fixture categories including Hurtbox and PlayerHurtbox") {
    auto path = writeTemp("baja_factory_phys_fixtures.json", R"({
        "actor": {
            "Physics": {
                "type": "dynamic",
                "size": [10.0, 10.0],
                "category": "Player",
                "fixtures": [
                    { "size": [40.0, 16.0], "offset": [0.0, -5.0], "category": "Player" },
                    { "size": [36.0, 125.0], "offset": [0.0, -62.0], "density": 0.0, "isSensor": true, "category": "Hurtbox" },
                    { "size": [8.0, 8.0], "category": "PlayerHurtbox" },
                    { "size": [9.0, 9.0], "category": "Enemy" },
                    { "size": [10.0, 10.0], "category": "Projectile" },
                    { "size": [11.0, 11.0], "category": "LowObstacle" },
                    { "size": [12.0, 12.0], "category": "Sensor" },
                    { "size": [13.0, 13.0], "category": "Wall", "blocksVision": true }
                ]
            }
        }
    })");
    FactoryScene scene(path);
    auto entity = scene.factory->spawn("actor", {0.0f, 0.0f});
    REQUIRE(entity != entt::null);
    const auto& pending = scene.registry->get<engine::PendingPhysicsBodyComponent>(entity);
    CHECK_FALSE(pending.isStatic);
    REQUIRE(pending.fixtures.size() == 8);
    CHECK(pending.fixtures[0].categoryBits == engine::PhysicsCategory::Player);
    CHECK(pending.fixtures[1].categoryBits == engine::PhysicsCategory::Hurtbox);
    CHECK(pending.fixtures[1].isSensor);
    CHECK(pending.fixtures[1].maskBits == engine::PhysicsCategory::Projectile);
    CHECK(pending.fixtures[2].categoryBits == engine::PhysicsCategory::Hurtbox);
    CHECK(pending.fixtures[3].categoryBits == engine::PhysicsCategory::Enemy);
    CHECK(pending.fixtures[4].categoryBits == engine::PhysicsCategory::Projectile);
    CHECK(pending.fixtures[5].categoryBits == engine::PhysicsCategory::LowObstacle);
    CHECK(pending.fixtures[6].categoryBits == engine::PhysicsCategory::Sensor);
    CHECK((pending.fixtures[7].categoryBits & engine::PhysicsCategory::Wall) != 0);
    CHECK((pending.fixtures[7].categoryBits & engine::PhysicsCategory::VisibilityBlocker) != 0);
}

TEST_CASE("EntityFactory spawns children and records parent and child links") {
    auto path = writeTemp("baja_factory_children.json", R"({
        "prop": { "Transform": { "rotation": 0.0 } },
        "parent": {
            "Transform": { "scale": [1.0, 1.0] },
            "children": [
                { "archetype": "prop", "offset": [5.0, -2.0], "rotation": 0.25 },
                { "archetype": "" }
            ]
        }
    })");
    FactoryScene scene(path);
    auto parent = scene.factory->spawn("parent", {10.0f, 20.0f});
    REQUIRE(parent != entt::null);
    const auto& children = scene.registry->get<engine::ChildComponent>(parent);
    REQUIRE(children.children.size() == 1);
    auto child = children.children[0];
    REQUIRE(scene.registry->valid(child));
    const auto& link = scene.registry->get<engine::ParentComponent>(child);
    CHECK(link.parentId == parent);
    CHECK(link.localPosition.x == 5.0f);
    CHECK(link.localPosition.y == -2.0f);
    CHECK(link.localRotation == 0.25f);
}

TEST_CASE("EntityFactory honors a custom component loader") {
    auto path = writeTemp("baja_factory_custom.json", R"({
        "tagged": { "Tag": { "label": "ok" } }
    })");
    FactoryScene scene(path);
    bool loaded = false;
    scene.factory->registerComponentLoader("Tag", [&](entt::registry&, entt::entity, const nlohmann::json& data) {
        loaded = (data.value("label", "") == "ok");
    });
    auto entity = scene.factory->spawn("tagged", {0.0f, 0.0f});
    REQUIRE(entity != entt::null);
    CHECK(loaded);
}

TEST_CASE("EntityFactory uses default animation facing and camera values") {
    auto path = writeTemp("baja_factory_defaults.json", R"({
        "npc": {
            "AnimationState": { "animationSetId": "npc" },
            "CameraFocus": {}
        }
    })");
    FactoryScene scene(path);
    auto entity = scene.factory->spawn("npc", {0.0f, 0.0f});
    REQUIRE(entity != entt::null);
    CHECK(scene.registry->get<engine::AnimationStateComponent>(entity).state == engine::AnimationState::Idle);
    CHECK(scene.registry->get<engine::AnimationStateComponent>(entity).facing == engine::FacingDirection::Right);
    CHECK(scene.registry->get<engine::CameraFocusComponent>(entity).viewHeight == 1080.0f);
    CHECK(scene.registry->get<engine::CameraFocusComponent>(entity).smoothness == 0.1f);
}
