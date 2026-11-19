#include "SpdlogManager.hpp"
#include "SpdlogLogger.hpp"
#include "LogConfig.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <chrono>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <string>

namespace engine {
    namespace {
        /// Relative to the working directory (out/game/<Config> when run from the build tree).
        constexpr const char* LogDirectory = "logs";
        constexpr const char* LogFilePath = "logs/game.log";
    } // namespace

    // --- SpdlogManager Implementation ---

    SpdlogManager::SpdlogManager(const std::string& configFilePath)
        // Format: [Time Zone] [LoggerName] [Level] [ThreadID] Message
        : m_defaultPattern("[%H:%M:%S %z] [%n] [%^%l%$] [thread %t] %v") {
        // Set the global pattern for all new loggers
        spdlog::set_pattern(m_defaultPattern);

        m_sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
        try {
            std::filesystem::create_directories(LogDirectory);
            // Truncate on every start so the file holds only the latest run.
            m_sinks.push_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(LogFilePath, true));
        } catch (const std::exception& e) {
            // The game still runs with console logging if the file cannot be opened.
            std::cerr << "Warning: Could not open log file " << LogFilePath << ": " << e.what() << std::endl;
        }

        loadConfiguration(configFilePath);

        // Info and debug lines reach the file within a second even without a warning.
        spdlog::flush_every(std::chrono::seconds(1));
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
        auto new_logger = createLogger(name);
        new_logger->set_level(spdlog::level::info); // Default level is Info

        // 3. Cache it for future use
        m_loggers[name] = new_logger;

        return std::make_shared<SpdlogLogger>(new_logger);
    }

    std::shared_ptr<spdlog::logger> SpdlogManager::createLogger(const std::string& name) {
        auto logger = std::make_shared<spdlog::logger>(name, m_sinks.begin(), m_sinks.end());
        // Applies the global pattern and registers the name, like the spdlog factory helpers.
        spdlog::initialize_logger(logger);
        // Flush warnings and errors at once so a crash still leaves them in logs/game.log.
        logger->flush_on(spdlog::level::warn);
        return logger;
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
                logger = createLogger(name);
            }

            // Set the level as defined in the file
            logger->set_level(levelFromString(level_str));

            // Store in our map so getLogger returns this pre-configured instance
            m_loggers[name] = logger;
        }
    }
} // namespace engine
