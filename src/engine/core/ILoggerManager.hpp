#pragma once

#include <memory>
#include <string>

namespace Bajabongo
{
    class ILogger;

    class ILoggerManager
    {
    public:
        virtual ~ILoggerManager() = default;

        virtual std::shared_ptr<ILogger> GetLogger(const std::string& name) = 0;
    };
}