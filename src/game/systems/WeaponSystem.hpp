#pragma once

#include <entt/fwd.hpp>
#include <random>

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
        std::mt19937 m_rng;
    };

} // namespace game