#pragma once

#include "engine/core/math/MathAliases.hpp"
#include "game/components/HealthComponent.hpp"

namespace game::ai {

    /// Strict less-than, matching the original distanceSq < radiusSq checks.
    inline bool isWithinRadius(const engine::Vector2f& from, const engine::Vector2f& to, float radius) {
        const float dx = to.x - from.x;
        const float dy = to.y - from.y;
        return dx * dx + dy * dy < radius * radius;
    }

    inline bool hasForgottenTarget(float lastSeenTimer, float memoryDuration) {
        return lastSeenTimer > memoryDuration;
    }

    inline bool isEnemyDefeated(const game::HealthComponent* health) {
        return health != nullptr && health->currentHp <= 0.0f;
    }

} // namespace game::ai
