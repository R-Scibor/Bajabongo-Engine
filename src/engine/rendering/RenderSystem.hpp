#pragma once

#include <entt/fwd.hpp>
#include <memory>
#include <SFML/Graphics/RenderTarget.hpp>

namespace engine
{
    struct EngineContext;
    class IRenderer;
    class SpriteManager;
    class IResourceManager;

    class RenderSystem
    {
    public:
        explicit RenderSystem(engine::EngineContext& context);

        void update(sf::RenderTarget& target);

        void setDebugDraw(bool enable);

    private:
        engine::EngineContext& m_context;
        entt::registry& m_registry;
        std::shared_ptr<engine::IRenderer> m_renderer;
        std::shared_ptr<engine::SpriteManager> m_spriteManager;
        std::shared_ptr<engine::IResourceManager> m_resourceManager;
        bool m_debugDraw = false;
    };
}
