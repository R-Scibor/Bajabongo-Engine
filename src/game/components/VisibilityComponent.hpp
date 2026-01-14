#pragma once
#include "engine/core/math/Vector2.hpp"
#include "engine/core/math/MathAliases.hpp"
#include <vector>

namespace game {
    struct VisibilityComponent {
        std::vector<engine::Vector2f> visibilityPolygon;
        float viewRadius = 500.0f; // Default view radius
    };
}
