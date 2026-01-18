#include "engine/pch.h"
#include "MapSelectionState.hpp"
#include "game/components/MapMarkerComponent.hpp"
#include "game/components/VisibleToPlayerComponent.hpp"
#include "engine/components/TransformComponent.hpp"
#include "engine/components/RenderableComponent.hpp"
#include "engine/core/EngineContext.hpp"
#include "engine/core/ILogger.hpp"
#include "engine/core/ILoggerManager.hpp"
#include "engine/core/IInputManager.hpp"
#include "engine/events/StateEvents.hpp"
#include "engine/rendering/IRenderer.hpp"
#include "engine/rendering/SFMLRenderer.hpp"
#include "engine/rendering/RenderSystem.hpp"
#include "engine/rendering/Sprite.hpp" 
#include <SFML/Window/Event.hpp>
#include <cmath>

namespace game {

    MapSelectionState::MapSelectionState(engine::EngineContext& context)
        : m_renderSystem(std::make_unique<engine::RenderSystem>(context))
    {
    }

    void MapSelectionState::onEnter(engine::EngineContext& context) {
        m_logger = context.m_logManager->GetLogger("MapSelectionState");
        if (m_logger) m_logger->info("Entering MapSelectionState");

        // Reset View to Default (Screen Space) to ensure UI is visible
        if (auto renderer = std::dynamic_pointer_cast<engine::SFMLRenderer>(context.m_renderer)) {
            auto& window = renderer->getNativeRenderWindow();
            window.setView(window.getDefaultView());
        }

        // Clear any previous entities (e.g. from GameplayState if swapped)
        context.m_registry->clear();

        auto& registry = *context.m_registry;

        // --- Spawn Background ---
        auto bgEntity = registry.create();
        registry.emplace<engine::TransformComponent>(bgEntity, engine::Vector2f{0.f, 0.f});
        // Background at layer 0
        registry.emplace<engine::RenderableComponent>(bgEntity, "map_selection_screen_sprite", 0);
        registry.emplace<game::VisibleToPlayerComponent>(bgEntity);

        // --- Spawn Markers ---
        // Helper lambda
        auto spawnMarker = [&](const std::string& mapFile, const std::string& name, 
                               const std::string& markerSprite, const std::string& textSprite, 
                               engine::Vector2f pos) 
        {
            auto marker = registry.create();
            // Scale marker x3
            registry.emplace<engine::TransformComponent>(marker, pos, 0.0f, engine::Vector2f{3.0f, 3.0f});
            // Marker at layer 1
            registry.emplace<engine::RenderableComponent>(marker, markerSprite, 1);
            registry.emplace<game::VisibleToPlayerComponent>(marker);
            
            // Create Text Entity (hidden by default)
            auto textEnt = registry.create();
            // Position text relative to marker (e.g., slightly above or overlapping as requested)
            // Scale x3 and move up slightly to appear "over" (above) the icon
            registry.emplace<engine::TransformComponent>(textEnt, engine::Vector2f{pos.x, pos.y - 60.0f}, 0.0f, engine::Vector2f{3.0f, 3.0f});
            // Text at layer 2 (on top of marker)
            auto& renderable = registry.emplace<engine::RenderableComponent>(textEnt, textSprite, 2);
            renderable.color.a = 0; // Hidden initially
            registry.emplace<game::VisibleToPlayerComponent>(textEnt);

            registry.emplace<MapMarkerComponent>(marker, mapFile, name, textEnt, pos.y);
        };

        // Positions (Guessed based on FHD 1920x1080)
        // Note: MapLoader uses std::ifstream, so paths are relative to the executable (e.g. build/bin).
        spawnMarker("../../assets/data/map_hideout.json", "Hideout", "icon_hideout_sprite", "icon_hideout_text_sprite", {1380.f, 600.f});
        spawnMarker("../../assets/data/warehouse.json", "Warehouse", "icon_warehouse_sprite", "icon_warehouse_text_sprite", {450.f, 570.f});
        spawnMarker("../../assets/data/hospital.json", "Hospital", "icon_hospital_sprite", "icon_hospital_text_sprite", {960.f, 540.f});
    }

    void MapSelectionState::onExit(engine::EngineContext& context) {
        if (m_logger) m_logger->info("Exiting MapSelectionState");
        context.m_registry->clear();
    }

