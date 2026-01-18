#pragma once

#include <entt/fwd.hpp>
#include <memory>
#include "engine/core/math/Vector2.hpp"

namespace engine
{
    struct EngineContext;
    class IRenderer;
    class ILogger;
    class IInputManager;
    
    class CameraSystem
    {
    public:
        explicit CameraSystem(EngineContext& context);

        void update(float fixedDeltaTime);

    private:
        entt::registry& m_registry;
        std::shared_ptr<IRenderer> m_renderer;
        std::shared_ptr<ILogger> m_logger;
        std::shared_ptr<IInputManager> m_inputManager;
    };
}