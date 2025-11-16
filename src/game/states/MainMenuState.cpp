#include "MainMenuState.hpp"

#include "engine/core/EngineContext.hpp"
#include "engine/core/StateManager.hpp"
#include "engine/core/ILoggerManager.hpp"
#include "engine/core/ILogger.hpp"

#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>

namespace game {

    void MainMenuState::onEnter(engine::EngineContext& context) {
        context.m_logManager->GetLogger("Game")->info("Entering MainMenuState.");
    }

    void MainMenuState::onExit(engine::EngineContext& context) {
        context.m_logManager->GetLogger("Game")->info("Exiting MainMenuState.");
    }

    void MainMenuState::update(engine::EngineContext& context, float fixedDeltaTime) {
        // Nothing to update in the main menu
    }

    void MainMenuState::render(engine::EngineContext& context) {
        // In a real game, you would draw the main menu UI here.
    }
    void MainMenuState::handleEvent(engine::EngineContext& context, const sf::Event& event)
    {
        // SFML 3 style: use is<> / getIf<> on sf::Event
        if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>())
        {
            // Check Enter via the Key enum (not sf::Keyboard::Enter)
            if (keyPressed->code == sf::Keyboard::Key::Enter)
            {
                context.m_logManager
                    ->GetLogger("Game")
                    ->info("Enter key pressed, requesting state swap to 'Gameplay'.");

                context.m_stateManager->requestSwap("Gameplay");
            }
        }
    }

} // namespace game
