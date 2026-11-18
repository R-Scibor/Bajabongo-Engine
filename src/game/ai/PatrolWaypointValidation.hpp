#pragma once

#include "EnemyNavigation.hpp"
#include "engine/components/PhysicsBodyComponent.hpp"
#include "engine/components/TransformComponent.hpp"
#include "engine/core/EngineContext.hpp"
#include "engine/core/math/Random.hpp"
#include "engine/physics/PhysicsConstants.hpp"
#include "game/components/PatrolBehaviorComponent.hpp"

#include <box2d/box2d.h>
#include <cmath>
#include <vector>

namespace game {

    struct PatrolValidationLimits {
        float minAdvance = 16.0f;
        float hitMargin = 8.0f;
        float fallbackRadius = 32.0f;
        int randomAttempts = 8;
        float halfWidth = 20.0f;
        float halfHeight = 8.0f;
    };

    inline b2ShapeProxy feetProxy(float halfWidth, float halfHeight) {
        b2Vec2 points[4] = {
            {-halfWidth, -halfHeight}, {halfWidth, -halfHeight}, {halfWidth, halfHeight}, {-halfWidth, halfHeight}};
        return b2MakeProxy(points, 4, 0.0f);
    }

    /// Shape-cast the feet box from `from` toward `to`. On a wall hit, shortens to a
    /// point before impact. Returns false if the clear run is shorter than minAdvance
    /// or the end overlaps a wall.
    inline bool tryReachPatrolPoint(b2WorldId worldId, const engine::Vector2f& from, const engine::Vector2f& to,
                                    const PatrolValidationLimits& limits, engine::Vector2f& out) {
        const engine::Vector2f delta{to.x - from.x, to.y - from.y};
        const float dist = std::sqrt(delta.x * delta.x + delta.y * delta.y);
        if (dist < 0.01f) {
            if (ai::areaBlockedByWalls(worldId, from, limits.halfWidth, limits.halfHeight)) return false;
            out = from;
            return true;
        }

        const engine::Vector2f dir{delta.x / dist, delta.y / dist};
        const b2ShapeProxy proxy = feetProxy(limits.halfWidth, limits.halfHeight);
        b2QueryFilter filter = b2DefaultQueryFilter();
        filter.maskBits = engine::PhysicsCategory::Wall | engine::PhysicsCategory::LowObstacle;

        struct Hit {
            bool hit = false;
            float fraction = 1.0f;
        } hit;

        auto callback = [](b2ShapeId shape, b2Pos, b2Vec2, float fraction, void* context) -> float {
            if (b2Shape_IsSensor(shape)) return -1.0f;
            auto* result = static_cast<Hit*>(context);
            result->hit = true;
            result->fraction = fraction;
            return fraction;
        };

        b2World_CastShape(worldId, {from.x, from.y}, &proxy, {delta.x, delta.y}, filter, callback, &hit);

        engine::Vector2f proposed = to;
        if (hit.hit) {
            const float travel = hit.fraction * dist - limits.hitMargin;
            if (travel < limits.minAdvance) return false;
            proposed = {from.x + dir.x * travel, from.y + dir.y * travel};
        }

        if (ai::areaBlockedByWalls(worldId, proposed, limits.halfWidth, limits.halfHeight)) return false;
        out = proposed;
        return true;
    }

    inline engine::Vector2f validateOnePatrolWaypoint(b2WorldId worldId, const engine::Vector2f& from,
                                                      const engine::Vector2f& candidate, engine::Random& rng,
                                                      const PatrolValidationLimits& limits) {
        engine::Vector2f accepted;
        if (tryReachPatrolPoint(worldId, from, candidate, limits, accepted)) return accepted;

        const engine::Vector2f delta{candidate.x - from.x, candidate.y - from.y};
        float dist = std::sqrt(delta.x * delta.x + delta.y * delta.y);
        if (dist < limits.minAdvance) dist = limits.fallbackRadius * 4.0f;

        for (int attempt = 0; attempt < limits.randomAttempts; ++attempt) {
            const float angle = rng.real(0.0f, 6.2831853f);
            const float length = (attempt < limits.randomAttempts / 2) ? dist : dist * 0.5f;
            const engine::Vector2f probe{from.x + std::cos(angle) * length, from.y + std::sin(angle) * length};
            if (tryReachPatrolPoint(worldId, from, probe, limits, accepted)) return accepted;
        }

        const engine::Vector2f dirs[8] = {{1.0f, 0.0f},        {-1.0f, 0.0f},       {0.0f, 1.0f},
                                          {0.0f, -1.0f},       {0.7071f, 0.7071f},  {-0.7071f, 0.7071f},
                                          {0.7071f, -0.7071f}, {-0.7071f, -0.7071f}};
        for (const auto& dir : dirs) {
            const engine::Vector2f probe{from.x + dir.x * limits.fallbackRadius,
                                         from.y + dir.y * limits.fallbackRadius};
            if (tryReachPatrolPoint(worldId, from, probe, limits, accepted)) return accepted;
        }

        return from;
    }

    /// Push a point if it is meaningfully different from the last accepted one.
    inline void appendIfSeparated(std::vector<engine::Vector2f>& out, const engine::Vector2f& point, float minSep) {
        if (!out.empty()) {
            const float dx = point.x - out.back().x;
            const float dy = point.y - out.back().y;
            if (dx * dx + dy * dy < minSep * minSep) return;
        }
        out.push_back(point);
    }

    inline std::vector<engine::Vector2f> validatePatrolWaypoints(b2WorldId worldId, const engine::Vector2f& spawn,
                                                                 const std::vector<engine::Vector2f>& candidates,
                                                                 engine::Random& rng,
                                                                 const PatrolValidationLimits& limits) {
        std::vector<engine::Vector2f> world;
        engine::Vector2f from = spawn;
        const float minSep = limits.minAdvance * 0.5f;
        for (const auto& candidate : candidates) {
            const engine::Vector2f accepted = validateOnePatrolWaypoint(worldId, from, candidate, rng, limits);
            appendIfSeparated(world, accepted, minSep);
            from = accepted;
        }
        if (world.empty()) {
            world.push_back(spawn);
        }
        return world;
    }

    /// Run after PhysicsBodyCreationSystem so Collision / HalfCollision exist in the world.
    inline void validateLoadedPatrolWaypoints(engine::EngineContext& context) {
        if (!context.m_registry || !b2World_IsValid(context.m_physicsWorld)) return;

        auto view = context.m_registry->view<PatrolBehaviorComponent, engine::TransformComponent>();
        for (auto entity : view) {
            auto& patrol = view.get<PatrolBehaviorComponent>(entity);
            const auto& transform = view.get<engine::TransformComponent>(entity);

            PatrolValidationLimits limits;
            if (const auto* body = context.m_registry->try_get<engine::PhysicsBodyComponent>(entity)) {
                const ai::SolidFootprint foot = ai::solidFootprint(body->bodyId, transform.position);
                limits.halfWidth = foot.halfWidth;
                limits.halfHeight = foot.halfHeight;
            }

            patrol.waypoints = validatePatrolWaypoints(context.m_physicsWorld, transform.position, patrol.waypoints,
                                                       context.m_random, limits);
            patrol.currentWaypointIndex = 0;
            patrol.reversing = false;
        }
    }

} // namespace game
