#include "engine/core/math/MathAliases.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Rect default and parameterized constructors") {
    const engine::Rectf zero;
    CHECK(zero.left == 0.0f);
    CHECK(zero.top == 0.0f);
    CHECK(zero.width == 0.0f);
    CHECK(zero.height == 0.0f);

    const engine::Recti box(10, 20, 30, 40);
    CHECK(box.left == 10);
    CHECK(box.top == 20);
    CHECK(box.width == 30);
    CHECK(box.height == 40);
    CHECK(engine::rectRight(box) == 40);
    CHECK(engine::rectBottom(box) == 60);
}

TEST_CASE("contains reports interior and inclusive edges") {
    const engine::Rectf box(0.0f, 0.0f, 10.0f, 5.0f);
    CHECK(engine::contains(box, 5.0f, 2.5f));
    CHECK(engine::contains(box, 0.0f, 0.0f));
    CHECK(engine::contains(box, 10.0f, 5.0f));
    CHECK(engine::contains(box, engine::Vector2f{10.0f, 0.0f}));
    CHECK_FALSE(engine::contains(box, -0.01f, 2.0f));
    CHECK_FALSE(engine::contains(box, 5.0f, 5.01f));
}

TEST_CASE("intersects detects overlap, edge touch, and separation") {
    const engine::Rectf a(0.0f, 0.0f, 10.0f, 10.0f);
    const engine::Rectf overlap(5.0f, 5.0f, 10.0f, 10.0f);
    const engine::Rectf touch(10.0f, 0.0f, 4.0f, 4.0f);
    const engine::Rectf apart(11.0f, 0.0f, 4.0f, 4.0f);
    const engine::Rectf inside(2.0f, 2.0f, 1.0f, 1.0f);

    CHECK(engine::intersects(a, overlap));
    CHECK(engine::intersects(overlap, a));
    CHECK(engine::intersects(a, touch));
    CHECK(engine::intersects(a, a));
    CHECK(engine::intersects(a, inside));
    CHECK_FALSE(engine::intersects(a, apart));
}

TEST_CASE("zero-size Rect behaves as a point AABB") {
    const engine::Rectf point(5.0f, 5.0f, 0.0f, 0.0f);
    CHECK(engine::contains(point, 5.0f, 5.0f));
    CHECK_FALSE(engine::contains(point, 5.1f, 5.0f));
    CHECK(engine::intersects(point, engine::Rectf(0.0f, 0.0f, 5.0f, 5.0f)));
    CHECK_FALSE(engine::intersects(point, engine::Rectf(0.0f, 0.0f, 4.9f, 4.9f)));
}
