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

    class EnemyStuckRecoverySystem {
    public:
        explicit EnemyStuckRecoverySystem(engine::EngineContext& context);
        void update(float dt);

    private:
        void executeUnstuckBehavior(entt::entity entity, EnemyComponent& enemy, const engine::TransformComponent& transform);
        engine::Vector2f findSafeUnstuckPosition(entt::entity entity, const engine::Vector2f& preferredPos);

        engine::EngineContext& mContext;
        std::shared_ptr<engine::ILogger> mLogger;
    };

}
