#pragma once
#include "engine/core/math/MathAliases.hpp"
#include <vector>

namespace game {

struct PatrolBehaviorComponent {
    std::vector<engine::Vector2f> waypoints;
    int currentWaypointIndex = 0;
    float waypointReachThreshold = 20.0f;
    bool loopMode = true; // true = loop, false = ping-pong
    bool reversing = false; // For ping-pong
};

} // namespace game
