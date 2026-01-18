#pragma once
#include "engine/core/IGameState.hpp"
#include <memory>

namespace engine {
    class ILogger;
}

namespace game {

    class MapSelectionState : public engine::IGameState {
    public:
        void onEnter(engine::EngineContext& context) override;
        void onExit(engine::EngineContext& context) override;
        void handleEvent(engine::EngineContext& context, const sf::Event& event) override;
        void update(engine::EngineContext& context, float fixedDeltaTime) override;
        void render(engine::EngineContext& context) override;

    private:
        std::shared_ptr<engine::ILogger> m_logger;
    };

} // namespace game
