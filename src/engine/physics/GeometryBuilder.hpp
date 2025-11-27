#pragma once

#include <vector>
#include <memory>

namespace engine {
    class ILogger;

    class GeometryBuilder {
    public:
        explicit GeometryBuilder(std::shared_ptr<ILogger> logger);

        void analyzeGrid(const std::vector<std::vector<int>>& grid, float tileSize);

    private:
        std::shared_ptr<ILogger> m_logger;
    };
}