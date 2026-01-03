#pragma once

#include <string>
#include <memory>
#include <nlohmann/json.hpp>

namespace engine {
    struct EngineContext;
    class ILogger;

    class MapLoader {
    public:
        explicit MapLoader(EngineContext& context);
        ~MapLoader() = default;

        bool load(const std::string& filepath);

    private:
        void processImageLayer(const nlohmann::json& layer);
        void processObjectLayer(const nlohmann::json& layer, bool isHalfCollision);
        void createCollisionBody(const nlohmann::json& object, bool isHalfCollision);

        EngineContext& m_context;
        std::shared_ptr<ILogger> m_logger;
    };
}
