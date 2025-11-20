#pragma once

#include <entt/fwd.hpp>
#include <memory>

namespace engine
{
    struct EngineContext;
    class IRenderer;
    class ILogger;

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
        std::shared_ptr<ILogger> m_logger;
    };
}
