#pragma once

#include <string>
#include <unordered_map>
#include <nlohmann/json.hpp>
#include "engine/core/ILogger.hpp"
#include "engine/core/ILoggerManager.hpp"

namespace engine {

    /**
     * @class ArchetypeManager
     * @brief Manages the loading and retrieval of entity archetypes from JSON data.
     *
     * The ArchetypeManager is responsible for parsing JSON files containing entity archetypes
     * and providing access to these archetypes for entity creation. It uses an internal
     * map to store the archetypes keyed by their unique identifiers.
     */
    class ArchetypeManager {
    public:
        /**
         * @brief Constructs a new ArchetypeManager.
         *
         * @param logManager Shared pointer to the logger manager for logging operations.
         */
        explicit ArchetypeManager(std::shared_ptr<ILoggerManager> logManager = nullptr);

        /**
         * @brief Loads archetypes from a specified JSON file.
         *
         * Parses the file and populates the internal archetype map.
         *
         * @param filepath The path to the JSON file containing archetype definitions.
         * @return true If the file was successfully opened and parsed.
         * @return false If the file could not be opened or contained invalid JSON.
         */
        bool loadArchetypes(const std::string& filepath);

        /**
         * @brief Retrieves a pointer to the JSON data for a specific archetype.
         *
         * @param id The unique identifier of the archetype.
         * @return const nlohmann::json* Pointer to the archetype JSON object, or nullptr if not found.
         */
        const nlohmann::json* getArchetype(const std::string& id) const;

        /**
         * @brief Retrieves all loaded archetypes.
         *
         * @return const std::unordered_map<std::string, nlohmann::json>& Reference to the map of all archetypes.
         */
        const std::unordered_map<std::string, nlohmann::json>& getAllArchetypes() const {
            return m_archetypes;
        }

    private:
        /** @brief Map storing archetypes keyed by their ID. */
        std::unordered_map<std::string, nlohmann::json> m_archetypes;
        
        /** @brief Logger instance for the ArchetypeManager. */
        std::shared_ptr<ILogger> m_logger;
    };

} // namespace engine