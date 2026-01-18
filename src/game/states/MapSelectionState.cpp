#include "engine/pch.h"
#include "MapSelectionState.hpp"
#include "engine/core/EngineContext.hpp"
#include "engine/core/ILogger.hpp"
#include "engine/core/ILoggerManager.hpp"
#include "engine/core/IInputManager.hpp"
#include "engine/events/StateEvents.hpp"
#include "engine/rendering/IRenderer.hpp"
#include <SFML/Window/Event.hpp>

namespace game {

    void MapSelectionState::onEnter(engine::EngineContext& context) {
        m_logger = context.m_logManager->GetLogger("MapSelectionState");
        if (m_logger) m_logger->info("Entering MapSelectionState");
    }

    void MapSelectionState::onExit(engine::EngineContext& context) {
        if (m_logger) m_logger->info("Exiting MapSelectionState");
    }

    void MapSelectionState::handleEvent(engine::EngineContext& context, const sf::Event& event) {
        if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
            if (keyPressed->code == sf::Keyboard::Key::Escape) {
                if (m_logger) m_logger->info("Escape pressed. Returning to Gameplay (Hideout).");
                context.currentMission.reset(); // Clear mission config to load default/hideout
                context.m_dispatcher->enqueue<engine::RequestStateSwapEvent>("Gameplay");
            }
        }
        
        if (const auto* mouseButton = event.getIf<sf::Event::MouseButtonPressed>()) {
             if (mouseButton->button == sf::Mouse::Button::Left) {
                  // Simple split screen selection for now
                  // Left side = Warehouse, Right side = Hospital (Example)
                  
                  // Use InputManager to be safe across SFML versions
                  auto mousePos = context.m_inputManager->getMousePosition();

                  // Using fixed values for demonstration. In real implementation, check against UI rects.
                  if (mousePos.x < 640) { // Left half
                      if (m_logger) m_logger->info("Selected Warehouse");
                      context.currentMission = engine::EngineContext::MissionConfig{
                          .mapFile = "assets/data/warehouse.json",
                          .displayName = "Warehouse"
                      };
                      // Swap current state (MapSelection) with Gameplay, loading the new mission
                      context.m_dispatcher->enqueue<engine::RequestStateSwapEvent>("Gameplay");
                  } else { // Right half
                      if (m_logger) m_logger->info("Selected Hospital");
                      context.currentMission = engine::EngineContext::MissionConfig{
                          .mapFile = "assets/data/hospital.json",
                          .displayName = "Hospital"
                      };
                      context.m_dispatcher->enqueue<engine::RequestStateSwapEvent>("Gameplay");
                  }
             }
        }
    }

    void MapSelectionState::update(engine::EngineContext& context, float fixedDeltaTime) {
        // Update logic
    }

    void MapSelectionState::render(engine::EngineContext& context) {
        // For now, we rely on the renderer to clear the screen (handled by Application/MainLoop)
        // We will just draw a debug rect or rely on the clear color for now.
        // If we had a "MapSelection" sprite, we would draw it here.
        
        // TODO: Draw UI for map selection
    }

} // namespace game
