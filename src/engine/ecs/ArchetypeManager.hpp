#pragma once

#include <string>
#include <unordered_map>
#include <nlohmann/json.hpp>
#include "engine/core/ILogger.hpp"
#include "engine/core/ILoggerManager.hpp"

namespace engine {

    class ArchetypeManager {
    public:
        explicit ArchetypeManager(std::shared_ptr<ILoggerManager> logManager = nullptr);

        bool loadArchetypes(const std::string& filepath);
        const nlohmann::json* getArchetype(const std::string& id) const;

    private:
        std::unordered_map<std::string, nlohmann::json> m_archetypes;
        std::shared_ptr<ILogger> m_logger;
    };

} // namespace engine