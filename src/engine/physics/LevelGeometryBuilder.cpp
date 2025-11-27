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
                // Filter out points that are too close to the previous point
                if (!b2Points.empty()) {
                    float dx = p.x - b2Points.back().x;
                    float dy = p.y - b2Points.back().y;
                    if (dx*dx + dy*dy < 0.0001f) continue;
                }
                b2Points.push_back({ p.x, p.y });
            }
            
            // Also check closure point distance if looping
             if (b2Points.size() > 1 &&
                 points.front().x == points.back().x &&
                 points.front().y == points.back().y) {
                 // It is a loop, so the duplicate is already there (or we check against first)
                 // My previous logic assumed points.front() == points.back() exactly.
                 // If they are "close enough" maybe we should snap?
                 // For now, just rely on exact match from GeometryBuilder.
             }

            if (b2Points.size() < 3) continue; // Minimum for chain? Maybe 2 for open.

            b2ChainDef chainDef = b2DefaultChainDef();
            chainDef.points = b2Points.data();
            chainDef.count = static_cast<int>(b2Points.size());
            
            // Check if loop
            if (points.size() > 2 &&
                points.front().x == points.back().x &&
                points.front().y == points.back().y) {
                chainDef.isLoop = true;
                if (chainDef.isLoop) {
                    chainDef.count--; // Exclude the last duplicate point
                }
            } else {
                chainDef.isLoop = false;
            }

            // Validate chain
            if (chainDef.isLoop && chainDef.count < 3) {
                m_logger->warn("Skipping degenerate loop chain with {} points.", chainDef.count);
                continue;
            }
            if (!chainDef.isLoop && chainDef.count < 2) {
                m_logger->warn("Skipping degenerate chain with {} points.", chainDef.count);
                continue;
            }

            if (m_logger) {
                 m_logger->debug("Creating chain: Loop={}, Count={}", chainDef.isLoop, chainDef.count);
            }

            b2CreateChain(m_levelBodyId, &chainDef);
            chainCount++;
        }

        m_logger->info("Created level body with {} chains.", chainCount);
    }

}