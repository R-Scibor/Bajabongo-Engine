#include "HeadlessScene.hpp"
#include "engine/components/TransformComponent.hpp"
#include "engine/core/math/Angle.hpp"
#include "engine/physics/PhysicsConstants.hpp"
#include "game/components/VisibilityComponent.hpp"
#include "game/systems/VisibilityMath.hpp"
#include "game/systems/VisibilitySystem.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

using Catch::Approx;

TEST_CASE("viewHalfAngleRadians is half the field of view in radians") {
    CHECK(game::viewHalfAngleRadians(90.0f) == Approx(0.5f * engine::Pi / 2.0f).margin(0.0001f));
    CHECK(game::viewHalfAngleRadians(0.0f) == 0.0f);
}

TEST_CASE("visibilityOrigin adds the offset to the position") {
    engine::Vector2f origin = game::visibilityOrigin({10.0f, 2.0f}, {1.0f, -3.0f});
    CHECK(origin.x == 11.0f);
    CHECK(origin.y == -1.0f);
}

TEST_CASE("isAngleInViewCone includes edges and wraps across plus or minus pi") {
    const float half = game::viewHalfAngleRadians(90.0f);
    CHECK(game::isAngleInViewCone(0.0f, 0.0f, half));
    CHECK(game::isAngleInViewCone(half, 0.0f, half));
    CHECK(game::isAngleInViewCone(-half, 0.0f, half));
    CHECK_FALSE(game::isAngleInViewCone(engine::Pi, 0.0f, half));

    // Facing nearly -pi; an angle just past +pi should wrap into the cone.
    const float direction = -engine::Pi + 0.1f;
    CHECK(game::isAngleInViewCone(engine::Pi - 0.05f, direction, half));
}

TEST_CASE("isWithinMinViewRadius is inclusive") {
    engine::Vector2f origin{0.0f, 0.0f};
    CHECK(game::isWithinMinViewRadius({10.0f, 0.0f}, origin, 10.0f));
    CHECK(game::isWithinMinViewRadius({0.0f, 0.0f}, origin, 10.0f));
    CHECK_FALSE(game::isWithinMinViewRadius({10.1f, 0.0f}, origin, 10.0f));
}

TEST_CASE("shouldSampleWallVertex is true in the cone or inside the min radius") {
    const float half = game::viewHalfAngleRadians(90.0f);
    engine::Vector2f origin{0.0f, 0.0f};
    CHECK(game::shouldSampleWallVertex({20.0f, 0.0f}, origin, 0.0f, half, 10.0f));
    CHECK(game::shouldSampleWallVertex({0.0f, 5.0f}, origin, 0.0f, half, 10.0f));
    CHECK_FALSE(game::shouldSampleWallVertex({0.0f, 20.0f}, origin, 0.0f, half, 10.0f));
}

TEST_CASE("visibilityRayLength uses the full radius only inside the cone") {
    const float half = game::viewHalfAngleRadians(90.0f);
    CHECK(game::visibilityRayLength(0.0f, 0.0f, half, 50.0f, 10.0f) == 50.0f);
    CHECK(game::visibilityRayLength(engine::Pi, 0.0f, half, 50.0f, 10.0f) == 10.0f);
}

TEST_CASE("VisibilitySystem builds a closing polygon in an empty world") {
    baja::test::HeadlessScene scene;
    game::VisibilitySystem system(scene.context);

    auto entity = scene.registry->create();
    scene.registry->emplace<engine::TransformComponent>(entity).position = {0.0f, 0.0f};
    auto& visibility = scene.registry->emplace<game::VisibilityComponent>(entity);
    visibility.viewRadius = 50.0f;
    visibility.minViewRadius = 10.0f;
    visibility.viewAngle = 90.0f;
    visibility.viewDirection = 0.0f;

    system.update();
    REQUIRE(visibility.visibilityPolygon.size() > 2);
    CHECK(visibility.visibilityPolygon.front().x == 0.0f);
    CHECK(visibility.visibilityPolygon.front().y == 0.0f);
    CHECK(visibility.visibilityPolygon.back().x == visibility.visibilityPolygon[1].x);
    CHECK(visibility.visibilityPolygon.back().y == visibility.visibilityPolygon[1].y);
}

TEST_CASE("VisibilitySystem shortens rays that hit a wall") {
    baja::test::HeadlessScene scene;
    scene.addStaticBox(40.0f, 0.0f, 1.0f, 20.0f, engine::PhysicsCategory::VisibilityBlocker);
    game::VisibilitySystem system(scene.context);

    auto entity = scene.registry->create();
    scene.registry->emplace<engine::TransformComponent>(entity).position = {0.0f, 0.0f};
    auto& visibility = scene.registry->emplace<game::VisibilityComponent>(entity);
    visibility.viewRadius = 50.0f;
    visibility.minViewRadius = 10.0f;
    visibility.viewAngle = 90.0f;
    visibility.viewDirection = 0.0f;

    system.update();
    REQUIRE(visibility.visibilityPolygon.size() > 2);

    float maxX = visibility.visibilityPolygon.front().x;
    for (const auto& point : visibility.visibilityPolygon) {
        if (point.x > maxX) {
            maxX = point.x;
        }
    }
    CHECK(maxX > 30.0f);
    CHECK(maxX < 50.0f);
}