    void MapSelectionState::handleEvent(engine::EngineContext& context, const sf::Event& event) {
        if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
            if (keyPressed->code == sf::Keyboard::Key::Escape) {
                if (m_logger) m_logger->info("Escape pressed. Returning to Gameplay (Hideout).");
                context.currentMission.reset();
                context.m_dispatcher->enqueue<engine::RequestStateSwapEvent>("Gameplay");
            }
        }
        
        if (const auto* mouseButton = event.getIf<sf::Event::MouseButtonPressed>()) {
             if (mouseButton->button == sf::Mouse::Button::Left) {
                  // Hit test logic
                  auto view = context.m_registry->view<engine::TransformComponent, MapMarkerComponent>();
                  
                  for (auto [entity, transform, marker] : view.each()) {
                      if (marker.isHovered) { // Re-use hover state for click detection
                          if (m_logger) m_logger->info("Selected {}", marker.mapDisplayName);
                          context.currentMission = engine::EngineContext::MissionConfig{
                              .mapFile = marker.mapConfigPath,
                              .displayName = marker.mapDisplayName
                          };
                          context.m_dispatcher->enqueue<engine::RequestStateSwapEvent>("Gameplay");
                          break;
                      }
                  }
             }
        }
    }

    void MapSelectionState::update(engine::EngineContext& context, float fixedDeltaTime) {
        m_time += fixedDeltaTime;
        
        auto mousePos = context.m_inputManager->getMousePosition();
        engine::Vector2f mouseWorldPos = { static_cast<float>(mousePos.x), static_cast<float>(mousePos.y) };
        // Assuming view is default 1:1 with window for this screen.

        auto view = context.m_registry->view<engine::TransformComponent, MapMarkerComponent, engine::RenderableComponent>();
        
        for (auto [entity, transform, marker, renderable] : view.each()) {
            // Get Sprite Bounds
            engine::Vector2f size = {32.f, 32.f}; // Default
            if (context.m_spriteManager) {
                const auto* sprite = context.m_spriteManager->getSprite(renderable.spriteId);
                if (sprite) {
                    size.x = static_cast<float>(sprite->uvRect.size.x);
                    size.y = static_cast<float>(sprite->uvRect.size.y);
                }
            }

            // Hit Test (AABB centered at position)
            float halfW = (size.x * transform.scale.x) / 2.0f;
            float halfH = (size.y * transform.scale.y) / 2.0f;
            
            bool hover = (mouseWorldPos.x >= transform.position.x - halfW &&
                          mouseWorldPos.x <= transform.position.x + halfW &&
                          mouseWorldPos.y >= transform.position.y - halfH &&
                          mouseWorldPos.y <= transform.position.y + halfH);
            
            marker.isHovered = hover;

            // Animation & Text Visibility
            if (hover) {
                // Move up/down
                float offset = std::sin(m_time * 5.0f) * 5.0f;
                transform.position.y = marker.originalY + offset;
                
                // Show text
                if (context.m_registry->valid(marker.textEntity)) {
                    auto& textRender = context.m_registry->get<engine::RenderableComponent>(marker.textEntity);
                    textRender.color.a = 255;
                    // Sync text position with marker
                    auto& textTrans = context.m_registry->get<engine::TransformComponent>(marker.textEntity);
                    textTrans.position.y = transform.position.y - 60.0f; // Follow marker Y with offset
                }
            } else {
                // Reset
                transform.position.y = marker.originalY;
                
                // Hide text
                if (context.m_registry->valid(marker.textEntity)) {
                    auto& textRender = context.m_registry->get<engine::RenderableComponent>(marker.textEntity);
                    textRender.color.a = 0;
                    auto& textTrans = context.m_registry->get<engine::TransformComponent>(marker.textEntity);
                    textTrans.position.y = marker.originalY - 60.0f; // Reset position with offset
                }
            }
        }
    }

    void MapSelectionState::render(engine::EngineContext& context) {
        if (m_renderSystem && context.m_renderer) {
            auto sfmlRenderer = std::dynamic_pointer_cast<engine::SFMLRenderer>(context.m_renderer);
            if (sfmlRenderer) {
                m_renderSystem->update(sfmlRenderer->getNativeRenderWindow());
            }
        }
    }

} // namespace game
