#pragma once
#include "engine/core/EngineContext.hpp"

namespace game {

class EnemyAISystem {
public:
    explicit EnemyAISystem(engine::EngineContext& context);
    void update(float dt);
    
private:
    engine::EngineContext& m_context;
};

} // namespace game
