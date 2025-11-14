#pragma once

#include "engine/core/ILogger.hpp"
#include <memory>

// Forward declaration of spdlog::logger to avoid including the header here
namespace spdlog
{
    class logger;
}

namespace engine
{
    class SpdlogLogger : public ILogger
    {
    public:
        explicit SpdlogLogger(std::shared_ptr<spdlog::logger> logger);
        ~SpdlogLogger() override = default;

        void log(spdlog::level::level_enum level, std::string_view message) override;

    private:
        std::shared_ptr<spdlog::logger> m_logger;
    };
}