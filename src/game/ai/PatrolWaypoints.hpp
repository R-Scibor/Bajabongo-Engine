#pragma once

#include "engine/core/math/MathAliases.hpp"
#include "game/components/PatrolBehaviorComponent.hpp"

#include <nlohmann/json.hpp>
#include <exception>
#include <string>
#include <vector>

namespace game {

    struct PatrolWaypointSpec {
        std::vector<engine::Vector2f> raw;
        bool absolute = false;
        float wanderRadius = 150.0f;
    };

    /// Parse [[x, y], ...] or a string holding that JSON, or "x,y;x,y".
    inline bool parseWaypointList(const nlohmann::json& value, std::vector<engine::Vector2f>& out) {
        nlohmann::json array = value;
        if (value.is_string()) {
            const std::string text = value.get<std::string>();
            if (text.empty()) return false;
            try {
                array = nlohmann::json::parse(text);
            } catch (const nlohmann::json::exception&) {
                array = nlohmann::json::array();
                // "x,y;x,y"
                std::size_t start = 0;
                while (start < text.size()) {
                    const std::size_t sep = text.find(';', start);
                    const std::string token =
                        text.substr(start, sep == std::string::npos ? std::string::npos : sep - start);
                    const std::size_t comma = token.find(',');
                    if (comma != std::string::npos) {
                        try {
                            const float x = std::stof(token.substr(0, comma));
                            const float y = std::stof(token.substr(comma + 1));
                            array.push_back(nlohmann::json::array({x, y}));
                        } catch (const std::exception&) {
                            return false;
                        }
                    }
                    if (sep == std::string::npos) break;
                    start = sep + 1;
                }
            }
        }

        if (!array.is_array() || array.empty()) return false;

        std::vector<engine::Vector2f> parsed;
        parsed.reserve(array.size());
        for (const auto& wp : array) {
            if (wp.is_array() && wp.size() >= 2 && wp[0].is_number() && wp[1].is_number()) {
                parsed.emplace_back(wp[0].get<float>(), wp[1].get<float>());
            } else if (wp.is_object() && wp.contains("x") && wp.contains("y")) {
                parsed.emplace_back(wp["x"].get<float>(), wp["y"].get<float>());
            } else {
                return false;
            }
        }
        if (parsed.empty()) return false;
        out = std::move(parsed);
        return true;
    }

    /// Archetype waypoints are spawn offsets. Absolute waypoints are map-pixel positions.
    /// Both are multiplied by mapScale. An empty list becomes a square around the spawn.
    inline std::vector<engine::Vector2f> resolvePatrolWaypoints(const engine::Vector2f& spawn, float mapScale,
                                                                const PatrolWaypointSpec& spec) {
        const float scale = (mapScale > 0.0f) ? mapScale : 1.0f;
        std::vector<engine::Vector2f> world;
        world.reserve(spec.raw.size());
        for (const auto& raw : spec.raw) {
            if (spec.absolute) {
                world.emplace_back(raw.x * scale, raw.y * scale);
            } else {
                world.emplace_back(spawn.x + raw.x * scale, spawn.y + raw.y * scale);
            }
        }
        if (world.empty()) {
            const float radius = spec.wanderRadius * scale;
            world = {{spawn.x - radius, spawn.y - radius},
                     {spawn.x + radius, spawn.y - radius},
                     {spawn.x + radius, spawn.y + radius},
                     {spawn.x - radius, spawn.y + radius}};
        }
        return world;
    }

    /// Read a Tiled "waypoints" property from an entity object (array or string).
    inline bool tryReadMapWaypoints(const nlohmann::json& tiledObject, std::vector<engine::Vector2f>& out) {
        if (!tiledObject.is_object()) return false;

        auto tryValue = [&](const nlohmann::json& value) { return parseWaypointList(value, out); };

        if (tiledObject.contains("waypoints")) {
            if (tryValue(tiledObject["waypoints"])) return true;
        }
        if (tiledObject.contains("properties") && tiledObject["properties"].is_array()) {
            for (const auto& prop : tiledObject["properties"]) {
                if (!prop.is_object()) continue;
                const std::string name = prop.value("name", "");
                if (name != "waypoints") continue;
                if (prop.contains("value") && tryValue(prop["value"])) return true;
            }
        }
        return false;
    }

    /// Abandon the current target and go to the next point. False if the route has
    /// fewer than two waypoints.
    inline bool skipToNextPatrolWaypoint(PatrolBehaviorComponent& patrol) {
        const int count = static_cast<int>(patrol.waypoints.size());
        if (count <= 1) return false;
        if (patrol.currentWaypointIndex < 0 || patrol.currentWaypointIndex >= count) {
            patrol.currentWaypointIndex = 0;
        }

        if (patrol.loopMode) {
            patrol.currentWaypointIndex = (patrol.currentWaypointIndex + 1) % count;
            return true;
        }

        if (patrol.reversing) {
            if (patrol.currentWaypointIndex <= 0) {
                patrol.reversing = false;
                patrol.currentWaypointIndex = 1;
            } else {
                patrol.currentWaypointIndex--;
                if (patrol.currentWaypointIndex == 0) patrol.reversing = false;
            }
        } else {
            if (patrol.currentWaypointIndex >= count - 1) {
                patrol.reversing = true;
                patrol.currentWaypointIndex = count - 2;
            } else {
                patrol.currentWaypointIndex++;
                if (patrol.currentWaypointIndex == count - 1) patrol.reversing = true;
            }
        }
        return true;
    }

} // namespace game
