#pragma once

#include "engine/core/math/Angle.hpp"
#include "engine/core/math/MathAliases.hpp"

#include <cmath>

namespace game {

    inline float viewHalfAngleRadians(float viewAngleDegrees) {
        return viewAngleDegrees * 0.5f * (engine::Pi / 180.0f);
    }

    inline engine::Vector2f visibilityOrigin(const engine::Vector2f& position, const engine::Vector2f& offset) {
        return position + offset;
    }

    inline bool isAngleInViewCone(float angle, float direction, float halfAngle, float epsilon = 0.0001f) {
        const float diff = engine::wrapAngle(angle - direction);
        return diff >= -halfAngle - epsilon && diff <= halfAngle + epsilon;
    }

    inline bool isWithinMinViewRadius(const engine::Vector2f& point, const engine::Vector2f& origin, float minRadius) {
        const float dx = point.x - origin.x;
        const float dy = point.y - origin.y;
        return dx * dx + dy * dy <= minRadius * minRadius;
    }

    inline bool shouldSampleWallVertex(const engine::Vector2f& vertex, const engine::Vector2f& origin, float direction,
                                       float halfAngle, float minRadius, float epsilon = 0.0001f) {
        const engine::Vector2f dir = vertex - origin;
        const float angle = std::atan2(dir.y, dir.x);
        return isAngleInViewCone(angle, direction, halfAngle, epsilon) ||
               isWithinMinViewRadius(vertex, origin, minRadius);
    }

    inline float visibilityRayLength(float angle, float direction, float halfAngle, float radius, float minRadius,
                                     float epsilon = 0.0001f) {
        return isAngleInViewCone(angle, direction, halfAngle, epsilon) ? radius : minRadius;
    }

} // namespace game
