#pragma once
#include "engine/core/EngineContext.hpp"
#include <memory>

namespace engine { class ILogger; }

namespace game
{
    class VisibilityTaggingSystem
    {
    public:
        explicit VisibilityTaggingSystem(engine::EngineContext& context);
        
        void update();
        
    private:
        engine::EngineContext& m_context;
        std::shared_ptr<engine::ILogger> m_logger;
    };
}
