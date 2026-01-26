#include "SpdlogManager.hpp"
#include "SpdlogLogger.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm> // For std::transform

namespace engine
{
    // --- Helper Functions (Internal linkage) ---

    /**
     * @brief Removes leading and trailing whitespace from a string.
     */
    std::string trim(const std::string& str)
    {
        const std::string whitespace = " \t";
        const auto strBegin = str.find_first_not_of(whitespace);
        if (strBegin == std::string::npos)
            return ""; // no content

        const auto strEnd = str.find_last_not_of(whitespace);
        const auto strRange = strEnd - strBegin + 1;

        return str.substr(strBegin, strRange);
    }

    /**
     * @brief Converts a string representation of a log level to spdlog enum.
     * * Case-insensitive (e.g., "Debug", "debug", "DEBUG").
     */
    spdlog::level::level_enum level_from_string(const std::string& level_str)
    {
        std::string lower_level = level_str;
        std::transform(lower_level.begin(), lower_level.end(), lower_level.begin(), ::tolower);

        if (lower_level == "trace") return spdlog::level::trace;
        if (lower_level == "debug") return spdlog::level::debug;
        if (lower_level == "warn") return spdlog::level::warn;
        if (lower_level == "error") return spdlog::level::err;
        if (lower_level == "critical") return spdlog::level::critical;
        
        // Default fallback
        return spdlog::level::info;
    }

    // --- SpdlogManager Implementation ---

    SpdlogManager::SpdlogManager(const std::string& configFilePath)
        // Format: [Time Zone] [LoggerName] [Level] [ThreadID] Message
        : m_defaultPattern("[%H:%M:%S %z] [%n] [%^%l%$] [thread %t] %v")
    {
        // Set the global pattern for all new loggers
        spdlog::set_pattern(m_defaultPattern);
        LoadConfiguration(configFilePath);
    }

    SpdlogManager::~SpdlogManager()
    {
        // Ensure all logs are flushed to disk/console before exiting
        spdlog::shutdown();
    }

    std::shared_ptr<ILogger> SpdlogManager::GetLogger(const std::string& name)
    {
        // 1. Check if the logger already exists in our cache
        auto it = m_loggers.find(name);
        if (it != m_loggers.end())
        {
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

    void SpdlogManager::LoadConfiguration(const std::string& configFilePath)
    {
        std::ifstream configFile(configFilePath);
        if (!configFile.is_open())
        {
            // Use std::cerr because the logging system itself isn't fully configured yet
            std::cerr << "Warning: Could not open logging config file: " << configFilePath << ". Using default settings." << std::endl;
            return;
        }

        std::string line;
        while (std::getline(configFile, line))
        {
            // Skip comments (# or ;) and empty lines
            if (line.empty() || line[0] == '#' || line[0] == ';')
                continue;

            auto delimiterPos = line.find('=');
            if (delimiterPos != std::string::npos)
            {
                std::string name = trim(line.substr(0, delimiterPos));
                std::string level_str = trim(line.substr(delimiterPos + 1));

                // Check if spdlog already has this logger (e.g. created globally)
                auto logger = spdlog::get(name);
                if (!logger)
                {
                    logger = spdlog::stdout_color_mt(name);
                }
                
                // Set the level as defined in the file
                logger->set_level(level_from_string(level_str));
                
                // Store in our map so GetLogger returns this pre-configured instance
                m_loggers[name] = logger;
            }
        }
    }
}