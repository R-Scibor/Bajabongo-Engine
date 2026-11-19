#pragma once

#include "engine/core/math/MathAliases.hpp"

#include <cmath>

namespace game {

    /// Muzzle height above the shooter's body origin when the default does not fit.
    /// Enemies have their origin at the sprite center (default -35); the player has it at the feet.
    struct MuzzleComponent {
        float heightOffset = -35.0f;
    };

    /// Muzzle height for shooters without a MuzzleComponent (sprite origin at the body center).
    constexpr float DEFAULT_MUZZLE_HEIGHT_OFFSET = -35.0f;
    /// How far ahead of the muzzle pivot along the aim direction a projectile spawns.
    constexpr float MUZZLE_FORWARD_OFFSET = 60.0f;

    inline float muzzleHeightOffset(const MuzzleComponent* muzzle) {
        return muzzle != nullptr ? muzzle->heightOffset : DEFAULT_MUZZLE_HEIGHT_OFFSET;
    }

    /// Point the shooter aims and fires from: the body origin raised to muzzle height.
    inline engine::Vector2f muzzlePivot(const engine::Vector2f& bodyPosition, float heightOffset) {
        return {bodyPosition.x, bodyPosition.y + heightOffset};
    }

    /// Aim angle from the muzzle pivot to a world point. Projectiles spawned with
    /// projectileSpawnPoint() on this angle travel through `target`.
    inline float aimAngleFrom(const engine::Vector2f& pivot, const engine::Vector2f& target) {
        return std::atan2(target.y - pivot.y, target.x - pivot.x);
    }

    inline engine::Vector2f projectileSpawnPoint(const engine::Vector2f& pivot, float angle,
                                                 float forwardOffset = MUZZLE_FORWARD_OFFSET) {
        return {pivot.x + std::cos(angle) * forwardOffset, pivot.y + std::sin(angle) * forwardOffset};
    }

} // namespace game
