#include "game/ai/PatrolWaypoints.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

TEST_CASE("parseWaypointList accepts a JSON array of points") {
    std::vector<engine::Vector2f> out;
    REQUIRE(game::parseWaypointList(nlohmann::json::parse("[[100, 200], [300, 200]]"), out));
    REQUIRE(out.size() == 2);
    CHECK(out[0].x == Catch::Approx(100.0f));
    CHECK(out[0].y == Catch::Approx(200.0f));
    CHECK(out[1].x == Catch::Approx(300.0f));
    CHECK(out[1].y == Catch::Approx(200.0f));
}

TEST_CASE("parseWaypointList accepts a JSON string and a semicolon list") {
    std::vector<engine::Vector2f> fromJson;
    REQUIRE(game::parseWaypointList(nlohmann::json("[[10, 20], {\"x\": 30, \"y\": 40}]"), fromJson));
    REQUIRE(fromJson.size() == 2);
    CHECK(fromJson[1].x == Catch::Approx(30.0f));
    CHECK(fromJson[1].y == Catch::Approx(40.0f));

    std::vector<engine::Vector2f> fromCsv;
    REQUIRE(game::parseWaypointList(nlohmann::json("1.5,2.5;3,4"), fromCsv));
    REQUIRE(fromCsv.size() == 2);
    CHECK(fromCsv[0].x == Catch::Approx(1.5f));
    CHECK(fromCsv[0].y == Catch::Approx(2.5f));
}

TEST_CASE("resolvePatrolWaypoints treats archetype points as scaled spawn offsets") {
    const engine::Vector2f spawn{2321.0f, 2828.0f};
    game::PatrolWaypointSpec spec;
    spec.raw = {{100.0f, 200.0f}, {300.0f, 200.0f}};
    const auto world = game::resolvePatrolWaypoints(spawn, 2.0f, spec);
    REQUIRE(world.size() == 2);
    CHECK(world[0].x == Catch::Approx(2521.0f));
    CHECK(world[0].y == Catch::Approx(3228.0f));
    CHECK(world[1].x == Catch::Approx(2921.0f));
    CHECK(world[1].y == Catch::Approx(3228.0f));
}

TEST_CASE("resolvePatrolWaypoints scales absolute map points without adding spawn") {
    game::PatrolWaypointSpec spec;
    spec.raw = {{100.0f, 200.0f}};
    spec.absolute = true;
    const auto world = game::resolvePatrolWaypoints({999.0f, 999.0f}, 2.0f, spec);
    REQUIRE(world.size() == 1);
    CHECK(world[0].x == Catch::Approx(200.0f));
    CHECK(world[0].y == Catch::Approx(400.0f));
}

TEST_CASE("resolvePatrolWaypoints builds a wander square when the list is empty") {
    const engine::Vector2f spawn{10.0f, 20.0f};
    game::PatrolWaypointSpec spec;
    spec.wanderRadius = 150.0f;
    const auto world = game::resolvePatrolWaypoints(spawn, 2.0f, spec);
    REQUIRE(world.size() == 4);
    CHECK(world[0].x == Catch::Approx(10.0f - 300.0f));
    CHECK(world[0].y == Catch::Approx(20.0f - 300.0f));
    CHECK(world[2].x == Catch::Approx(10.0f + 300.0f));
    CHECK(world[2].y == Catch::Approx(20.0f + 300.0f));
}

TEST_CASE("tryReadMapWaypoints reads a Tiled properties entry") {
    const auto obj = nlohmann::json::parse(R"({
        "type": "gopnik_patroller",
        "properties": [{"name": "waypoints", "type": "string", "value": "[[50, 60], [70, 80]]"}]
    })");
    std::vector<engine::Vector2f> out;
    REQUIRE(game::tryReadMapWaypoints(obj, out));
    REQUIRE(out.size() == 2);
    CHECK(out[0].x == Catch::Approx(50.0f));
    CHECK(out[1].y == Catch::Approx(80.0f));
}

TEST_CASE("skipToNextPatrolWaypoint loops around a path") {
    game::PatrolBehaviorComponent patrol;
    patrol.waypoints = {{0.0f, 0.0f}, {1.0f, 0.0f}, {2.0f, 0.0f}};
    patrol.loopMode = true;
    REQUIRE(game::skipToNextPatrolWaypoint(patrol));
    CHECK(patrol.currentWaypointIndex == 1);
    REQUIRE(game::skipToNextPatrolWaypoint(patrol));
    CHECK(patrol.currentWaypointIndex == 2);
    REQUIRE(game::skipToNextPatrolWaypoint(patrol));
    CHECK(patrol.currentWaypointIndex == 0);
}

TEST_CASE("skipToNextPatrolWaypoint ping-pongs and rejects a single point") {
    game::PatrolBehaviorComponent patrol;
    patrol.waypoints = {{0.0f, 0.0f}, {1.0f, 0.0f}, {2.0f, 0.0f}};
    patrol.loopMode = false;
    REQUIRE(game::skipToNextPatrolWaypoint(patrol));
    CHECK(patrol.currentWaypointIndex == 1);
    REQUIRE(game::skipToNextPatrolWaypoint(patrol));
    CHECK(patrol.currentWaypointIndex == 2);
    CHECK(patrol.reversing);
    REQUIRE(game::skipToNextPatrolWaypoint(patrol));
    CHECK(patrol.currentWaypointIndex == 1);
    REQUIRE(game::skipToNextPatrolWaypoint(patrol));
    CHECK(patrol.currentWaypointIndex == 0);
    CHECK_FALSE(patrol.reversing);

    game::PatrolBehaviorComponent single;
    single.waypoints = {{0.0f, 0.0f}};
    CHECK_FALSE(game::skipToNextPatrolWaypoint(single));
}
