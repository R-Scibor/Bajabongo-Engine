#pragma once

namespace engine {
    /// @brief π as float, shared by angle helpers.
    inline constexpr float Pi = 3.14159265358979323846f;

    /**
     * @brief Wraps an angle in radians to the range [-π, π].
     *
     * Uses the same loop form as the previous call sites so values already
     * in range are unchanged.
     */
    inline float wrapAngle(float angle) {
        constexpr float twoPi = 2.0f * Pi;
        while (angle > Pi) {
            angle -= twoPi;
        }
        while (angle < -Pi) {
            angle += twoPi;
        }
        return angle;
    }
} // namespace engine
