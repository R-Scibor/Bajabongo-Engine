#pragma once
#include <memory>

namespace engine {
    struct EngineContext;
    class ILogger;

    /**
     * @brief System responsible for updating animation states and syncing them to rendering.
     * * Operates on entities with both `AnimationComponent` and `RenderableComponent`.
     */
    class AnimationSystem {
    public:
        explicit AnimationSystem(EngineContext& context);
        
        /**
         * @brief Advances animation timers and updates sprite frames.
         * @param deltaTime Time elapsed since the last frame (in seconds).
         */
        void update(float deltaTime);

    private:
        EngineContext& m_context;
        std::shared_ptr<ILogger> m_logger;
    };
}