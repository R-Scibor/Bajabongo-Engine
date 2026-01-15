#pragma once
#include "engine/core/math/Vector2.hpp"
#include "engine/core/math/MathAliases.hpp"
#include <vector>

namespace game {
    struct VisibilityComponent {
        std::vector<engine::Vector2f> visibilityPolygon;
        
        float viewRadius = 500.0f;      // Distance
        float minViewRadius = 50.0f;    // Always visible circle around player
        float viewAngle = 90.0f;        // Field of view in DEGREES (90° cone)
        float viewDirection = 0.0f;     // Direction in RADIANS (0 = right, PI/2 = up, etc.)
        float targetViewDirection = 0.0f;  // For smoothing
        float viewInterpSpeed = 8.0f;      // Radians per second
        
        engine::Vector2f offset = {0.0f, 0.0f}; // Offset from transform position
    };
}
