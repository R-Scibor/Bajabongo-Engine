#include "engine/core/math/MathAliases.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Vector2 default constructor is a zero vector") {
    const engine::Vector2f z;
    CHECK(z.x == 0.0f);
    CHECK(z.y == 0.0f);
}

TEST_CASE("Vector2 addition and subtraction") {
    const engine::Vector2f a(3.0f, 4.0f);
    const engine::Vector2f b(1.0f, -2.0f);
    const auto sum = a + b;
    CHECK(sum.x == 4.0f);
    CHECK(sum.y == 2.0f);
    const auto diff = a - b;
    CHECK(diff.x == 2.0f);
    CHECK(diff.y == 6.0f);
}

TEST_CASE("Vector2 scalar multiply and divide") {
    const engine::Vector2f a(3.0f, -6.0f);
    const auto scaled = a * 2;
    CHECK(scaled.x == 6.0f);
    CHECK(scaled.y == -12.0f);
    const auto halved = a / 3.0f;
    CHECK(halved.x == 1.0f);
    CHECK(halved.y == -2.0f);
}

TEST_CASE("Vector2 compound assignment updates in place") {
    engine::Vector2i v(1, 2);
    v += engine::Vector2i(3, 4);
    CHECK(v.x == 4);
    CHECK(v.y == 6);
    v -= engine::Vector2i(1, 1);
    CHECK(v.x == 3);
    CHECK(v.y == 5);
}
