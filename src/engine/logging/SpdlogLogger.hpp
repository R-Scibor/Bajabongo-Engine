#pragma once

#include "engine/core/ILogger.hpp"
#include <memory>

// Forward declaration of spdlog::logger to avoid including the header here
namespace spdlog
{
    class logger;
}

namespace Bajabongo
{
    class SpdlogLogger : public ILogger
    {
    public:
        explicit SpdlogLogger(std::shared_ptr<spdlog::logger> logger);
        ~SpdlogLogger() override = default;

        void trace(std::string_view message) override;
        void debug(std::string_view message) override;
        void info(std::string_view message) override;
        void warn(std::string_view message) override;
        void error(std::string_view message) override;

    private:
        std::shared_ptr<spdlog::logger> m_logger;
    };
}