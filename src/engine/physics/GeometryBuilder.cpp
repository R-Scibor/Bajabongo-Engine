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
    }

}