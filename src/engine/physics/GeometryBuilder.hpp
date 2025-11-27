#pragma once

#include <vector>
#include <memory>
#include "engine/core/math/MathAliases.hpp"

namespace engine {
    class ILogger;

    /**
     * @brief Responsible for analyzing grid-based level data and generating optimized geometry.
     *
     * This class handles the conversion of a 2D tile grid into a set of vector chains (polygons/lines),
     * suitable for physics or rendering. It performs island detection, contour tracing, and vertex simplification.
     */
    class GeometryBuilder {
    public:
        explicit GeometryBuilder(std::shared_ptr<ILogger> logger);

        /**
         * @brief Analyzes the provided grid and generates geometry chains.
         *
         * @param grid A 2D vector representing the level (1 = solid, 0 = empty).
         * @param tileSize The size of a single tile in world units.
         * @return A list of chains, where each chain is a list of vertices.
         */
        std::vector<std::vector<Vector2f>> analyzeGrid(const std::vector<std::vector<int>>& grid, float tileSize);

    private:
        struct TileIsland {
            std::vector<Vector2i> tiles;
        };

        /**
         * @brief Groups connected solid tiles into islands using flood fill.
         */
        std::vector<TileIsland> findIslands(const std::vector<std::vector<int>>& grid);

        /**
         * @brief Traces the outer contour of an island using a wall-following algorithm.
         */
        std::vector<Vector2f> traceContour(const TileIsland& island, const std::vector<std::vector<int>>& grid, float tileSize);

        /**
         * @brief Simplifies a chain of vertices by removing collinear points.
         */
        std::vector<Vector2f> simplifyContour(const std::vector<Vector2f>& points);

    private:
        std::shared_ptr<ILogger> m_logger;
    };
}