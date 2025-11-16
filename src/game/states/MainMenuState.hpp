#pragma once

#include "engine/core/IGameState.hpp"

namespace game {

    /**
     * @brief The state for the main menu.
     *
     * Waits for user input to transition into the main gameplay.
     */
    class MainMenuState : public engine::IGameState {
    public:
        void onEnter(engine::EngineContext& context) override;
        void onExit(engine::EngineContext& context) override;
        void handleEvent(engine::EngineContext& context, const sf::Event& event) override;
        void update(engine::EngineContext& context, float fixedDeltaTime) override;
        void render(engine::EngineContext& context) override;
    };

} // namespace game