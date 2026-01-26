#pragma once

#include "engine/core/ILogger.hpp"
#include <memory>

// Forward declaration of spdlog::logger to avoid including heavy headers here.
// This reduces compilation time for files that only include SpdlogLogger.hpp.
namespace spdlog
{
    class logger;
}

namespace engine
{
    /**
     * @brief Concrete implementation of ILogger using the spdlog library.
     *
     * This class acts as an **Adapter**, wrapping a `spdlog::logger` instance
     * and exposing it through the engine's standard `ILogger` interface.
     */
    class SpdlogLogger : public ILogger
    {
    public:
        /**
         * @brief Constructs the wrapper around an existing spdlog logger.
         * @param logger Shared pointer to the underlying spdlog instance.
         */
        explicit SpdlogLogger(std::shared_ptr<spdlog::logger> logger);
        
        ~SpdlogLogger() override = default;

        /**
         * @brief Logs a message with a specific severity level.
         * * Delegates the actual logging work to the wrapped spdlog instance.
         * @param level The severity level (mapped to spdlog enums).
         * @param message The message string view to log.
         */
        void log(spdlog::level::level_enum level, std::string_view message) override;

    private:
        /// @brief The underlying third-party logger instance.
        std::shared_ptr<spdlog::logger> m_logger;
    };
}