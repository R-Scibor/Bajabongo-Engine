#pragma once
#include <memory>

namespace engine {
    struct EngineContext;
    class ILogger;

    class AnimationSystem {
    public:
        explicit AnimationSystem(EngineContext& context);
        
        void update(float deltaTime);

    private:
        EngineContext& m_context;
        std::shared_ptr<ILogger> m_logger;
    };
}
