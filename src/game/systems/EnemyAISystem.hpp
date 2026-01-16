#pragma once
#include "engine/core/EngineContext.hpp"
#include "engine/core/ILogger.hpp"
#include "engine/core/math/MathAliases.hpp"
#include <memory>
#include <entt/entt.hpp>
#include <box2d/box2d.h>

namespace game {

class EnemyAISystem {
public:
    explicit EnemyAISystem(engine::EngineContext& context);
    
    void update(float fixedDeltaTime);
    
private:
    // Update Phases
    void updateDetection();
    void updateStates(float dt);
    void updateMovement(float dt);
    void updateCombat();
    void updateStuckDetection(float dt);
    
    // Helper Methods
    bool hasLineOfSight(const engine::Vector2f& start, const engine::Vector2f& end);
    engine::Vector2f castRayWithSlide(const engine::Vector2f& origin, const engine::Vector2f& direction, float distance);
    void alertNearbyEnemies(entt::entity source, const engine::Vector2f& position);
    
    engine::EngineContext& mContext;
    entt::registry& mRegistry;
    b2WorldId mWorldId;
    std::shared_ptr<engine::ILogger> mLogger;
};

} // namespace game
