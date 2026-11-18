#include "HeadlessScene.hpp"
#include "engine/physics/PhysicsConstants.hpp"
#include "game/ai/EnemyNavigation.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

using Catch::Approx;
using game::ai::areaBlockedByWalls;
using game::ai::mostOpenDirection;
using game::ai::solidFootprint;

TEST_CASE("solidFootprint uses the offset feet collider") {
    baja::test::HeadlessScene scene;

    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = b2_dynamicBody;
    bodyDef.position = {100.0f, 100.0f};
    b2BodyId bodyId = b2CreateBody(scene.worldId, &bodyDef);

    b2Polygon box = b2MakeOffsetBox(20.0f, 8.0f, {0.0f, 60.0f}, b2MakeRot(0.0f));
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.filter.categoryBits = engine::PhysicsCategory::Default;
    shapeDef.filter.maskBits = 0xFFFFFFFFu;
    b2CreatePolygonShape(bodyId, &shapeDef, &box);

    const auto foot = solidFootprint(bodyId, {100.0f, 100.0f});
    REQUIRE(foot.found);
    CHECK(foot.worldCenter.x == Approx(100.0f).margin(0.2f));
    CHECK(foot.worldCenter.y == Approx(160.0f).margin(0.2f));
    CHECK(foot.halfWidth == Approx(20.0f).margin(0.2f));
    CHECK(foot.halfHeight == Approx(8.0f).margin(0.2f));
}

TEST_CASE("a feet-centered wall probe hits a wall that a sprite-origin probe misses") {
    baja::test::HeadlessScene scene;
    scene.addStaticBox(0.0f, 80.0f, 40.0f, 8.0f, engine::PhysicsCategory::Wall);

    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = b2_dynamicBody;
    bodyDef.position = {0.0f, 0.0f};
    b2BodyId bodyId = b2CreateBody(scene.worldId, &bodyDef);

    b2Polygon box = b2MakeOffsetBox(20.0f, 8.0f, {0.0f, 60.0f}, b2MakeRot(0.0f));
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.filter.categoryBits = engine::PhysicsCategory::Default;
    shapeDef.filter.maskBits = 0xFFFFFFFFu;
    b2CreatePolygonShape(bodyId, &shapeDef, &box);

    const auto foot = solidFootprint(bodyId, {0.0f, 0.0f});
    REQUIRE(foot.found);

    b2QueryFilter filter = b2DefaultQueryFilter();
    filter.maskBits = engine::PhysicsCategory::Wall;

    // Old slide ray: sprite origin plus a 16-unit buffer, length ~ velocity*dt*2.5
    b2RayResult shortRay = b2World_CastRayClosest(scene.worldId, {0.0f, 16.0f}, {0.0f, 3.3f}, filter);
    CHECK_FALSE(shortRay.hit);

    const engine::Vector2f dir{0.0f, 1.0f};
    const float lead = game::ai::halfExtentAlong(foot, dir) + 1.0f;
    const float lookahead = 48.0f;
    b2Vec2 origin = {foot.worldCenter.x + dir.x * lead, foot.worldCenter.y + dir.y * lead};
    b2Vec2 translation = {dir.x * lookahead, dir.y * lookahead};
    b2RayResult feetRay = b2World_CastRayClosest(scene.worldId, origin, translation, filter);
    REQUIRE(feetRay.hit);
}

TEST_CASE("mostOpenDirection prefers the unblocked side") {
    baja::test::HeadlessScene scene;
    scene.addStaticBox(80.0f, 0.0f, 10.0f, 40.0f, engine::PhysicsCategory::Wall);

    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = b2_dynamicBody;
    bodyDef.position = {0.0f, 0.0f};
    b2BodyId bodyId = b2CreateBody(scene.worldId, &bodyDef);
    b2Polygon box = b2MakeBox(16.0f, 8.0f);
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.filter.categoryBits = engine::PhysicsCategory::Default;
    shapeDef.filter.maskBits = 0xFFFFFFFFu;
    b2CreatePolygonShape(bodyId, &shapeDef, &box);

    const auto foot = solidFootprint(bodyId, {0.0f, 0.0f});
    const auto dir = mostOpenDirection(scene.worldId, foot, 80.0f);
    REQUIRE((dir.x != 0.0f || dir.y != 0.0f));
    CHECK(dir.x < 0.0f);
}

TEST_CASE("areaBlockedByWalls detects a wall overlap") {
    baja::test::HeadlessScene scene;
    scene.addStaticBox(50.0f, 0.0f, 10.0f, 10.0f, engine::PhysicsCategory::Wall);
    CHECK(areaBlockedByWalls(scene.worldId, {50.0f, 0.0f}, 8.0f, 8.0f));
    CHECK_FALSE(areaBlockedByWalls(scene.worldId, {200.0f, 0.0f}, 8.0f, 8.0f));
}
