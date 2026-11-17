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

TEST_CASE("wrapAngle leaves values just inside the bounds alone") {
    const float inside = engine::Pi - 1e-4f;
    CHECK(engine::wrapAngle(inside) == inside);
    CHECK(engine::wrapAngle(-inside) == -inside);
}

TEST_CASE("wrapAngle handles several turns and shortest-path deltas") {
    CHECK(engine::wrapAngle(5.0f * engine::Pi) == Catch::Approx(engine::Pi).margin(1e-4f));
    CHECK(engine::wrapAngle(-5.0f * engine::Pi) == Catch::Approx(-engine::Pi).margin(1e-4f));
    CHECK(engine::wrapAngle(7.0f * engine::Pi) == Catch::Approx(engine::Pi).margin(1e-4f));

    // Typical shortest-path delta: two headings near opposite sides of +/- pi.
    const float from = engine::Pi - 0.1f;
    const float to = -engine::Pi + 0.1f;
    const float delta = engine::wrapAngle(to - from);
    CHECK(delta == Catch::Approx(0.2f).margin(1e-5f));
    CHECK(delta >= -engine::Pi);
    CHECK(delta <= engine::Pi);
}

TEST_CASE("wrapAngle result always sits in [-pi, pi]") {
    const float samples[] = {0.0f, -0.0f, 1e-6f, -1e-6f, engine::Pi,         -engine::Pi,
                             4.0f, -4.0f, 10.0f, -10.0f, 20.0f * engine::Pi, -13.0f * engine::Pi};
    for (float sample : samples) {
        const float wrapped = engine::wrapAngle(sample);
        CHECK(wrapped <= engine::Pi);
        CHECK(wrapped >= -engine::Pi);
    }
}
