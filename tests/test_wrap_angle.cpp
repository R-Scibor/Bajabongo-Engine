#include "engine/core/math/Angle.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

TEST_CASE("wrapAngle keeps values already in [-pi, pi]") {
    CHECK(engine::wrapAngle(0.0f) == 0.0f);
    CHECK(engine::wrapAngle(engine::Pi) == engine::Pi);
    CHECK(engine::wrapAngle(-engine::Pi) == -engine::Pi);
    CHECK(engine::wrapAngle(1.25f) == 1.25f);
}

TEST_CASE("wrapAngle reduces multiples of two-pi into range") {
    CHECK(engine::wrapAngle(2.0f * engine::Pi) == Catch::Approx(0.0f).margin(1e-5f));
    CHECK(engine::wrapAngle(-2.0f * engine::Pi) == Catch::Approx(0.0f).margin(1e-5f));
    CHECK(engine::wrapAngle(3.0f * engine::Pi) == Catch::Approx(engine::Pi).margin(1e-5f));
    CHECK(engine::wrapAngle(-3.0f * engine::Pi) == Catch::Approx(-engine::Pi).margin(1e-5f));
}

TEST_CASE("wrapAngle maps just outside the bounds inward") {
    const float justOver = engine::Pi + 0.25f;
    const float wrapped = engine::wrapAngle(justOver);
    CHECK(wrapped == justOver - 2.0f * engine::Pi);
    CHECK(wrapped < 0.0f);

    const float justUnder = -engine::Pi - 0.25f;
    const float wrappedUnder = engine::wrapAngle(justUnder);
    CHECK(wrappedUnder == justUnder + 2.0f * engine::Pi);
    CHECK(wrappedUnder > 0.0f);
}
