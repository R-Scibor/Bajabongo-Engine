#include "SpdlogManager.hpp"
#include "SpdlogLogger.hpp"
#include "LogConfig.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <iostream>
#include <fstream>
#include <string>

namespace engine {
    // --- SpdlogManager Implementation ---

    SpdlogManager::SpdlogManager(const std::string& configFilePath)
        // Format: [Time Zone] [LoggerName] [Level] [ThreadID] Message
        : m_defaultPattern("[%H:%M:%S %z] [%n] [%^%l%$] [thread %t] %v") {
        // Set the global pattern for all new loggers
        spdlog::set_pattern(m_defaultPattern);
        loadConfiguration(configFilePath);
    }

    SpdlogManager::~SpdlogManager() {
        // Ensure all logs are flushed to disk/console before exiting
        spdlog::shutdown();
    }

    std::shared_ptr<ILogger> SpdlogManager::getLogger(const std::string& name) {
        // 1. Check if the logger already exists in our cache
        auto it = m_loggers.find(name);
        if (it != m_loggers.end()) {
            return std::make_shared<SpdlogLogger>(it->second);
        }

        // 2. If not found, create a new one with default settings
        // 'stdout_color_mt' creates a multi-threaded safe console logger with colors.
        auto new_logger = spdlog::stdout_color_mt(name);
        new_logger->set_level(spdlog::level::info); // Default level is Info

        // 3. Cache it for future use
        m_loggers[name] = new_logger;

        return std::make_shared<SpdlogLogger>(new_logger);
    }

    void SpdlogManager::loadConfiguration(const std::string& configFilePath) {
        std::ifstream configFile(configFilePath);
        if (!configFile.is_open()) {
            // Use std::cerr because the logging system itself isn't fully configured yet
            std::cerr << "Warning: Could not open logging config file: " << configFilePath
                      << ". Using default settings." << std::endl;
            return;
        }

        std::string line;
        while (std::getline(configFile, line)) {
            std::string name;
            std::string level_str;
            if (!parseLogConfigLine(line, name, level_str)) continue;

            // Check if spdlog already has this logger (e.g. created globally)
            auto logger = spdlog::get(name);
            if (!logger) {
                logger = spdlog::stdout_color_mt(name);
            }

            // Set the level as defined in the file
            logger->set_level(levelFromString(level_str));

            // Store in our map so getLogger returns this pre-configured instance
            m_loggers[name] = logger;
        }
    }
} // namespace engine
