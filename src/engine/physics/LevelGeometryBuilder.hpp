#pragma once

#include <vector>
#include <memory>
#include <box2d/id.h>
#include "engine/core/math/MathAliases.hpp"

namespace engine {
    struct EngineContext;
    class ILogger;

    class LevelGeometryBuilder {
    public:
        explicit LevelGeometryBuilder(const EngineContext& context);
        ~LevelGeometryBuilder();

        void createLevelBody(const std::vector<std::vector<Vector2f>>& chains);
        void clear();

    private:
        b2WorldId m_worldId;
        b2BodyId m_levelBodyId;
        std::shared_ptr<ILogger> m_logger;
    };
}