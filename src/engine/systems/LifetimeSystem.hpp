#pragma once

#include <entt/fwd.hpp>

namespace engine {
    struct EngineContext;

    class LifetimeSystem {
    public:
        explicit LifetimeSystem(EngineContext& context);

        void update(float fixedDeltaTime);

    private:
        EngineContext& m_context;
    };
}