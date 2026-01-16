#pragma once

namespace game {

struct TurretBehaviorComponent {
    float rotationSpeed = 3.0f; // radians/second
    float shootAngleTolerance = 0.1f; // radians
};

} // namespace game
