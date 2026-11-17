#include "engine/pch.h"
#include "VisibilitySystem.hpp"
#include "VisibilityMath.hpp"
#include "engine/core/math/Angle.hpp"
#include "engine/components/TransformComponent.hpp"
#include "engine/physics/PhysicsConstants.hpp"
#include "engine/core/ILoggerManager.hpp"
#include <algorithm>
#include <cmath>
#include <set>

namespace game {

    VisibilitySystem::VisibilitySystem(engine::EngineContext& context)
        : m_context(context)
        , m_worldId(context.m_physicsWorld) {
        m_logger = context.m_logManager->getLogger("VisibilitySystem");
    }

    // Callback for AABB Query
    struct QueryCallbackData {
        std::vector<engine::Vector2f>* vertices;
        b2WorldId worldId;
        engine::Vector2f center;
        float radiusSq;
    };

    static bool QueryCallback(b2ShapeId shapeId, void* context) {
        QueryCallbackData* data = static_cast<QueryCallbackData*>(context);

        b2Filter filter = b2Shape_GetFilter(shapeId);
        if (!(filter.categoryBits & engine::PhysicsCategory::VisibilityBlocker)) {
            return true; // Continue
        }

        b2BodyId bodyId = b2Shape_GetBody(shapeId);
        b2Transform transform = b2Body_GetTransform(bodyId);

        // We need to get vertices from the shape.
        // Box2D 3.0 shapes: Circle, Capsule, Segment, Polygon, Chain
        // For walls we mostly expect Chain or Polygon.

        b2ShapeType type = b2Shape_GetType(shapeId);

        if (type == b2_polygonShape) {
            b2Polygon poly = b2Shape_GetPolygon(shapeId);
            for (int i = 0; i < poly.count; ++i) {
                b2Vec2 worldPoint = b2TransformPoint(transform, poly.vertices[i]);
                data->vertices->emplace_back(worldPoint.x, worldPoint.y);
            }
        } else if (type == b2_chainSegmentShape) {
            b2ChainSegment segment = b2Shape_GetChainSegment(shapeId);
            // Chain segment has point1 and point2
            b2Vec2 p1 = b2TransformPoint(transform, segment.segment.point1);
            b2Vec2 p2 = b2TransformPoint(transform, segment.segment.point2);
            data->vertices->emplace_back(p1.x, p1.y);
            data->vertices->emplace_back(p2.x, p2.y);
        }
        // Add other shapes if necessary, but map uses chains mostly for walls.

        return true;
    }

    void VisibilitySystem::update() {
        auto view = m_context.m_registry->view<engine::TransformComponent, VisibilityComponent>();

        if (view.begin() == view.end()) {
            // m_logger->debug("No entities with VisibilityComponent found.");
        }

        for (auto entity : view) {
            auto& transform = view.get<engine::TransformComponent>(entity);
            auto& visibility = view.get<VisibilityComponent>(entity);

            visibility.visibilityPolygon.clear();

            engine::Vector2f origin = visibilityOrigin(transform.position, visibility.offset);
            float radius = visibility.viewRadius;
            float minRadius = visibility.minViewRadius;
            float direction = visibility.viewDirection; // Mouse aim direction
            float halfAngle = viewHalfAngleRadians(visibility.viewAngle);

            // 1. Query walls in view cone range (slightly wider for safety)
            std::vector<engine::Vector2f> wallVertices;
            queryWorld(origin, radius, wallVertices);

            // m_logger->debug("Visibility System: Entity {} found {} wall vertices in range.", entt::to_integral(entity), wallVertices.size());

            // 2. Create angles to cast rays
            std::vector<float> angles;

            // Add cone boundary rays
            float leftAngle = direction - halfAngle;
            float rightAngle = direction + halfAngle;
            angles.push_back(leftAngle);
            angles.push_back(rightAngle);

            const float epsilon = 0.0001f;
            for (const auto& v : wallVertices) {
                engine::Vector2f dir = v - origin;
                float angle = std::atan2(dir.y, dir.x);

                // Only consider walls within cone field of view OR within min radius
                if (shouldSampleWallVertex(v, origin, direction, halfAngle, minRadius, epsilon)) {
                    angles.push_back(angle);
                    angles.push_back(angle - epsilon);
                    angles.push_back(angle + epsilon);
                }
            }

            // Add cone arc boundary points (curved edge)
            const int arcSegments = 32; // Smooth arc
            for (int i = 0; i <= arcSegments; ++i) {
                float t = static_cast<float>(i) / arcSegments;
                float arcAngle = leftAngle + t * (rightAngle - leftAngle);
                angles.push_back(arcAngle);
            }

            // Add full circle boundary points for minRadius
            const int circleSegments = 90;
            for (int i = 0; i < circleSegments; ++i) {
                float angle = -engine::Pi + (float)i / circleSegments * 2.0f * engine::Pi;
                angles.push_back(angle);
            }

            // Sort angles
            std::sort(angles.begin(), angles.end());

            // Remove duplicates
            angles.erase(
                std::unique(angles.begin(), angles.end(), [](float a, float b) { return std::abs(a - b) < 0.0001f; }),
                angles.end());

            // 3. Cast rays and build polygon
            std::vector<engine::Vector2f> points;
            points.push_back(origin); // Fan origin

            for (float angle : angles) {
                engine::Vector2f dir(std::cos(angle), std::sin(angle));

                // Determine ray length for this angle
                float currentRadius = visibilityRayLength(angle, direction, halfAngle, radius, minRadius, epsilon);

                engine::Vector2f end = origin + dir * currentRadius;
                engine::Vector2f hit = castRay(origin, end);
                points.push_back(hit);
            }

            // Close the polygon
            if (points.size() > 1) {
                points.push_back(points[1]); // Connect back to first ray
            }

            visibility.visibilityPolygon = points;
        }
    }

    void VisibilitySystem::queryWorld(const engine::Vector2f& center, float radius,
                                      std::vector<engine::Vector2f>& wallVertices) {
        b2AABB aabb;
        aabb.lowerBound = {center.x - radius, center.y - radius};
        aabb.upperBound = {center.x + radius, center.y + radius};

        QueryCallbackData data;
        data.vertices = &wallVertices;
        data.worldId = m_worldId;
        data.center = center;
        data.radiusSq = radius * radius;

        b2World_OverlapAABB(m_worldId, b2Pos_zero, aabb, b2DefaultQueryFilter(), QueryCallback, &data);
    }

    engine::Vector2f VisibilitySystem::castRay(const engine::Vector2f& start, const engine::Vector2f& end) {
        b2QueryFilter filter = b2DefaultQueryFilter();
        filter.maskBits = engine::PhysicsCategory::VisibilityBlocker;

        // Use b2World_CastRayClosest instead - it returns b2RayResult directly
        b2RayResult result =
            b2World_CastRayClosest(m_worldId, {start.x, start.y}, {end.x - start.x, end.y - start.y}, filter);

        if (result.hit) {
            return {result.point.x, result.point.y};
        }

        return end;
    }

} // namespace game
