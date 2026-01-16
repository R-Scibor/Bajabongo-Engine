#pragma once
#include "engine/core/EngineContext.hpp"
#include "engine/core/math/MathAliases.hpp"
#include <box2d/box2d.h>

namespace game::ai {

    class AIUtils {
    public:
        static bool hasLineOfSight(engine::EngineContext& context, const engine::Vector2f& start, const engine::Vector2f& end);
        static bool isPositionVisibleToPlayer(engine::EngineContext& context, const engine::Vector2f& position);
    };

}
