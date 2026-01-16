#pragma once

namespace game {

struct RushBehaviorComponent {
    float rushSpeed = 250.0f;
    float rushActivationRange = 150.0f;
    float rushCooldown = 5.0f;
    float rushCooldownTimer = 0.0f;
};

} // namespace game
