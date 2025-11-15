#pragma once

#include <entt/fwd.hpp>

namespace engine
{
    struct EngineContext;
    class IRenderer;

    class RenderSystem
    {
    public:
        RenderSystem(const EngineContext& context);

        void update();

    private:
        entt::registry& m_registry;
        IRenderer& m_renderer;
    };
}
