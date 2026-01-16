#pragma once
#include "engine/core/EngineContext.hpp"
#include <entt/entt.hpp>
#include <memory>
#include "engine/core/ILogger.hpp"

namespace game::ai {

    class EnemyCombatSystem {
    public:
        explicit EnemyCombatSystem(engine::EngineContext& context);
        void update(float dt);

    private:
        engine::EngineContext& mContext;
        std::shared_ptr<engine::ILogger> mLogger;
    };

}
