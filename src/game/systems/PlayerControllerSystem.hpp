#pragma once

#include <entt/fwd.hpp>

namespace engine {
    struct EngineContext;
}

namespace game {

    class PlayerControllerSystem {
    public:
        explicit PlayerControllerSystem(engine::EngineContext& context);

        void update(float fixedDeltaTime);

    private:
        engine::EngineContext& m_context;
    };

} // namespace game