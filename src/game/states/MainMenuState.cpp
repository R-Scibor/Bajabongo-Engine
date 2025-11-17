#include "MainMenuState.hpp"

#include "engine/core/EngineContext.hpp"
#include "engine/core/ILogger.hpp"
#include "engine/events/StateEvents.hpp"

#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <entt/entt.hpp>

namespace game {

    void MainMenuState::onEnter(engine::EngineContext& context) {
        if (m_logger) {
            m_logger->info("Entering MainMenuState.");
        }
    }

    void MainMenuState::onExit(engine::EngineContext& context) {
        if (m_logger) {
            m_logger->info("Exiting MainMenuState.");
        }
    }

    void MainMenuState::update(engine::EngineContext& context, float fixedDeltaTime) {
        // Nothing to update in the main menu
    }

    void MainMenuState::render(engine::EngineContext& context) {
        // In a real game, you would draw the main menu UI here.
    }
    void MainMenuState::handleEvent(engine::EngineContext& context, const sf::Event& event)
    {
        if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
            if (keyPressed->code == sf::Keyboard::Key::Enter) {
                if (m_logger) {
                    m_logger->info("Enter pressed. Enqueuing RequestStatePushEvent for 'Gameplay'.");
                }
                context.m_dispatcher->enqueue<engine::RequestStatePushEvent>("Gameplay");
            }
            else if (keyPressed->code == sf::Keyboard::Key::Escape) {
                if (m_logger) {
                    m_logger->info("Escape pressed. Enqueuing RequestStateClearEvent to quit.");
                }
                context.m_dispatcher->enqueue<engine::RequestStateClearEvent>();
            }
        }
    }

} // namespace game
