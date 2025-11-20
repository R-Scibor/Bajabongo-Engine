#include "engine/pch.h"
#include "RenderSystem.hpp"

#include <vector>
#include <algorithm>
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

        struct RenderItem {
            int layer;
            float y;
            entt::entity entity;
        };

        std::vector<RenderItem> items;
        items.reserve(view.size_hint());

        view.each([&](auto entity, const auto& transform, const auto& renderable) {
            items.push_back({ renderable.layer, transform.position.y, entity });
        });

        std::sort(items.begin(), items.end(), [](const RenderItem& a, const RenderItem& b) {
            if (a.layer != b.layer) {
                return a.layer < b.layer;
            }
            return a.y < b.y;
        });

        for (const auto& item : items)
        {
            auto& transform = view.get<TransformComponent>(item.entity);
            // We already have layer, but we need other renderable data (spriteId)
            // Re-fetching renderable is cheap/fast reference
            auto& renderable = view.get<RenderableComponent>(item.entity);

            // Fetch sprite description
            const auto* spriteDesc = m_spriteManager.getSprite(renderable.spriteId);
            if (spriteDesc) {
                // Draw using the renderer
                m_renderer.drawSprite(*spriteDesc, transform);
            }
        }
    }
}
