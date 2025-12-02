#pragma once

#include <entt/fwd.hpp>

namespace engine {
    struct EngineContext;
}

namespace game {

    class WeaponSystem {
    public:
        explicit WeaponSystem(engine::EngineContext& context);

        void update(float fixedDeltaTime);

    private:
        engine::EngineContext& m_context;
    };

} // namespace game