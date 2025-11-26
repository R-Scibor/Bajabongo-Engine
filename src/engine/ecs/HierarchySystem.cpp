#include "engine/pch.h"
#include "engine/ecs/HierarchySystem.hpp"

#include "engine/core/EngineContext.hpp"
#include "engine/core/ILogger.hpp"
#include "engine/core/ILoggerManager.hpp"

#include "engine/components/ParentComponent.hpp"
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
                // Orphan policy: for now, we just log a warning.
                // In the future, we might want to destroy the entity or detach it.
                // To avoid spamming logs every frame, we might want a 'dirty' flag or just ensure logic cleans up properly.
                // For now, let's not spam:
                // m_logger->warn("Entity {} has invalid parent {}", (uint32_t)entity, (uint32_t)parentComp.parentId);
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
            
            // Optional: Propagate scale?
            // Usually hierarchy systems might multiply scale, but sometimes you want child scale to be independent (except for position).
            // For simple attachment (like holding a gun), usually we just want position/rotation sync.
            // If we wanted full scene graph scaling: transform.scale = parentTransform.scale * childLocalScale.
            // Given the current requirements, we'll leave child's own scale alone (it acts as local scale).
        });
    }
}