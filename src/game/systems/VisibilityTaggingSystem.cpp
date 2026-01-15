#include "engine/pch.h"
#include "VisibilityTaggingSystem.hpp"
#include "engine/components/TransformComponent.hpp"
#include "engine/components/RenderableComponent.hpp"
#include "engine/core/math/GeometryUtils.hpp"
#include "engine/core/ILoggerManager.hpp"
#include "game/components/VisibilityComponent.hpp"
#include "game/components/VisibleToPlayerComponent.hpp"
#include "game/components/PlayerComponent.hpp"
#include <entt/entt.hpp>

namespace game
{
    VisibilityTaggingSystem::VisibilityTaggingSystem(engine::EngineContext& context)
        : m_context(context)
    {
        if (context.m_logManager) {
            m_logger = context.m_logManager->GetLogger("VisibilityTaggingSystem");
        }
    }

    void VisibilityTaggingSystem::update()
    {
        auto& registry = *m_context.m_registry;
        
        // 1. Collect all player vision cones
        struct VisionCone {
            std::vector<engine::Vector2f> polygon;
        };
        
        std::vector<VisionCone> visionCones;
        
        auto playerView = registry.view<PlayerComponent, VisibilityComponent>();
        for (auto entity : playerView) {
            const auto& visibility = playerView.get<VisibilityComponent>(entity);
            if (!visibility.visibilityPolygon.empty()) {
                visionCones.push_back({visibility.visibilityPolygon});
            }
        }
        
        // 2. Check all renderable entities (except players themselves)
        // Note: We check entities with RenderableComponent to tag them. 
        // We could also check PhysicsBodyComponent if we wanted to tag invisible physics objects.
        auto entityView = registry.view<engine::TransformComponent, engine::RenderableComponent>();
        
        for (auto entity : entityView) {
            // Skip players - they always see themselves
            if (registry.any_of<PlayerComponent>(entity)) {
                continue;
            }
            
            const auto& transform = entityView.get<engine::TransformComponent>(entity);
            
            // Check if entity position is inside ANY vision cone
            bool isVisible = false;
            for (const auto& cone : visionCones) {
                if (engine::GeometryUtils::isPointInPolygon(transform.position, cone.polygon)) {
                    isVisible = true;
                    break;
                }
            }
            
            // Tag or untag entity
            if (isVisible) {
                if (!registry.any_of<VisibleToPlayerComponent>(entity)) {
                    registry.emplace<VisibleToPlayerComponent>(entity);
                }
            } else {
                // Only remove if NOT always visible
                if (auto* vis = registry.try_get<VisibleToPlayerComponent>(entity)) {
                    if (!vis->alwaysVisible) {
                        registry.remove<VisibleToPlayerComponent>(entity);
                    }
                }
            }
        }
    }
}
