#include "engine/pch.h"
#include "RenderSystem.hpp"

#include <entt/entt.hpp>

#include "engine/core/EngineContext.hpp"
#include "engine/components/TransformComponent.hpp"
#include "engine/components/RenderableComponent.hpp"
#include "engine/rendering/IRenderer.hpp"

namespace engine
{
    RenderSystem::RenderSystem(const EngineContext& context)
        : m_registry{ *context.m_registry }
        , m_renderer{ *context.m_renderer }
    {
    }

    void RenderSystem::update()
    {
        auto view = m_registry.view<TransformComponent, RenderableComponent>();

        for (auto entity : view)
        {
            auto& transform = view.get<TransformComponent>(entity);
            auto& renderable = view.get<RenderableComponent>(entity);

            m_renderer.drawShape(transform.position, renderable.radius);
        }
    }
}
