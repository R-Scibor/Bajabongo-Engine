#pragma once

#include "engine/core/math/MathAliases.hpp"
#include "engine/physics/PhysicsConstants.hpp"

#include <box2d/box2d.h>
#include <cmath>

namespace game::ai {

    struct SolidFootprint {
        engine::Vector2f localCenter{0.0f, 0.0f};
        engine::Vector2f worldCenter{0.0f, 0.0f};
        float halfWidth = 16.0f;
        float halfHeight = 8.0f;
        bool found = false;
    };

    inline float halfExtentAlong(const SolidFootprint& foot, engine::Vector2f direction) {
        return std::abs(direction.x) * foot.halfWidth + std::abs(direction.y) * foot.halfHeight;
    }

    /// First non-sensor polygon on the body, or a default box at the transform.
    inline SolidFootprint solidFootprint(b2BodyId bodyId, const engine::Vector2f& transformFallback) {
        SolidFootprint foot;
        foot.worldCenter = transformFallback;
        if (!b2Body_IsValid(bodyId)) return foot;

        b2ShapeId shapes[8];
        const int count = b2Body_GetShapes(bodyId, shapes, 8);
        for (int i = 0; i < count; ++i) {
            if (b2Shape_IsSensor(shapes[i])) continue;
            if (b2Shape_GetType(shapes[i]) != b2_polygonShape) continue;

            const b2Polygon poly = b2Shape_GetPolygon(shapes[i]);
            if (poly.count <= 0) continue;

            float minX = poly.vertices[0].x;
            float maxX = minX;
            float minY = poly.vertices[0].y;
            float maxY = minY;
            for (int v = 1; v < poly.count; ++v) {
                minX = std::min(minX, poly.vertices[v].x);
                maxX = std::max(maxX, poly.vertices[v].x);
                minY = std::min(minY, poly.vertices[v].y);
                maxY = std::max(maxY, poly.vertices[v].y);
            }

            foot.localCenter = {poly.centroid.x, poly.centroid.y};
            foot.halfWidth = 0.5f * (maxX - minX);
            foot.halfHeight = 0.5f * (maxY - minY);
            const b2Pos world = b2Body_GetWorldPoint(bodyId, {foot.localCenter.x, foot.localCenter.y});
            foot.worldCenter = {world.x, world.y};
            foot.found = true;
            return foot;
        }

        const b2Pos pos = b2Body_GetPosition(bodyId);
        foot.worldCenter = {pos.x, pos.y};
        return foot;
    }

    inline bool areaBlockedByWalls(b2WorldId worldId, const engine::Vector2f& center, float halfWidth,
                                   float halfHeight) {
        bool hit = false;
        b2AABB aabb;
        aabb.lowerBound = {center.x - halfWidth, center.y - halfHeight};
        aabb.upperBound = {center.x + halfWidth, center.y + halfHeight};
        b2QueryFilter filter = b2DefaultQueryFilter();
        filter.maskBits = engine::PhysicsCategory::Wall | engine::PhysicsCategory::LowObstacle;
        auto callback = [](b2ShapeId, void* context) -> bool {
            *static_cast<bool*>(context) = true;
            return false;
        };
        b2World_OverlapAABB(worldId, {0.0f, 0.0f}, aabb, filter, callback, &hit);
        return hit;
    }

    /// Returns a unit direction toward the longest clear probe, or (0, 0) if all sides are tight.
    inline engine::Vector2f mostOpenDirection(b2WorldId worldId, const SolidFootprint& foot, float probeLength) {
        const engine::Vector2f dirs[8] = {{1.0f, 0.0f},        {-1.0f, 0.0f},       {0.0f, 1.0f},
                                          {0.0f, -1.0f},       {0.7071f, 0.7071f},  {-0.7071f, 0.7071f},
                                          {0.7071f, -0.7071f}, {-0.7071f, -0.7071f}};
        b2QueryFilter filter = b2DefaultQueryFilter();
        filter.maskBits = engine::PhysicsCategory::Wall | engine::PhysicsCategory::VisibilityBlocker |
                          engine::PhysicsCategory::LowObstacle;

        float best = -1.0f;
        engine::Vector2f bestDir{0.0f, 0.0f};
        for (const auto& direction : dirs) {
            const float lead = halfExtentAlong(foot, direction) + 1.0f;
            const b2Vec2 origin = {foot.worldCenter.x + direction.x * lead, foot.worldCenter.y + direction.y * lead};
            const b2Vec2 translation = {direction.x * probeLength, direction.y * probeLength};
            const b2RayResult result = b2World_CastRayClosest(worldId, origin, translation, filter);
            const float clear = result.hit ? result.fraction * probeLength : probeLength;
            if (clear > best) {
                best = clear;
                bestDir = direction;
            }
        }
        if (best < 8.0f) return {0.0f, 0.0f};
        return bestDir;
    }

} // namespace game::ai
