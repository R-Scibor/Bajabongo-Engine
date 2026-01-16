#pragma once

namespace game {

struct SniperBehaviorComponent {
    float aimDuration = 1.5f;
    float aimTimer = 0.0f;
    float maxRange = 700.0f;
    bool isAiming = false;
};

} // namespace game
