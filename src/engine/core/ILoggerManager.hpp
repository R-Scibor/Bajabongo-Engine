#pragma once

#include <memory>
#include <string>

namespace engine
{
    class ILogger;

    /**
     * @brief Interface for a central Logger Registry/Factory.
     * * Manages the lifecycle of named loggers (e.g., "Physics", "Audio", "Core").
     */
    class ILoggerManager
    {
    public:
        virtual ~ILoggerManager() = default;

        /**
         * @brief Retrieves (or creates) a logger with a specific name.
         * * Using named loggers allows filtering logs by subsystem in the console or files.
         * @param name The name of the subsystem (e.g., "Physics").
         * @return Shared pointer to the requested logger instance.
         */
        virtual std::shared_ptr<ILogger> GetLogger(const std::string& name) = 0;
    };
}