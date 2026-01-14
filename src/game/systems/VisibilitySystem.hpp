#pragma once

#include "engine/core/EngineContext.hpp"
#include "engine/core/ILogger.hpp"
#include "game/components/VisibilityComponent.hpp"
#include <box2d/box2d.h>
#include <vector>
#include <memory>

namespace game {

    class VisibilitySystem {
    public:
        explicit VisibilitySystem(engine::EngineContext& context);

        void update();

    private:
        engine::EngineContext& m_context;
        std::shared_ptr<engine::ILogger> m_logger;
        b2WorldId m_worldId;

        // Helper to query world for blocking shapes
        void queryWorld(const engine::Vector2f& center, float radius, std::vector<engine::Vector2f>& wallVertices);
        
        // Raycast helper
        engine::Vector2f castRay(const engine::Vector2f& start, const engine::Vector2f& end);
    };

}
