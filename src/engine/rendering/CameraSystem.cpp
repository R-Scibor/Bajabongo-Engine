#include "engine/pch.h"
#include "CameraSystem.hpp"
#include "engine/core/EngineContext.hpp"
#include "engine/rendering/IRenderer.hpp"
#include "engine/components/CameraFocusComponent.hpp"
#include "engine/components/TransformComponent.hpp"
#include "engine/core/ILogger.hpp"
#include "engine/core/ILoggerManager.hpp"
#include "engine/core/IInputManager.hpp"
#include <entt/entt.hpp>
#include <algorithm> // for std::min, std::clamp

namespace engine
{
    CameraSystem::CameraSystem(EngineContext& context)
        : m_registry(*context.m_registry)
        , m_renderer(context.m_renderer)
        , m_inputManager(context.m_inputManager)
    {
        m_logger = context.m_logManager->GetLogger("CameraSystem");
    }

    void CameraSystem::update(float fixedDeltaTime)
    {
        if (!m_renderer) return;

        auto view = m_registry.view<CameraFocusComponent>();
        
        // Currently, we find the first entity with a focus component and follow it.
        // In the future, we might handle multiple focus points or camera priorities.
        for (auto entity : view) {
            // Ensure the target actually has a position (Transform)
            if (!m_registry.all_of<TransformComponent>(entity)) continue;

            auto& focus = view.get<CameraFocusComponent>(entity);
            const auto& transform = m_registry.get<TransformComponent>(entity);

            // --- 1. Handle Zoom Input ---
            if (m_inputManager) {
                // Consume scroll delta so it isn't processed multiple times
                float scrollDelta = m_inputManager->consumeMouseScrollDelta();
                if (scrollDelta != 0.0f) {
                    focus.targetViewHeight -= scrollDelta * focus.zoomSpeed;
                    focus.targetViewHeight = std::clamp(focus.targetViewHeight, focus.minZoom, focus.maxZoom);
                }
            }

            // Smoothly interpolate current view height towards target
            // Basic lerp: value += (target - value) * factor
            focus.viewHeight += (focus.targetViewHeight - focus.viewHeight) * focus.smoothness;

            // Apply Zoom to Renderer
            Vector2u windowSize = m_renderer->getWindowSize();
            if (windowSize.y > 0) {
                float aspectRatio = static_cast<float>(windowSize.x) / static_cast<float>(windowSize.y);
                float viewHeight = focus.viewHeight;
                float viewWidth = viewHeight * aspectRatio;
                m_renderer->setViewSize({viewWidth, viewHeight});
            }

            // --- 2. Handle Movement (Follow) ---
            Vector2f currentPos = m_renderer->getViewCenter();
            Vector2f targetPos = transform.position;
            
            // Calculate interpolation factor (t)
            // Using 'smoothness' directly. For frame-rate independence, this should mathematically 
            // generally be: 1.0f - std::pow(base, -dt). But for fixed updates, simple factor works.
            float t = std::clamp(focus.smoothness, 0.0f, 1.0f);
            
            // Linear Interpolation (Lerp)
            Vector2f newPos = currentPos + (targetPos - currentPos) * t;
            
            m_renderer->setViewCenter(newPos);
            
            // Only process one camera focus entity per frame
            break;
        }
    }
}