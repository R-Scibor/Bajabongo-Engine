#include "engine/core/math/MathAliases.hpp"
#include "engine/core/math/MathJson.hpp"

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

TEST_CASE("Vector2 JSON round-trips x and y") {
    const engine::Vector2f original{1.5f, -2.25f};
    const nlohmann::json j = original;
    CHECK(j.at("x") == 1.5f);
    CHECK(j.at("y") == -2.25f);
    const auto restored = j.get<engine::Vector2f>();
    CHECK(restored.x == original.x);
    CHECK(restored.y == original.y);
}

TEST_CASE("Rect JSON round-trips all four fields") {
    const engine::Recti original{2, 4, 8, 16};
    const nlohmann::json j = original;
    const auto restored = j.get<engine::Recti>();
    CHECK(restored.left == 2);
    CHECK(restored.top == 4);
    CHECK(restored.width == 8);
    CHECK(restored.height == 16);
}

TEST_CASE("Vector2 JSON parse rejects a missing field") {
    const nlohmann::json j = {{"x", 1.0f}};
    CHECK_THROWS_AS(j.get<engine::Vector2f>(), nlohmann::json::out_of_range);
}
