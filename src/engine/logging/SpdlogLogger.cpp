#include "SpdlogLogger.hpp"
#include <spdlog/logger.h>

namespace engine
{
    SpdlogLogger::SpdlogLogger(std::shared_ptr<spdlog::logger> logger)
        : m_logger(std::move(logger))
    {
    }

    void SpdlogLogger::log(spdlog::level::level_enum level, std::string_view message)
    {
        m_logger->log(level, message);
    }
}