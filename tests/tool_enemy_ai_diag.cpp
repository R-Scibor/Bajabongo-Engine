#include "EnemyAiHarness.hpp"

#include <cstdint>
#include <iostream>
#include <string>

#ifndef BAJA_ASSETS_DIR
#define BAJA_ASSETS_DIR ""
#endif

int main(int argc, char** argv) {
    const std::filesystem::path assets = (argc > 1) ? argv[1] : std::filesystem::path{BAJA_ASSETS_DIR};
    const std::uint32_t seed = (argc > 2) ? static_cast<std::uint32_t>(std::stoul(argv[2])) : 1u;
    const int seconds = (argc > 3) ? std::stoi(argv[3]) : 30;
    const bool chaseBehindWall = (argc > 4) && std::string(argv[4]) == "chase";
    const int frames = seconds * 60;

    std::cout << "headless enemy AI  seconds=" << seconds << " frames=" << frames << " seed=" << seed
              << " chaseBehindWall=" << (chaseBehindWall ? "yes" : "no") << " assets=" << assets << '\n';

    if (chaseBehindWall) {
        auto stats = baja::test::runHeadlessEnemyAi(assets, "hospital", seed, frames, 1.0f / 60.0f, true);
        std::cout << "======== hospital chase-behind-wall ========\n";
        if (!stats.error.empty()) {
            std::cout << "ERROR " << stats.error << '\n';
            return 1;
        }
        std::cout << "SUMMARY hospital-chase enemies=" << stats.enemyCount << " primed=" << stats.chasePlaced
                  << " stuckChecks=" << stats.stuckChecks << " pursuitStuck=" << stats.pursuitStuckChecks
                  << " movedZero=" << stats.movedZero << " teleports=" << stats.teleports
                  << " impulses=" << stats.impulses << " recovered=" << stats.recovered << " killed=" << stats.killed
                  << " barelyMoved(<10)=" << stats.barelyMoved << " pursuitEnd=" << stats.pursuitEnemies
                  << " maxStuck=" << stats.maxStuck << " minDisp=" << stats.minDisp << " maxDisp=" << stats.maxDisp
                  << '\n';
        return 0;
    }

    for (const char* mapName : {"hideout", "hospital"}) {
        auto stats = baja::test::runHeadlessEnemyAi(assets, mapName, seed, frames);
        std::cout << "======== " << mapName << " ========\n";
        if (!stats.error.empty()) {
            std::cout << "ERROR " << stats.error << '\n';
            continue;
        }
        std::cout << "SUMMARY " << mapName << " enemies=" << stats.enemyCount << " stuckChecks=" << stats.stuckChecks
                  << " pursuitStuck=" << stats.pursuitStuckChecks << " movedZero=" << stats.movedZero
                  << " teleports=" << stats.teleports << " impulses=" << stats.impulses
                  << " visibleImpulses=" << stats.visibleImpulses << " recovered=" << stats.recovered
                  << " killed=" << stats.killed << " barelyMoved(<10)=" << stats.barelyMoved
                  << " pursuitEnd=" << stats.pursuitEnemies << " maxStuck=" << stats.maxStuck
                  << " minDisp=" << stats.minDisp << " maxDisp=" << stats.maxDisp << '\n';
    }
    return 0;
}
