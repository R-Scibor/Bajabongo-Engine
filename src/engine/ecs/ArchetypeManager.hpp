#pragma once

#include <string>
#include <unordered_map>
#include <nlohmann/json.hpp>
#include "engine/core/ILogger.hpp"

namespace engine {

    class ArchetypeManager {
    public:
        ArchetypeManager() = default;

        bool loadArchetypes(const std::string& filepath);
        const nlohmann::json* getArchetype(const std::string& id) const;

        // Set logger for error reporting
        void setLogger(std::shared_ptr<ILogger> logger) { m_logger = logger; }

    private:
        std::unordered_map<std::string, nlohmann::json> m_archetypes;
        std::shared_ptr<ILogger> m_logger;
    };

} // namespace engine