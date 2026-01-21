#pragma once

#include "engine/core/IGameState.hpp"
#include "engine/core/EngineContext.hpp"
#include "engine/core/ILogger.hpp"
#include "engine/core/ILoggerManager.hpp"

#include <memory>
#include <vector>
#include <string>

namespace game {

    /**
     * @brief The state for the main menu.
     *
     * Waits for user input to transition into the main gameplay.
     */
    class MainMenuState : public engine::IGameState {
    public:
        explicit MainMenuState(engine::EngineContext& context)
            : m_logger(context.m_logManager->GetLogger("MainMenu")) {}

        void onEnter(engine::EngineContext& context) override;
        void onExit(engine::EngineContext& context) override;
        void handleEvent(engine::EngineContext& context, const sf::Event& event) override;
        void update(engine::EngineContext& context, float fixedDeltaTime) override;
        void render(engine::EngineContext& context) override;

    private:
        std::shared_ptr<engine::ILogger> m_logger;
        std::vector<std::string> m_menuItems;
        int m_selectedItemIndex = 0;
        float m_timeInState = 0.0f;

        // Returns index of menu item at given screen coordinates, or -1 if none
        int getItemAt(float x, float y) const;
    };

} // namespace game