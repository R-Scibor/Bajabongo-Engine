#include "engine/pch.h"
#include "AIUtils.hpp"
#include "engine/physics/PhysicsConstants.hpp"
#include "game/components/VisibilityComponent.hpp"
#include "engine/components/TransformComponent.hpp"
#include <cmath>

namespace game::ai {

    bool AIUtils::hasLineOfSight(engine::EngineContext& context, const engine::Vector2f& start, const engine::Vector2f& end) {
        b2QueryFilter filter = b2DefaultQueryFilter();
        filter.categoryBits = 0xFFFFFFFF;
        filter.maskBits = engine::PhysicsCategory::VisibilityBlocker | engine::PhysicsCategory::Wall;
        
        engine::Vector2f direction = end - start;
        float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
        
        if (length < 0.01f) return true;
        
        direction = direction / length;
        
        b2Vec2 origin = {start.x, start.y};
        b2Vec2 translation = {direction.x * length, direction.y * length};
        
        b2RayResult result = b2World_CastRayClosest(context.m_physicsWorld, origin, translation, filter);
        
        return !result.hit;
    }

    bool AIUtils::isPositionVisibleToPlayer(engine::EngineContext& context, const engine::Vector2f& position) {
        auto view = context.m_registry->view<VisibilityComponent, engine::TransformComponent>();
        bool isVisible = false;
        
        view.each([&](entt::entity entity, const VisibilityComponent& visibility, const engine::TransformComponent& transform) {
            if (isVisible) return;
            
            engine::Vector2f viewerPos = transform.position + visibility.offset;
            engine::Vector2f toTarget = position - viewerPos;
            float distSq = toTarget.x * toTarget.x + toTarget.y * toTarget.y;
            
            if (distSq > visibility.viewRadius * visibility.viewRadius) return;
            
            if (distSq < visibility.minViewRadius * visibility.minViewRadius) {
                isVisible = true;
                return;
            }
            
            float angleToTarget = std::atan2(toTarget.y, toTarget.x);
            float halfFOV = (visibility.viewAngle * 3.14159f / 180.0f) / 2.0f;
            
            float angleDiff = angleToTarget - visibility.viewDirection;
            while (angleDiff > 3.14159f) angleDiff -= 2.0f * 3.14159f;
            while (angleDiff < -3.14159f) angleDiff += 2.0f * 3.14159f;
            
            if (std::abs(angleDiff) <= halfFOV) {
                if (hasLineOfSight(context, viewerPos, position)) {
                    isVisible = true;
                }
            }
        });
        
        return isVisible;
    }

}
