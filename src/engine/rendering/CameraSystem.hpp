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
    
    /**
     * @brief System that controls the camera's position and zoom level.
     * * Implements smooth following (interpolation) and mouse wheel zooming.
     */
    class CameraSystem
    {
    public:
        explicit CameraSystem(EngineContext& context);

        /**
         * @brief Updates camera transform.
         * * Should be called in the fixed update or late update phase to ensure
         * the target entity has already moved for this frame.
         */
        void update(float fixedDeltaTime);

    private:
        entt::registry& m_registry;
        std::shared_ptr<IRenderer> m_renderer;
        std::shared_ptr<ILogger> m_logger;
        std::shared_ptr<IInputManager> m_inputManager;
    };
}