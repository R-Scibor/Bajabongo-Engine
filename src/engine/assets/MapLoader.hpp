#pragma once

#include <string>
#include <memory>

namespace engine {
    struct EngineContext;
    class ILogger;

    class MapLoader {
    public:
        explicit MapLoader(EngineContext& context);
        ~MapLoader() = default;

        bool load(const std::string& filepath);

    private:
        EngineContext& m_context;
        std::shared_ptr<ILogger> m_logger;
    };
}