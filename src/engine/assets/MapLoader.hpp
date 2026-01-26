/**
 * @file MapLoader.hpp
 * @brief Defines the MapLoader class for loading Tiled maps (JSON format).
 */

#pragma once

#include <string>
#include <memory>
#include <nlohmann/json.hpp>

namespace engine {
    struct EngineContext;
    class ILogger;

    /**
     * @brief Loads game maps from Tiled JSON export files.
     *
     * Handles creating entities, physics bodies, and visual components based on
     * map layers (Image Layers, Object Layers, Entity Layers).
     */
    class MapLoader {
    public:
        /**
         * @brief Constructs the MapLoader.
         * @param context The EngineContext used to access ECS registry, entity factory, etc.
         */
        explicit MapLoader(EngineContext& context);
        
        ~MapLoader() = default;

        /**
         * @brief Loads a map from the specified file path.
         * @param filepath Path to the Tiled JSON file.
         * @return true if loading succeeded, false otherwise.
         */
        bool load(const std::string& filepath);

    private:
        /**
         * @brief Processes an Image Layer to create a background visual.
         * @param layer JSON object representing the layer.
         */
        void processImageLayer(const nlohmann::json& layer);

        /**
         * @brief Processes an Object Layer to create physics bodies (collision/half-collision).
         * @param layer JSON object representing the layer.
         * @param isHalfCollision If true, creates low obstacles; otherwise, full walls.
         */
        void processObjectLayer(const nlohmann::json& layer, bool isHalfCollision);

        /**
         * @brief Processes an Object Layer specifically for spawning game entities.
         * @param layer JSON object representing the layer.
         */
        void processEntityLayer(const nlohmann::json& layer);

        /**
         * @brief Helper to create a physics body from a Tiled object.
         * @param object JSON object defining the rectangle.
         * @param isHalfCollision If true, sets category bits for low obstacles.
         */
        void createCollisionBody(const nlohmann::json& object, bool isHalfCollision);

        EngineContext& m_context;
        std::shared_ptr<ILogger> m_logger;
    };
}
