#include "SpdlogLogger.hpp"
#include <spdlog/logger.h>

namespace Bajabongo
{
    SpdlogLogger::SpdlogLogger(std::shared_ptr<spdlog::logger> logger)
        : m_logger(std::move(logger))
    {
    }

    void SpdlogLogger::trace(std::string_view message)
    {
        m_logger->trace(message);
    }

    void SpdlogLogger::debug(std::string_view message)
    {
        m_logger->debug(message);
    }

    void SpdlogLogger::info(std::string_view message)
    {
        m_logger->info(message);
    }

    void SpdlogLogger::warn(std::string_view message)
    {
        m_logger->warn(message);
    }

    void SpdlogLogger::error(std::string_view message)
    {
        m_logger->error(message);
    }
}