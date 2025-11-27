#include "engine/pch.h"
#include "GeometryBuilder.hpp"
#include "engine/core/ILogger.hpp"

namespace engine {

    GeometryBuilder::GeometryBuilder(std::shared_ptr<ILogger> logger)
        : m_logger(std::move(logger))
    {
    }

    void GeometryBuilder::analyzeGrid(const std::vector<std::vector<int>>& grid, float tileSize) {
        if (grid.empty()) {
            if (m_logger) m_logger->warn("GeometryBuilder: Grid is empty!");
            return;
        }

        size_t height = grid.size();
        size_t width = grid[0].size();

        if (m_logger) {
            m_logger->info("GeometryBuilder: Analyzing grid of size {}x{} with tileSize {}", width, height, tileSize);
        }

        auto islands = findIslands(grid);
        if (m_logger) {
            m_logger->info("GeometryBuilder: Found {} islands.", islands.size());
            for (size_t i = 0; i < islands.size(); ++i) {
                m_logger->debug("  Island {}: {} tiles", i, islands[i].tiles.size());
            }
        }
    }

    std::vector<GeometryBuilder::TileIsland> GeometryBuilder::findIslands(const std::vector<std::vector<int>>& grid) {
        std::vector<TileIsland> islands;
        if (grid.empty()) return islands;

        int height = static_cast<int>(grid.size());
        int width = static_cast<int>(grid[0].size());

        std::vector<std::vector<bool>> visited(height, std::vector<bool>(width, false));

        // Helper directions for 4-connectivity
        const int dx[] = { 1, -1, 0, 0 };
        const int dy[] = { 0, 0, 1, -1 };

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                // If it's a wall (1) and not visited yet
                if (grid[y][x] == 1 && !visited[y][x]) {
                    TileIsland island;
                    std::vector<Vector2i> stack;
                    stack.push_back({ x, y });
                    visited[y][x] = true;

                    while (!stack.empty()) {
                        Vector2i current = stack.back();
                        stack.pop_back();
                        island.tiles.push_back(current);

                        for (int i = 0; i < 4; ++i) {
                            int nx = current.x + dx[i];
                            int ny = current.y + dy[i];

                            if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                                if (grid[ny][nx] == 1 && !visited[ny][nx]) {
                                    visited[ny][nx] = true;
                                    stack.push_back({ nx, ny });
                                }
                            }
                        }
                    }
                    islands.push_back(island);
                }
            }
        }

        return islands;
    }

}