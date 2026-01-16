#pragma once
#include "engine/core/EngineContext.hpp"
#include <memory>
#include <entt/entt.hpp>
#include <box2d/box2d.h>

namespace engine {
    class ILogger;
}

namespace game::ai {
    class EnemyDetectionSystem;
    class EnemyStateTransitions;
    class EnemyMovementSystem;
    class EnemyCombatSystem;
    class EnemyStuckRecoverySystem;
}

namespace game {

class EnemyAISystem {
public:
    explicit EnemyAISystem(engine::EngineContext& context);
    ~EnemyAISystem(); 
    
    void update(float fixedDeltaTime);
    
private:
    engine::EngineContext& mContext;
    std::shared_ptr<engine::ILogger> mLogger;

    std::unique_ptr<game::ai::EnemyDetectionSystem> mDetectionSystem;
    std::unique_ptr<game::ai::EnemyStateTransitions> mStateSystem;
    std::unique_ptr<game::ai::EnemyMovementSystem> mMovementSystem;
    std::unique_ptr<game::ai::EnemyCombatSystem> mCombatSystem;
    std::unique_ptr<game::ai::EnemyStuckRecoverySystem> mStuckSystem;
};

} // namespace game
