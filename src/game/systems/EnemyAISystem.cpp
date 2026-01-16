#include "engine/pch.h"
#include "EnemyAISystem.hpp"
#include "game/components/EnemyComponent.hpp"
#include "engine/components/TransformComponent.hpp"
#include "engine/components/PhysicsBodyComponent.hpp"
#include "engine/core/ILoggerManager.hpp"
#include "engine/physics/PhysicsConstants.hpp"
#include <box2d/box2d.h>

namespace game {

EnemyAISystem::EnemyAISystem(engine::EngineContext& context)
    : mContext(context)
    , mRegistry(*context.m_registry)
    , mWorldId(context.m_physicsWorld)
{
    if (context.m_logManager) {
        mLogger = context.m_logManager->GetLogger("EnemyAI");
    }
    
    if (mLogger) {
        mLogger->info("EnemyAISystem initialized.");
    }
}

void EnemyAISystem::update(float fixedDeltaTime) {
    // Phase-based update (will be implemented incrementally)
    updateDetection();
    updateStates(fixedDeltaTime);
    updateMovement(fixedDeltaTime);
    updateCombat();
    updateStuckDetection(fixedDeltaTime);
}

void EnemyAISystem::updateDetection() {
    // Stub for Phase 2
}

void EnemyAISystem::updateStates(float dt) {
    // Stub for Phase 3
}

void EnemyAISystem::updateMovement(float dt) {
    // Stub for Phase 4
}

void EnemyAISystem::updateCombat() {
    // Stub for Phase 5
}

void EnemyAISystem::updateStuckDetection(float dt) {
    // Stub for Phase 6
}

bool EnemyAISystem::hasLineOfSight(const engine::Vector2f& start, const engine::Vector2f& end) {
    b2QueryFilter filter = b2DefaultQueryFilter();
    filter.maskBits = engine::PhysicsCategory::VisibilityBlocker;
    
    // Convert to b2Vec2 which is commonly expected by C++ bindings or constructed from floats
    b2Vec2 startVec = {start.x, start.y};
    b2Vec2 translation = {end.x - start.x, end.y - start.y};
    
    b2RayResult result = b2World_CastRayClosest(mWorldId, startVec, translation, filter);
    return !result.hit;
}

void EnemyAISystem::alertNearbyEnemies(entt::entity source, const engine::Vector2f& position) {
    // Stub for Phase 2
}

engine::Vector2f EnemyAISystem::castRayWithSlide(const engine::Vector2f& origin, 
                                                  const engine::Vector2f& direction, float distance) {
    // Stub for Phase 4
    return origin + direction * distance;
}

} // namespace game
