#include "engine/pch.h"
#include "engine/ecs/HierarchySystem.hpp"

#include "engine/core/EngineContext.hpp"
#include "engine/core/ILogger.hpp"
#include "engine/core/ILoggerManager.hpp"

#include "engine/components/ParentComponent.hpp"
#include "engine/components/ChildComponent.hpp"
#include "engine/components/TransformComponent.hpp"

#include <cmath>

namespace engine
{
    HierarchySystem::HierarchySystem(const EngineContext& context)
        : m_registry(*context.m_registry)
    {
        m_logger = context.m_logManager->GetLogger("HierarchySystem");
        m_logger->info("HierarchySystem initialized.");
    }

    void HierarchySystem::update()
    {
        auto view = m_registry.view<ParentComponent, TransformComponent>();

        view.each([this](entt::entity entity, const ParentComponent& parentComp, TransformComponent& transform)
        {
            if (!m_registry.valid(parentComp.parentId))
            {
                // Orphaned entity: Invalid parent
                return;
            }

            // Ensure parent has a transform
            if (!m_registry.all_of<TransformComponent>(parentComp.parentId))
            {
                return;
            }

            const auto& parentTransform = m_registry.get<TransformComponent>(parentComp.parentId);

            // Calculate Child's World Rotation
            // Rotations are in radians
            transform.rotation = parentTransform.rotation + parentComp.localRotation;

            // Calculate Child's World Position
            // We need to rotate the local offset by the parent's rotation and scale it by parent's scale
            
            float c = std::cos(parentTransform.rotation);
            float s = std::sin(parentTransform.rotation);

            Vector2f scaledOffset = {
                parentComp.localPosition.x * parentTransform.scale.x,
                parentComp.localPosition.y * parentTransform.scale.y
            };

            float rotatedX = scaledOffset.x * c - scaledOffset.y * s;
            float rotatedY = scaledOffset.x * s + scaledOffset.y * c;

            transform.position = parentTransform.position + Vector2f{ rotatedX, rotatedY };
            
            // Note: Scale is not propagated to child (independent scaling)
        });
    }

    void HierarchySystem::onParentDestroyed(entt::registry& registry, entt::entity entity)
    {
        // We use try_get because during destruction, some components might be in flux,
        // although on_destroy triggers before the component is removed from the pool.
        if (auto* childComp = registry.try_get<ChildComponent>(entity))
        {
            m_logger->trace("Entity {} destroyed. Cascading destruction to {} children.",
                entt::to_integral(entity), childComp->children.size());

            for (auto child : childComp->children)
            {
                if (registry.valid(child))
                {
                    // Recursively destroy child.
                    // This will trigger on_destroy<ChildComponent> for the child if it has children.
                    registry.destroy(child);
                }
            }
        }
    }

}