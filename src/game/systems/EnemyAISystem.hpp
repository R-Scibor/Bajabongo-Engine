#pragma once
#include "engine/core/EngineContext.hpp"
#include "engine/core/ILogger.hpp"
#include "engine/core/math/MathAliases.hpp"
#include <memory>
#include <entt/entt.hpp>
#include <box2d/box2d.h>

namespace engine {
    struct TransformComponent;
}

namespace game {

struct EnemyComponent;

class EnemyAISystem {
public:
    explicit EnemyAISystem(engine::EngineContext& context);
    
    void update(float fixedDeltaTime);
    
private:
    // Update Phases
    void updateDetection(float dt);
    void updateStates(float dt);
    void updateMovement(float dt);
    void updateCombat(float dt);
    void updateStuckDetection(float dt);

    void executeUnstuckBehavior(entt::entity entity, EnemyComponent& enemy, 
                                const engine::TransformComponent& transform);
    bool isPositionVisibleToPlayer(const engine::Vector2f& position);
    engine::Vector2f findSafeUnstuckPosition(entt::entity entity, 
                                              const engine::Vector2f& preferredPos);
    
    // State Transition Helpers
    void transitionFromIdle(entt::entity entity, EnemyComponent& enemy, const engine::TransformComponent& transform);
    void transitionFromPatrol(entt::entity entity, EnemyComponent& enemy, const engine::TransformComponent& transform);
    void transitionFromAlert(entt::entity entity, EnemyComponent& enemy, const engine::TransformComponent& transform);
    void transitionFromChase(entt::entity entity, EnemyComponent& enemy, const engine::TransformComponent& transform);
    void transitionFromShoot(entt::entity entity, EnemyComponent& enemy, const engine::TransformComponent& transform);
    
    // Movement Helpers
    engine::Vector2f calculatePatrolMovement(entt::entity entity, EnemyComponent& enemy, const engine::TransformComponent& transform);
    engine::Vector2f calculateChaseMovement(entt::entity entity, EnemyComponent& enemy, const engine::TransformComponent& transform);
    engine::Vector2f calculateRushMovement(entt::entity entity, EnemyComponent& enemy, const engine::TransformComponent& transform);
    engine::Vector2f calculateRetreatMovement(entt::entity entity, EnemyComponent& enemy, const engine::TransformComponent& transform);
    engine::Vector2f applyWallSliding(const engine::Vector2f& position, const engine::Vector2f& desiredVelocity, float dt);

    // Helper Methods
    bool hasLineOfSight(const engine::Vector2f& start, const engine::Vector2f& end);
    void alertNearbyEnemies(entt::entity source, const engine::Vector2f& position);
    
    engine::EngineContext& mContext;
    entt::registry& mRegistry;
    b2WorldId mWorldId;
    std::shared_ptr<engine::ILogger> mLogger;
};

} // namespace game
