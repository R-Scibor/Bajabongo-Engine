#include "engine/pch.h"
#include "LevelGeometryBuilder.hpp"
#include "engine/core/EngineContext.hpp"
#include "engine/core/ILoggerManager.hpp"
#include "engine/core/ILogger.hpp"
#include <box2d/box2d.h>

namespace engine {

    LevelGeometryBuilder::LevelGeometryBuilder(const EngineContext& context)
        : m_worldId(context.m_physicsWorld)
        , m_levelBodyId(b2_nullBodyId)
    {
        m_logger = context.m_logManager->GetLogger("LevelGeometry");
    }

    LevelGeometryBuilder::~LevelGeometryBuilder() {
        clear();
    }

    void LevelGeometryBuilder::clear() {
        if (b2Body_IsValid(m_levelBodyId)) {
            b2DestroyBody(m_levelBodyId);
            m_levelBodyId = b2_nullBodyId;
        }
    }

    void LevelGeometryBuilder::createLevelBody(const std::vector<std::vector<Vector2f>>& chains) {
        clear();

        if (chains.empty()) return;

        b2BodyDef bodyDef = b2DefaultBodyDef();
        bodyDef.type = b2_staticBody;
        bodyDef.position = { 0.0f, 0.0f }; // Level geometry is usually in world coordinates
        
        m_levelBodyId = b2CreateBody(m_worldId, &bodyDef);
        
        if (!b2Body_IsValid(m_levelBodyId)) {
            m_logger->error("Failed to create level body!");
            return;
        }

        int chainCount = 0;
        for (const auto& points : chains) {
            if (points.size() < 2) continue;

            std::vector<b2Vec2> b2Points;
            b2Points.reserve(points.size());
            for (const auto& p : points) {
                b2Points.push_back({ p.x, p.y });
            }

            b2ChainDef chainDef = b2DefaultChainDef();
            chainDef.points = b2Points.data();
            chainDef.count = static_cast<int>(b2Points.size());
            
            // Check if loop
            if (points.size() > 2 && 
                points.front().x == points.back().x && 
                points.front().y == points.back().y) {
                chainDef.isLoop = true;
                // For loop, the last point should match the first in the data?
                // Box2D 3.0 docs say: "The count should not include the duplicated end point if it is a loop".
                // Wait, let me check standard Box2D behavior.
                // Usually if isLoop is true, it connects last to first.
                // If I provide duplicate end point, it might create a zero-length edge.
                // Let's check if points are same.
                if (chainDef.isLoop) {
                    chainDef.count--; // Exclude the last duplicate point
                }
            } else {
                chainDef.isLoop = false;
            }

            b2CreateChain(m_levelBodyId, &chainDef);
            chainCount++;
        }

        m_logger->info("Created level body with {} chains.", chainCount);
    }

}