#include "engine/pch.h"
#include "ArchetypeManager.hpp"
#include <fstream>

/**
 * @file ArchetypeManager.cpp
 * @brief Implementation of the ArchetypeManager class.
 */

namespace engine {

    ArchetypeManager::ArchetypeManager(std::shared_ptr<ILoggerManager> logManager) {
        if (logManager) {
            m_logger = logManager->GetLogger("ArchetypeManager");
        }
    }

    bool ArchetypeManager::loadArchetypes(const std::string& filepath) {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            if (m_logger) {
                m_logger->error("ArchetypeManager: Failed to open file: {}", filepath);
            }
            return false;
        }

        nlohmann::json root;
        try {
            file >> root;
        } catch (const nlohmann::json::parse_error& e) {
            if (m_logger) {
                m_logger->error("ArchetypeManager: JSON parse error in {}: {}", filepath, e.what());
            }
            return false;
        }

        if (!root.is_object()) {
             if (m_logger) {
                m_logger->error("ArchetypeManager: Root JSON element is not an object in {}", filepath);
            }
            return false;
        }

        for (auto& [key, value] : root.items()) {
            m_archetypes[key] = value;
        }

        if (m_logger) {
            m_logger->info("ArchetypeManager: Loaded {} archetypes from {}", m_archetypes.size(), filepath);
        }

        return true;
    }

    const nlohmann::json* ArchetypeManager::getArchetype(const std::string& id) const {
        auto it = m_archetypes.find(id);
        if (it != m_archetypes.end()) {
            return &it->second;
        }
        return nullptr;
    }

} // namespace engine