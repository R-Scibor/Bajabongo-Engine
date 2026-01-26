#pragma once

#include <string_view>
#include <utility> // For std::forward

// We need spdlog headers here for the level enum and fmt support.
// This is a necessary coupling for the convenience API.
#include <spdlog/common.h>
#include <fmt/core.h>
#include <fmt/format.h> // For fmt::format

namespace engine
{
    /**
     * @brief Abstract interface for the Logging System.
     * * Provides a standardized way to log messages with different severity levels.
     * * Integrates with `fmt` library for modern, type-safe string formatting.
     */
    class ILogger
    {
    public:
        virtual ~ILogger() = default;

        // --- Interface Contract ---

        /**
         * @brief The core logging function to be implemented by concrete loggers.
         * @param level The severity level of the message (Trace, Info, Error, etc.).
         * @param message The pre-formatted message string to write.
         */
        virtual void log(spdlog::level::level_enum level, std::string_view message) = 0;

        // --- Convenience API (Template Methods) ---
        // These are non-virtual template functions that provide a user-friendly
        // formatting API. They delegate the actual logging to the pure virtual `log` method.

        // --- Backwards Compatibility / Simple Strings ---
        
        /// @brief Logs a trace message (lowest severity).
        void trace(std::string_view msg) { log(spdlog::level::trace, msg); }
        /// @brief Logs a debug message.
        void debug(std::string_view msg) { log(spdlog::level::debug, msg); }
        /// @brief Logs an informational message.
        void info(std::string_view msg) { log(spdlog::level::info, msg); }
        /// @brief Logs a warning message.
        void warn(std::string_view msg) { log(spdlog::level::warn, msg); }
        /// @brief Logs an error message (high severity).
        void error(std::string_view msg) { log(spdlog::level::err, msg); }


        // --- Formatted Logging (fmt style) ---

        /**
         * @brief Logs a formatted trace message.
         * @tparam Args Variadic template arguments for formatting.
         * @param fmt The format string (e.g., "Player {} moved to {}").
         * @param args The arguments to insert into the format string.
         */
        template <typename... Args>
        void trace(fmt::format_string<Args...> fmt, Args&&... args) {
            log(spdlog::level::trace, fmt::format(fmt, std::forward<Args>(args)...));
        }

        /**
         * @brief Logs a formatted debug message.
         */
        template <typename... Args>
        void debug(fmt::format_string<Args...> fmt, Args&&... args) {
            log(spdlog::level::debug, fmt::format(fmt, std::forward<Args>(args)...));
        }

        /**
         * @brief Logs a formatted info message.
         */
        template <typename... Args>
        void info(fmt::format_string<Args...> fmt, Args&&... args) {
            log(spdlog::level::info, fmt::format(fmt, std::forward<Args>(args)...));
        }

        /**
         * @brief Logs a formatted warning message.
         */
        template <typename... Args>
        void warn(fmt::format_string<Args...> fmt, Args&&... args) {
            log(spdlog::level::warn, fmt::format(fmt, std::forward<Args>(args)...));
        }

        /**
         * @brief Logs a formatted error message.
         */
        template <typename... Args>
        void error(fmt::format_string<Args...> fmt, Args&&... args) {
            log(spdlog::level::err, fmt::format(fmt, std::forward<Args>(args)...));
        }
    };
}