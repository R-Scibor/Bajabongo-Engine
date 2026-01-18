#pragma once

#include <entt/fwd.hpp>
#include <random>

namespace engine {
    struct EngineContext;
    struct PhysicsBodyComponent;
}

namespace game {
    struct WeaponComponent;

    class WeaponSystem {
    public:
        explicit WeaponSystem(engine::EngineContext& context);

        void update(float fixedDeltaTime);

    private:
        bool canFire(const WeaponComponent& weapon) const;
        void applyFiringEffects(WeaponComponent& weapon, float dt);
        void spawnProjectiles(engine::EngineContext& context, entt::entity entity, const WeaponComponent& weapon, const engine::PhysicsBodyComponent& bodyComp);

        engine::EngineContext& m_context;
        std::mt19937 m_rng;
    };

} // namespace game