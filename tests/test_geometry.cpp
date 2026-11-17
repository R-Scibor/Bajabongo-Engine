#include "engine/core/math/GeometryUtils.hpp"

#include <catch2/catch_test_macros.hpp>
#include <vector>

TEST_CASE("isPointInPolygon rejects empty or degenerate input") {
    CHECK_FALSE(engine::GeometryUtils::isPointInPolygon({0.0f, 0.0f}, {}));
    CHECK_FALSE(engine::GeometryUtils::isPointInPolygon({0.0f, 0.0f}, {{0.0f, 0.0f}, {1.0f, 0.0f}}));
}

TEST_CASE("isPointInPolygon accepts a point clearly inside a square") {
    const std::vector<engine::Vector2f> square{{0.0f, 0.0f}, {10.0f, 0.0f}, {10.0f, 10.0f}, {0.0f, 10.0f}};
    CHECK(engine::GeometryUtils::isPointInPolygon({5.0f, 5.0f}, square));
}

TEST_CASE("isPointInPolygon rejects a point clearly outside a square") {
    const std::vector<engine::Vector2f> square{{0.0f, 0.0f}, {10.0f, 0.0f}, {10.0f, 10.0f}, {0.0f, 10.0f}};
    CHECK_FALSE(engine::GeometryUtils::isPointInPolygon({15.0f, 5.0f}, square));
    CHECK_FALSE(engine::GeometryUtils::isPointInPolygon({-1.0f, -1.0f}, square));
}

TEST_CASE("isPointInPolygon accepts a point inside a triangle") {
    const std::vector<engine::Vector2f> triangle{{0.0f, 0.0f}, {4.0f, 0.0f}, {0.0f, 4.0f}};
    CHECK(engine::GeometryUtils::isPointInPolygon({1.0f, 1.0f}, triangle));
    CHECK_FALSE(engine::GeometryUtils::isPointInPolygon({3.0f, 3.0f}, triangle));
}
