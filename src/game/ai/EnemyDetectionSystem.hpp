#pragma once

#include "engine/core/EngineContext.hpp"
#include "engine/core/math/MathAliases.hpp"

#include <entt/fwd.hpp>
#include <memory>

namespace engine {
    class ILogger;
}

namespace game::ai {

    class EnemyDetectionSystem {
    public:
        explicit EnemyDetectionSystem(engine::EngineContext& context);
        void update(float dt);

    private:
        void alertNearbyEnemies(entt::entity source, const engine::Vector2f& position);

        engine::EngineContext& mContext;
        std::shared_ptr<engine::ILogger> mLogger;
    };

} // namespace game::ai
