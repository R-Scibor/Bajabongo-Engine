#pragma once

#include "engine/core/math/MathAliases.hpp" 

namespace game {

struct CampBehaviorComponent {
    float campDuration = 15.0f;
    float campTimer = 0.0f;
    float campZoneRadius = 200.0f;
    engine::Vector2f campPosition{0.0f, 0.0f};
};

} // namespace game
