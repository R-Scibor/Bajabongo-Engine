#include "engine/core/math/Random.hpp"

#include <catch2/catch_test_macros.hpp>
#include <vector>

TEST_CASE("Random with the same seed repeats the integer and real streams") {
    engine::Random a(12345u);
    engine::Random b(12345u);

    std::vector<int> intsA;
    std::vector<float> realsA;
    for (int i = 0; i < 64; ++i) {
        intsA.push_back(a.below(100));
        realsA.push_back(a.real(0.0f, 1.0f));
    }

    for (int i = 0; i < 64; ++i) {
        CHECK(b.below(100) == intsA[static_cast<std::size_t>(i)]);
        CHECK(b.real(0.0f, 1.0f) == realsA[static_cast<std::size_t>(i)]);
    }
}

TEST_CASE("Random reseeding restarts the stream") {
    engine::Random rng(7u);
    const int first = rng.below(1000);
    const int second = rng.below(1000);
    REQUIRE(first != second);

    rng.seed(7u);
    CHECK(rng.currentSeed() == 7u);
    CHECK(rng.below(1000) == first);
    CHECK(rng.below(1000) == second);
}

TEST_CASE("Random integer ranges match inclusive and exclusive contracts") {
    engine::Random rng(99u);
    for (int i = 0; i < 200; ++i) {
        const int b = rng.below(5);
        CHECK(b >= 0);
        CHECK(b <= 4);

        const int incl = rng.integer(-2, 2);
        CHECK(incl >= -2);
        CHECK(incl <= 2);
    }
    CHECK(rng.below(0) == 0);
    CHECK(rng.below(1) == 0);
    CHECK(rng.integer(3, 1) == 3);
}

TEST_CASE("Random real stays in a half-open range and is zero-width safe") {
    engine::Random rng(42u);
    for (int i = 0; i < 200; ++i) {
        const float x = rng.real(-1.5f, 1.5f);
        CHECK(x >= -1.5f);
        CHECK(x < 1.5f);
    }
    CHECK(rng.real(0.0f, 0.0f) == 0.0f);
    CHECK(rng.real(2.0f, -3.0f) == 2.0f);
}

TEST_CASE("Random default constructor records a seed that can be replayed") {
    engine::Random first;
    const auto seed = first.currentSeed();
    const int a = first.below(10000);
    const int b = first.below(10000);

    engine::Random replay(seed);
    CHECK(replay.currentSeed() == seed);
    CHECK(replay.below(10000) == a);
    CHECK(replay.below(10000) == b);
}
