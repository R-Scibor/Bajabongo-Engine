#pragma once

#include <entt/fwd.hpp>

namespace engine
{
    class IRenderer;

    class RenderSystem
    {
    public:
        RenderSystem(entt::registry& registry, IRenderer& renderer);

        void update();

    private:
        entt::registry& m_registry;
        IRenderer& m_renderer;
    };
}
