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

        void setDebugDraw(bool enabled) { m_debugDraw = enabled; }

    private:
        bool m_debugDraw = false;
        entt::registry& m_registry;
        IRenderer& m_renderer;
        class SpriteManager& m_spriteManager;
    };
}
