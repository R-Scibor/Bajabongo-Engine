#include "HeadlessScene.hpp"
#include "engine/physics/PhysicsConstants.hpp"
#include "game/ai/AIUtils.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cmath>

using Catch::Approx;

TEST_CASE("HeadlessScene creates a valid Box2D world") {
    baja::test::HeadlessScene scene;
    CHECK(b2World_IsValid(scene.worldId));
}

TEST_CASE("CastRayClosest misses in an empty world") {
    baja::test::HeadlessScene scene;
    b2RayResult result = b2World_CastRayClosest(scene.worldId, {0.0f, 0.0f}, {50.0f, 0.0f}, b2DefaultQueryFilter());
    CHECK_FALSE(result.hit);
}

TEST_CASE("CastRayClosest hits a wall near x = 45") {
    baja::test::HeadlessScene scene;
    scene.addStaticBox(45.0f, 0.0f, 1.0f, 10.0f, engine::PhysicsCategory::Wall);
    b2QueryFilter filter = b2DefaultQueryFilter();
    filter.maskBits = engine::PhysicsCategory::Wall;
    b2RayResult result = b2World_CastRayClosest(scene.worldId, {0.0f, 0.0f}, {80.0f, 0.0f}, filter);
    REQUIRE(result.hit);
    CHECK(result.point.x == Approx(44.0f).margin(0.2f));
}

namespace {
    struct OverlapProbe {
        int hits = 0;
        static bool callback(b2ShapeId, void* context) {
            auto* probe = static_cast<OverlapProbe*>(context);
            ++probe->hits;
            return true;
        }
    };
} // namespace

TEST_CASE("OverlapAABB reports a box inside the query and ignores one outside it") {
    baja::test::HeadlessScene scene;
    scene.addStaticBox(0.0f, 0.0f, 2.0f, 2.0f, engine::PhysicsCategory::Wall);
    scene.addStaticBox(100.0f, 0.0f, 2.0f, 2.0f, engine::PhysicsCategory::Wall);

    b2AABB inside{};
    inside.lowerBound = {-5.0f, -5.0f};
    inside.upperBound = {5.0f, 5.0f};
    OverlapProbe insideProbe;
    b2World_OverlapAABB(scene.worldId, b2Pos_zero, inside, b2DefaultQueryFilter(), OverlapProbe::callback,
                        &insideProbe);
    CHECK(insideProbe.hits >= 1);

    b2AABB empty{};
    empty.lowerBound = {40.0f, -5.0f};
    empty.upperBound = {50.0f, 5.0f};
    OverlapProbe emptyProbe;
    b2World_OverlapAABB(scene.worldId, b2Pos_zero, empty, b2DefaultQueryFilter(), OverlapProbe::callback, &emptyProbe);
    CHECK(emptyProbe.hits == 0);
}

TEST_CASE("AIUtils hasLineOfSight is true across a short gap and false through a wall") {
    baja::test::HeadlessScene scene;
    CHECK(game::ai::AIUtils::hasLineOfSight(scene.context, {0.0f, 0.0f}, {0.0f, 0.0f}));
    CHECK(game::ai::AIUtils::hasLineOfSight(scene.context, {0.0f, 0.0f}, {5.0f, 0.0f}));

    scene.addStaticBox(25.0f, 0.0f, 2.0f, 20.0f,
                       engine::PhysicsCategory::VisibilityBlocker | engine::PhysicsCategory::Wall);
    CHECK_FALSE(game::ai::AIUtils::hasLineOfSight(scene.context, {0.0f, 0.0f}, {50.0f, 0.0f}));
}
