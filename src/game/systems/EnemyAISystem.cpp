#include "engine/pch.h"
#include "EnemyAISystem.hpp"
#include "engine/core/ILoggerManager.hpp"

#include "game/ai/EnemyDetectionSystem.hpp"
#include "game/ai/EnemyStateTransitions.hpp"
#include "game/ai/EnemyMovementSystem.hpp"
#include "game/ai/EnemyCombatSystem.hpp"
#include "game/ai/EnemyStuckRecoverySystem.hpp"

namespace game {

EnemyAISystem::EnemyAISystem(engine::EngineContext& context)
    : mContext(context)
{
    if (context.m_logManager) {
        mLogger = context.m_logManager->GetLogger("EnemyAI");
    }
    
    if (mLogger) {
        mLogger->info("EnemyAISystem initialized (Composite).");
    }

    mDetectionSystem = std::make_unique<ai::EnemyDetectionSystem>(context);
    mStateSystem = std::make_unique<ai::EnemyStateTransitions>(context);
    mMovementSystem = std::make_unique<ai::EnemyMovementSystem>(context);
    mCombatSystem = std::make_unique<ai::EnemyCombatSystem>(context);
    mStuckSystem = std::make_unique<ai::EnemyStuckRecoverySystem>(context);
}

EnemyAISystem::~EnemyAISystem() = default;

void EnemyAISystem::update(float fixedDeltaTime) {
    if (mDetectionSystem) mDetectionSystem->update(fixedDeltaTime);
    if (mStateSystem) mStateSystem->update(fixedDeltaTime);
    if (mMovementSystem) mMovementSystem->update(fixedDeltaTime);
    if (mCombatSystem) mCombatSystem->update(fixedDeltaTime);
    if (mStuckSystem) mStuckSystem->update(fixedDeltaTime);
}

} // namespace game
