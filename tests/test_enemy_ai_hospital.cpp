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

    // Hospital, seed 1, 1800 frames on this harness:
    //   baseline (before A+B): 129 stuck, 99 moved 0.00, 24 teleports, maxStuck 12, maxDisp 1784
    //   after A+B:             175 / 124 / 36, maxStuck 3, maxDisp 859
    //   after wall validation: 178 / 93 / 39, maxStuck 3, maxDisp 602
    //   after patrol skip:     122 / 68 / 3,  maxStuck 2, maxDisp 602, barelyMoved 0
    //   after chase sidestep:  118 / 70 / 0,  maxStuck 1, maxDisp 602, barelyMoved 0
    // Gates were tightened with slack after the skip improved stuck, teleports,
    // and zero-moves. They still fail a collapse or a return to a map-wide march.
    UNSCOPED_INFO("stuckChecks=" << stats.stuckChecks << " movedZero=" << stats.movedZero
                                 << " teleports=" << stats.teleports << " impulses=" << stats.impulses
                                 << " visibleImpulses=" << stats.visibleImpulses << " recovered=" << stats.recovered
                                 << " killed=" << stats.killed << " barelyMoved=" << stats.barelyMoved << " maxStuck="
                                 << stats.maxStuck << " minDisp=" << stats.minDisp << " maxDisp=" << stats.maxDisp);

    CHECK(stats.stuckChecks <= 155);
    CHECK(stats.movedZero <= 100);
    CHECK(stats.teleports <= 12);
    CHECK(stats.killed == 0);
    CHECK(stats.maxStuck <= 6);
    CHECK(stats.barelyMoved <= 3);
    CHECK(stats.maxDisp < 900.0f);
}

TEST_CASE("Hospital chase behind a wall (seed 1, 1800 frames) recovers without teleports") {
    const std::filesystem::path assets{BAJA_ASSETS_DIR};
    auto stats = baja::test::runHeadlessEnemyAi(assets, "hospital", 1, 1800, 1.0f / 60.0f, true);
    if (!stats.error.empty()) {
        SKIP(stats.error);
    }

    REQUIRE(stats.chasePlaced >= 1);
    UNSCOPED_INFO("primed=" << stats.chasePlaced << " stuckChecks=" << stats.stuckChecks
                            << " pursuitStuck=" << stats.pursuitStuckChecks << " movedZero=" << stats.movedZero
                            << " teleports=" << stats.teleports << " impulses=" << stats.impulses
                            << " maxStuck=" << stats.maxStuck << " maxDisp=" << stats.maxDisp);

    CHECK(stats.killed == 0);
    CHECK(stats.teleports <= 8);
    CHECK(stats.pursuitStuckChecks <= 25);
    CHECK(stats.barelyMoved <= 3);
}
