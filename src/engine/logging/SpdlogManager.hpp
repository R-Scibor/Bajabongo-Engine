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
    class SpdlogManager : public ILoggerManager
    {
    public:
        explicit SpdlogManager(const std::string& configFilePath);
        ~SpdlogManager() override;

        std::shared_ptr<ILogger> GetLogger(const std::string& name) override;

    private:
        void LoadConfiguration(const std::string& configFilePath);

        std::unordered_map<std::string, std::shared_ptr<spdlog::logger>> m_loggers;
        std::string m_defaultPattern;
    };
}