#include "SpdlogManager.hpp"
#include "SpdlogLogger.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <iostream>
#include <fstream>
#include <string>

namespace engine
{
    // Helper function to trim whitespace from a string
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

    // Helper function to parse level from string
    spdlog::level::level_enum level_from_string(const std::string& level_str)
    {
        std::string lower_level = level_str;
        std::transform(lower_level.begin(), lower_level.end(), lower_level.begin(), ::tolower);

        if (lower_level == "trace") return spdlog::level::trace;
        if (lower_level == "debug") return spdlog::level::debug;
        if (lower_level == "warn") return spdlog::level::warn;
        if (lower_level == "error") return spdlog::level::err;
        if (lower_level == "critical") return spdlog::level::critical;
        // Default to info
        return spdlog::level::info;
    }

    SpdlogManager::SpdlogManager(const std::string& configFilePath)
        : m_defaultPattern("[%H:%M:%S %z] [%n] [%^%l%$] [thread %t] %v")
    {
        spdlog::set_pattern(m_defaultPattern);
        LoadConfiguration(configFilePath);
    }

    SpdlogManager::~SpdlogManager()
    {
        spdlog::shutdown();
    }

    std::shared_ptr<ILogger> SpdlogManager::GetLogger(const std::string& name)
    {
        auto it = m_loggers.find(name);
        if (it != m_loggers.end())
        {
            return std::make_shared<SpdlogLogger>(it->second);
        }

        // If logger not found, create a new one with default settings
        auto new_logger = spdlog::stdout_color_mt(name);
        new_logger->set_level(spdlog::level::info); // Default level
        m_loggers[name] = new_logger;

        return std::make_shared<SpdlogLogger>(new_logger);
    }

    void SpdlogManager::LoadConfiguration(const std::string& configFilePath)
    {
        std::ifstream configFile(configFilePath);
        if (!configFile.is_open())
        {
            std::cerr << "Warning: Could not open logging config file: " << configFilePath << ". Using default settings." << std::endl;
            return;
        }

        std::string line;
        while (std::getline(configFile, line))
        {
            // Skip comments and empty lines
            if (line.empty() || line[0] == '#' || line[0] == ';')
                continue;

            auto delimiterPos = line.find('=');
            if (delimiterPos != std::string::npos)
            {
                std::string name = trim(line.substr(0, delimiterPos));
                std::string level_str = trim(line.substr(delimiterPos + 1));

                auto logger = spdlog::get(name);
                if (!logger)
                {
                    logger = spdlog::stdout_color_mt(name);
                }
                logger->set_level(level_from_string(level_str));
                m_loggers[name] = logger;
            }
        }
    }
}