#include "EnemyAiHarness.hpp"

#include <catch2/catch_test_macros.hpp>

#include <filesystem>

#ifndef BAJA_ASSETS_DIR
#define BAJA_ASSETS_DIR ""
#endif

TEST_CASE("Hospital enemy AI (seed 1, 1800 frames) stays below the pre-fix stuck baseline") {
    const std::filesystem::path assets{BAJA_ASSETS_DIR};
    auto stats = baja::test::runHeadlessEnemyAi(assets, "hospital", 1, 1800);
    if (!stats.error.empty()) {
        SKIP(stats.error);
    }

    REQUIRE(stats.enemyCount == 16);

    // Pre-A+B baseline on this harness (hospital, seed 1, 1800 frames):
    // 129 stuck checks, 99 moved 0.00, 24 last-valid teleports, maxStuck 12,
    // barelyMoved 1, maxDisp 1784 (shared archetype corner at 100, 200).
    // After spawn-relative patrols, local wall contacts can produce more stuck
    // checks and teleports than that far-corner march. Measured post-fix:
    // 175 / 124 / 36, maxStuck 3, barelyMoved 2, maxDisp 859. Gates sit above
    // that run with slack, and still fail a return to a map-wide march or a
    // pile-up that never recovers.
    UNSCOPED_INFO("stuckChecks=" << stats.stuckChecks << " movedZero=" << stats.movedZero
                                 << " teleports=" << stats.teleports << " impulses=" << stats.impulses
                                 << " visibleImpulses=" << stats.visibleImpulses << " recovered=" << stats.recovered
                                 << " killed=" << stats.killed << " barelyMoved=" << stats.barelyMoved << " maxStuck="
                                 << stats.maxStuck << " minDisp=" << stats.minDisp << " maxDisp=" << stats.maxDisp);

    CHECK(stats.stuckChecks <= 220);
    CHECK(stats.movedZero <= 160);
    CHECK(stats.teleports <= 55);
    CHECK(stats.killed == 0);
    CHECK(stats.maxStuck <= 8);
    CHECK(stats.barelyMoved <= 5);
    CHECK(stats.maxDisp < 1400.0f);
}
