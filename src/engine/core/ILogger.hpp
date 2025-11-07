#pragma once

#include <string_view>

namespace Bajabongo
{
    class ILogger
    {
    public:
        virtual ~ILogger() = default;

        virtual void trace(std::string_view message) = 0;
        virtual void debug(std::string_view message) = 0;
        virtual void info(std::string_view message) = 0;
        virtual void warn(std::string_view message) = 0;
        virtual void error(std::string_view message) = 0;
    };
}