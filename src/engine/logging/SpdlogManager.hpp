#pragma once

#include "engine/core/ILoggerManager.hpp"
#include <string>
#include <unordered_map>
#include <memory>

// Forward declaration
namespace spdlog
{
    class logger;
}

namespace engine
{
    /**
     * @brief Concrete implementation of ILoggerManager using spdlog.
     *
     * Responsible for:
     * 1. Lifecycle management of spdlog instances.
     * 2. Loading logging configuration from a file (setting levels per subsystem).
     * 3. Creating new loggers on demand (Lazy Initialization).
     */
    class SpdlogManager : public ILoggerManager
    {
    public:
        /**
         * @brief Initializes the manager and loads the configuration.
         * @param configFilePath Path to the text file containing logger settings (e.g., "logger.ini").
         */
        explicit SpdlogManager(const std::string& configFilePath);
        
        /**
         * @brief Cleans up spdlog resources (flushes logs, shuts down thread pool).
         */
        ~SpdlogManager() override;

        /**
         * @brief Retrieves or creates a named logger.
         * * If a logger with the given name exists, it is returned.
         * * If not, a new logger is created using default settings (stdout color sink).
         * @param name The subsystem name (e.g., "Physics", "AI").
         * @return Shared pointer to the ILogger interface.
         */
        std::shared_ptr<ILogger> GetLogger(const std::string& name) override;

    private:
        /**
         * @brief Parses the configuration file to set initial log levels.
         * * Expected format: `LoggerName=Level` (e.g., `Physics=Debug`).
         */
        void LoadConfiguration(const std::string& configFilePath);

        /// @brief Cache of active spdlog instances, indexed by name.
        std::unordered_map<std::string, std::shared_ptr<spdlog::logger>> m_loggers;
        
        /// @brief Standard formatting pattern for log messages.
        std::string m_defaultPattern;
    };
}