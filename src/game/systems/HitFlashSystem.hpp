#pragma once

#include "engine/core/EngineContext.hpp"
#include <entt/entt.hpp>

namespace game {

    class HitFlashSystem {
    public:
        explicit HitFlashSystem(engine::EngineContext& context);
        ~HitFlashSystem() = default;

        void update(float dt);

    private:
        engine::EngineContext& m_context;
    };

}
