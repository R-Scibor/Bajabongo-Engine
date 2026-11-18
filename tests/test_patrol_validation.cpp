#include "HeadlessScene.hpp"
#include "engine/core/math/Random.hpp"
#include "engine/physics/PhysicsConstants.hpp"
#include "game/ai/PatrolWaypointValidation.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <cmath>

using Catch::Approx;

TEST_CASE("validatePatrolWaypoints keeps a clear candidate") {
    baja::test::HeadlessScene scene;
    engine::Random rng{1};
    game::PatrolValidationLimits limits;
    const engine::Vector2f spawn{0.0f, 0.0f};
    const auto world = game::validatePatrolWaypoints(scene.worldId, spawn, {{100.0f, 0.0f}}, rng, limits);
    REQUIRE(world.size() == 1);
    CHECK(world[0].x == Approx(100.0f).margin(0.2f));
    CHECK(world[0].y == Approx(0.0f).margin(0.2f));
}

TEST_CASE("validatePatrolWaypoints shortens a candidate that hits a wall") {
    baja::test::HeadlessScene scene;
    // Wall occupies x in [45, 55], blocking a path to x=100.
    scene.addStaticBox(50.0f, 0.0f, 5.0f, 40.0f, engine::PhysicsCategory::Wall);

    engine::Random rng{1};
    game::PatrolValidationLimits limits;
    limits.halfWidth = 16.0f;
    limits.halfHeight = 8.0f;
    limits.hitMargin = 8.0f;
    limits.minAdvance = 16.0f;

    const auto world = game::validatePatrolWaypoints(scene.worldId, {0.0f, 0.0f}, {{100.0f, 0.0f}}, rng, limits);
    REQUIRE_FALSE(world.empty());
    CHECK(world[0].x < 45.0f - limits.halfWidth);
    CHECK(world[0].x > 16.0f);
    CHECK_FALSE(game::ai::areaBlockedByWalls(scene.worldId, world[0], limits.halfWidth, limits.halfHeight));
}

TEST_CASE("validatePatrolWaypoints falls back near the spawn when boxed in") {
    baja::test::HeadlessScene scene;
    // Tight box around the origin, closer than minAdvance after the feet half-extent.
    scene.addStaticBox(0.0f, -18.0f, 30.0f, 4.0f, engine::PhysicsCategory::Wall);
    scene.addStaticBox(0.0f, 18.0f, 30.0f, 4.0f, engine::PhysicsCategory::Wall);
    scene.addStaticBox(-18.0f, 0.0f, 4.0f, 30.0f, engine::PhysicsCategory::Wall);
    scene.addStaticBox(18.0f, 0.0f, 4.0f, 30.0f, engine::PhysicsCategory::Wall);

    engine::Random rng{1};
    game::PatrolValidationLimits limits;
    limits.halfWidth = 4.0f;
    limits.halfHeight = 4.0f;
    limits.minAdvance = 16.0f;
    limits.fallbackRadius = 32.0f;

    const engine::Vector2f spawn{0.0f, 0.0f};
    const auto world =
        game::validatePatrolWaypoints(scene.worldId, spawn, {{100.0f, 0.0f}, {0.0f, 100.0f}}, rng, limits);
    REQUIRE_FALSE(world.empty());
    const float dx = world[0].x - spawn.x;
    const float dy = world[0].y - spawn.y;
    const float dist = std::sqrt(dx * dx + dy * dy);
    CHECK(dist < 24.0f);
}

TEST_CASE("tryReachPatrolPoint rejects a run that ends inside a wall") {
    baja::test::HeadlessScene scene;
    scene.addStaticBox(0.0f, 0.0f, 20.0f, 20.0f, engine::PhysicsCategory::LowObstacle);

    game::PatrolValidationLimits limits;
    engine::Vector2f out{999.0f, 999.0f};
    CHECK_FALSE(game::tryReachPatrolPoint(scene.worldId, {0.0f, 0.0f}, {0.0f, 0.0f}, limits, out));
}
