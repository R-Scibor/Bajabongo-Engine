#pragma once
#include "engine/core/EngineContext.hpp"
#include <entt/entt.hpp>
#include <memory>
#include "engine/core/ILogger.hpp"
#include "engine/core/math/MathAliases.hpp"

namespace engine {
    struct TransformComponent;
    struct PhysicsBodyComponent;
}

namespace game {
    struct EnemyComponent;
}

namespace game::ai {

    class EnemyMovementSystem {
    public:
        explicit EnemyMovementSystem(engine::EngineContext& context);
        void update(float dt);

    private:
        engine::Vector2f calculatePatrolMovement(entt::entity entity, EnemyComponent& enemy, const engine::TransformComponent& transform);
        engine::Vector2f calculateChaseMovement(entt::entity entity, EnemyComponent& enemy, const engine::TransformComponent& transform);
        engine::Vector2f calculateRushMovement(entt::entity entity, EnemyComponent& enemy, const engine::TransformComponent& transform);
        engine::Vector2f calculateRetreatMovement(entt::entity entity, EnemyComponent& enemy, const engine::TransformComponent& transform);
        engine::Vector2f applyWallSliding(const engine::Vector2f& position, const engine::Vector2f& desiredVelocity, float dt);

        engine::EngineContext& mContext;
        std::shared_ptr<engine::ILogger> mLogger;
    };

}
