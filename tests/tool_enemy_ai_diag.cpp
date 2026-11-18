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
    const int frames = seconds * 60;

    std::cout << "headless enemy AI  seconds=" << seconds << " frames=" << frames << " seed=" << seed
              << " assets=" << assets << '\n';

    for (const char* mapName : {"hideout", "hospital"}) {
        auto stats = baja::test::runHeadlessEnemyAi(assets, mapName, seed, frames);
        std::cout << "======== " << mapName << " ========\n";
        if (!stats.error.empty()) {
            std::cout << "ERROR " << stats.error << '\n';
            continue;
        }
        std::cout << "SUMMARY " << mapName << " enemies=" << stats.enemyCount << " stuckChecks=" << stats.stuckChecks
                  << " movedZero=" << stats.movedZero << " teleports=" << stats.teleports
                  << " impulses=" << stats.impulses << " visibleImpulses=" << stats.visibleImpulses
                  << " recovered=" << stats.recovered << " killed=" << stats.killed
                  << " barelyMoved(<10)=" << stats.barelyMoved << " maxStuck=" << stats.maxStuck
                  << " minDisp=" << stats.minDisp << " maxDisp=" << stats.maxDisp << '\n';
    }
    return 0;
}
