#pragma once
#include "engine/core/EngineContext.hpp"
#include <entt/entt.hpp>
#include <memory>
#include "engine/core/ILogger.hpp"
#include "engine/core/math/MathAliases.hpp"

namespace engine {
    struct TransformComponent;
}

namespace game {
    struct EnemyComponent;
}

namespace game::ai {

    class EnemyStateTransitions {
    public:
        explicit EnemyStateTransitions(engine::EngineContext& context);
        void update(float dt);

    private:
        void transitionFromIdle(entt::entity entity, EnemyComponent& enemy, const engine::TransformComponent& transform);
        void transitionFromPatrol(entt::entity entity, EnemyComponent& enemy, const engine::TransformComponent& transform);
        void transitionFromAlert(entt::entity entity, EnemyComponent& enemy, const engine::TransformComponent& transform);
        void transitionFromChase(entt::entity entity, EnemyComponent& enemy, const engine::TransformComponent& transform);
        void transitionFromShoot(entt::entity entity, EnemyComponent& enemy, const engine::TransformComponent& transform);
        void transitionFromRush(entt::entity entity, EnemyComponent& enemy, const engine::TransformComponent& transform);
        void transitionFromRetreat(entt::entity entity, EnemyComponent& enemy, const engine::TransformComponent& transform);
        void transitionFromApproach(entt::entity entity, EnemyComponent& enemy, const engine::TransformComponent& transform);

        engine::EngineContext& mContext;
        std::shared_ptr<engine::ILogger> mLogger;
    };

}
