#include "engine/pch.h"
#include "RenderSystem.hpp"

#include <entt/entt.hpp>

#include "engine/core/EngineContext.hpp"
#include "engine/components/TransformComponent.hpp"
#include "engine/components/RenderableComponent.hpp"
#include "engine/rendering/IRenderer.hpp"
#include "engine/rendering/Sprite.hpp"

namespace engine
{
    RenderSystem::RenderSystem(const EngineContext& context)
        : m_registry{ *context.m_registry }
        , m_renderer{ *context.m_renderer }
        , m_spriteManager{ *context.m_spriteManager }
    {
    }

    void RenderSystem::update()
    {
        auto view = m_registry.view<TransformComponent, RenderableComponent>();

        // Sort by layer (ascending)
        m_registry.sort<RenderableComponent>([](const auto& lhs, const auto& rhs) {
            return lhs.layer < rhs.layer;
        });

        for (auto entity : view)
        {
            auto& transform = view.get<TransformComponent>(entity);
            auto& renderable = view.get<RenderableComponent>(entity);

            // Fetch sprite description
            const auto* spriteDesc = m_spriteManager.getSprite(renderable.spriteId);
            if (spriteDesc) {
                // Draw using the renderer
                m_renderer.drawSprite(*spriteDesc, transform);
            }
        }
    }
}
