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
    class ILogger
    {
    public:
        virtual ~ILogger() = default;

        // --- Interface Contract ---
        // The single, pure virtual function that all concrete loggers must implement.
        // This is the core of the abstraction.
        virtual void log(spdlog::level::level_enum level, std::string_view message) = 0;

        // --- Convenience API (Template Methods) ---
        // These are non-virtual template functions that provide a user-friendly
        // formatting API. They delegate the actual logging to the pure virtual `log` method.
        // Because they are templates, their implementation must be in the header.

        // --- Backwards Compatibility ---
        // These overloads are provided for backwards compatibility with code
        // that uses std::stringstream or passes a std::string.
        void trace(std::string_view msg) { log(spdlog::level::trace, msg); }
        void debug(std::string_view msg) { log(spdlog::level::debug, msg); }
        void info(std::string_view msg) { log(spdlog::level::info, msg); }
        void warn(std::string_view msg) { log(spdlog::level::warn, msg); }
        void error(std::string_view msg) { log(spdlog::level::err, msg); }


        template <typename... Args>
        void trace(fmt::format_string<Args...> fmt, Args&&... args) {
            log(spdlog::level::trace, fmt::format(fmt, std::forward<Args>(args)...));
        }

        template <typename... Args>
        void debug(fmt::format_string<Args...> fmt, Args&&... args) {
            log(spdlog::level::debug, fmt::format(fmt, std::forward<Args>(args)...));
        }

        template <typename... Args>
        void info(fmt::format_string<Args...> fmt, Args&&... args) {
            log(spdlog::level::info, fmt::format(fmt, std::forward<Args>(args)...));
        }

        template <typename... Args>
        void warn(fmt::format_string<Args...> fmt, Args&&... args) {
            log(spdlog::level::warn, fmt::format(fmt, std::forward<Args>(args)...));
        }

        template <typename... Args>
        void error(fmt::format_string<Args...> fmt, Args&&... args) {
            log(spdlog::level::err, fmt::format(fmt, std::forward<Args>(args)...));
        }
    };
}