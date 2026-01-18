#pragma once

#include <entt/fwd.hpp>
#include <string>

namespace engine {
    struct EngineContext;
}

namespace game {

    class WeaponSwitchSystem {
    public:
        void update(engine::EngineContext& context, float dt);

    private:
        int getRequestedSlot(engine::EngineContext& context) const;
        void loadWeaponStats(engine::EngineContext& context, entt::entity entity, const std::string& archetypeId);
    };

} // namespace game
