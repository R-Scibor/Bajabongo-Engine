#include "engine/pch.h"
#include "RenderSystem.hpp"

#include <entt/entt.hpp>

#include "engine/components/TransformComponent.hpp"
#include "engine/components/RenderableComponent.hpp"
#include "engine/rendering/IRenderer.hpp"

namespace engine
{
    RenderSystem::RenderSystem(entt::registry& registry, IRenderer& renderer)
        : m_registry(registry)
        , m_renderer(renderer)
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
