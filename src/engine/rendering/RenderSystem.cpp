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
        : m_registry(*context.registry)
        , m_renderer(*context.renderer)
    {
    }

    void RenderSystem::update()
    {
        // Wszystkie encje, które mają pozycję i są renderowalne
        auto view = m_registry.view<TransformComponent, RenderableComponent>();

        for (auto entity : view)
        {
            auto& transform = view.get<TransformComponent>(entity);
            auto& renderable = view.get<RenderableComponent>(entity);

            // Wykorzystujemy abstrakcyjny renderer
            m_renderer.drawShape(transform.position, renderable.radius);
        }
    }
}
