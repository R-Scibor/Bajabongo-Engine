#include "engine/pch.h"
#include "CameraSystem.hpp"
#include "engine/core/EngineContext.hpp"
#include "engine/rendering/IRenderer.hpp"
#include "engine/components/CameraFocusComponent.hpp"
#include "engine/components/TransformComponent.hpp"
#include "engine/core/ILogger.hpp"
#include "engine/core/ILoggerManager.hpp"
#include <entt/entt.hpp>
#include <algorithm> // for std::min

namespace engine
{
    CameraSystem::CameraSystem(EngineContext& context)
        : m_registry(*context.m_registry)
        , m_renderer(context.m_renderer)
    {
        m_logger = context.m_logManager->GetLogger("CameraSystem");
    }

    void CameraSystem::update(float fixedDeltaTime)
    {
        if (!m_renderer) return;

        auto view = m_registry.view<CameraFocusComponent>();
        
        // We only support following one entity for now. If multiple exist, we pick the first one.
        for (auto entity : view) {
            if (!m_registry.all_of<TransformComponent>(entity)) continue;

            const auto& focus = view.get<CameraFocusComponent>(entity);
            const auto& transform = m_registry.get<TransformComponent>(entity);

            Vector2f currentPos = m_renderer->getViewCenter();
            Vector2f targetPos = transform.position;
            
            // Lerp factor: dt * smoothness
            // Clamp to 1.0f to prevent overshooting
            float t = std::min(fixedDeltaTime * focus.smoothness, 1.0f);
            
            Vector2f newPos = currentPos + (targetPos - currentPos) * t;
            
            m_renderer->setViewCenter(newPos);
            
            // We break after the first camera focus entity found
            break; 
        }
    }
}