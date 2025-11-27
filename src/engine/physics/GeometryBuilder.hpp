#pragma once

#include <vector>
#include <memory>
#include "engine/core/math/MathAliases.hpp"

namespace engine {
    class ILogger;

    class GeometryBuilder {
    public:
        explicit GeometryBuilder(std::shared_ptr<ILogger> logger);

        std::vector<std::vector<Vector2f>> analyzeGrid(const std::vector<std::vector<int>>& grid, float tileSize);

    private:
        struct TileIsland {
            std::vector<Vector2i> tiles;
        };

        std::vector<TileIsland> findIslands(const std::vector<std::vector<int>>& grid);
        std::vector<Vector2f> traceContour(const TileIsland& island, const std::vector<std::vector<int>>& grid, float tileSize);
        std::vector<Vector2f> simplifyContour(const std::vector<Vector2f>& points);

    private:
        std::shared_ptr<ILogger> m_logger;
    };
}