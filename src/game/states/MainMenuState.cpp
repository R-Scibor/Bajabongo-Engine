#include "MainMenuState.hpp"

#include <cstdlib>
#include "engine/core/EngineContext.hpp"
#include "engine/core/ILogger.hpp"
#include "engine/events/StateEvents.hpp"
#include "engine/events/AudioEvents.hpp"
#include "engine/rendering/IRenderer.hpp"
#include "engine/core/IResourceManager.hpp"

#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <entt/entt.hpp>

namespace game {

    void MainMenuState::onEnter(engine::EngineContext& context) {
        if (m_logger) {
            m_logger->info("Entering MainMenuState.");
        }
        m_menuItems = {"START", "LOAD", "CONFIG", "QUIT"};
        m_selectedItemIndex = 0;
        m_timeInState = 0.0f;

        if (context.m_resourceManager) {
            // Path adjusted for execution from build/bin directory (assumed depth 2)
            context.m_resourceManager->loadTexture("main_menu_bg", "../../assets/textures/main_menu_bg.png");
        }

        // Play Main Menu Music
        context.m_dispatcher->enqueue<engine::PlayMusicEvent>({
            .id = "main_menu",
            .volume = 30.0f,
            .loop = true,
            .crossfadeDuration = 2.0f
        });
    }

    void MainMenuState::onExit(engine::EngineContext& context) {
        if (m_logger) {
            m_logger->info("Exiting MainMenuState.");
        }
    }

    void MainMenuState::update(engine::EngineContext& context, float fixedDeltaTime) {
        m_timeInState += fixedDeltaTime;
    }

    void MainMenuState::render(engine::EngineContext& context) {
        auto& renderer = *context.m_renderer;

        // Draw Background
        if (context.m_resourceManager) {
            auto bg = context.m_resourceManager->getTexture("main_menu_bg");
            if (bg) {
                // Scale background to cover the screen (1920x1080)
                renderer.drawTexture(bg.get(), {0.0f, 0.0f}, {1920.0f, 1080.0f});
            }
        }

        // Strip constants
        const float stripX = 1420.0f;
        const float stripW = 300.0f;
        const float screenH = 1080.0f;

        // Draw Strip Background (Dark Slate #151515, 85% opacity)
        renderer.drawRect({stripX, 0.0f}, {stripW, screenH}, {21, 21, 21, 217});

        // Draw Strip Edge (White, 30% opacity)
        renderer.drawRect({stripX, 0.0f}, {1.0f, screenH}, {255, 255, 255, 76});

        // Tech Fluff (Decorative elements)
        engine::Color fluffColor = {255, 255, 255, 100};
        uint32_t smallFont = 14;

        renderer.drawText("[SYS_READY]", {stripX + 20.0f, 20.0f}, smallFont, fluffColor);
        renderer.drawText("// ORBITAL_LINK: CONNECTED", {stripX + 20.0f, screenH - 40.0f}, smallFont, fluffColor);
        renderer.drawText("v.0.3.14", {stripX + stripW - 80.0f, screenH - 40.0f}, smallFont, fluffColor);

        // Corner Decorations
        renderer.drawText("+", {stripX + 5.0f, 5.0f}, smallFont, fluffColor);
        renderer.drawText("+", {stripX + stripW - 15.0f, 5.0f}, smallFont, fluffColor);
        renderer.drawText("+", {stripX + 5.0f, screenH - 25.0f}, smallFont, fluffColor);
        renderer.drawText("+", {stripX + stripW - 15.0f, screenH - 25.0f}, smallFont, fluffColor);

        // Anchor Line Constants
        const float anchorX = stripX + 40.0f; // 1460
        const float textX = anchorX + 40.0f;  // 1500
        const float logoY = 100.0f;
        const float startY = 400.0f;
        const float gap = 60.0f;
        const float endY = startY + (m_menuItems.size() * gap);

        // Draw Anchor Line (running from Logo top to bottom of buttons)
        // Animate height growing over 1.5s
        float fullHeight = endY - logoY;
        float progress = std::min(m_timeInState / 1.5f, 1.0f);
        float currentHeight = fullHeight * progress;

        renderer.drawRect({anchorX, logoY}, {1.0f, currentHeight}, {255, 255, 255, 76});

        // Connect Anchor Line to Title
        float titleCenterY = logoY + 24.0f; // Approx center of 48px font
        renderer.drawRect({anchorX, titleCenterY}, {textX - anchorX - 5.0f, 2.0f}, {255, 255, 255, 76});

        // Draw Logo (Right of Anchor Line)
        renderer.drawText("MERIDIAN", {textX, logoY}, 48, {255, 255, 255, 255});

        // Draw Buttons
        for (size_t i = 0; i < m_menuItems.size(); ++i) {
            engine::Color color = {224, 224, 224, 255}; // #E0E0E0
            std::string label = m_menuItems[i];
            float itemY = startY + (i * gap);

            // "Draw Down" reveal: Only draw if the line has passed this item
            if (logoY + currentHeight < itemY) continue;

            uint32_t fontSize = 32;
            bool isBroken = (label == "LOAD" || label == "CONFIG");
            float drawX = textX;
            float drawY = itemY;

            if (isBroken) {
                color = {100, 100, 100, 100}; // Dark Grey, low opacity
                // Draw Tag
                std::string tag = (label == "LOAD") ? "[ERR_NO_DISK]" : "[ERR_ACCESS]";
                renderer.drawText(tag, {textX + 150.0f, itemY + 10.0f}, 14, {255, 50, 50, 255});
            }

            if (static_cast<int>(i) == m_selectedItemIndex) {
                if (isBroken) {
                    color = {255, 0, 0, 255}; // Critical Red
                    
                    // Shake Effect
                    float shakeX = (rand() % 5) - 2.0f;
                    float shakeY = (rand() % 5) - 2.0f;
                    drawX += shakeX;
                    drawY += shakeY;

                    // Draw Background Block (Red tint)
                    renderer.drawRect({textX - 10.0f, itemY}, {230.0f, 40.0f}, {255, 0, 0, 15});

                    // Draw Horizontal Dash (Red)
                    float dashY = itemY + 18.0f;
                    renderer.drawRect({anchorX, dashY}, {textX - anchorX - 5.0f, 2.0f}, color);
                } else {
                    color = {255, 150, 79, 255}; // Safety Orange #FF964F
                    fontSize = 34; // Scale up ~105%

                    // Draw Background Block (faint highlight)
                    renderer.drawRect({textX - 10.0f, itemY}, {230.0f, 40.0f}, {255, 255, 255, 15});
                    
                    // Draw Horizontal Dash connecting Anchor to Text
                    float dashY = itemY + 18.0f;
                    renderer.drawRect({anchorX, dashY}, {textX - anchorX - 5.0f, 2.0f}, color);

                    // Ghost Effect (Cyan copy, slightly offset)
                    renderer.drawText(label, {textX + 3.0f, itemY}, fontSize, {0, 255, 255, 76});
                }
            }

            renderer.drawText(label, {drawX, drawY}, fontSize, color);
        }
    }

    void MainMenuState::handleEvent(engine::EngineContext& context, const sf::Event& event)
    {
        bool activate = false;

        if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
            if (keyPressed->code == sf::Keyboard::Key::Up || keyPressed->code == sf::Keyboard::Key::W) {
                m_selectedItemIndex--;
                if (m_selectedItemIndex < 0) m_selectedItemIndex = static_cast<int>(m_menuItems.size()) - 1;
            }
            else if (keyPressed->code == sf::Keyboard::Key::Down || keyPressed->code == sf::Keyboard::Key::S) {
                m_selectedItemIndex++;
                if (m_selectedItemIndex >= static_cast<int>(m_menuItems.size())) m_selectedItemIndex = 0;
            }
            else if (keyPressed->code == sf::Keyboard::Key::Enter || keyPressed->code == sf::Keyboard::Key::Space) {
                activate = true;
            }
            else if (keyPressed->code == sf::Keyboard::Key::Escape) {
                 context.m_dispatcher->enqueue<engine::RequestStateClearEvent>();
            }
        }
        else if (const auto* mouseMoved = event.getIf<sf::Event::MouseMoved>()) {
            int hit = getItemAt(static_cast<float>(mouseMoved->position.x), static_cast<float>(mouseMoved->position.y));
            if (hit != -1) {
                m_selectedItemIndex = hit;
            }
        }
        else if (const auto* mouseBtn = event.getIf<sf::Event::MouseButtonPressed>()) {
            if (mouseBtn->button == sf::Mouse::Button::Left) {
                int hit = getItemAt(static_cast<float>(mouseBtn->position.x), static_cast<float>(mouseBtn->position.y));
                if (hit != -1) {
                    m_selectedItemIndex = hit;
                    activate = true;
                }
            }
        }

        if (activate && m_selectedItemIndex >= 0 && m_selectedItemIndex < static_cast<int>(m_menuItems.size())) {
            const auto& item = m_menuItems[m_selectedItemIndex];
            if (item == "START") {
                if (m_logger) m_logger->info("Starting Game...");
                context.m_dispatcher->enqueue<engine::RequestStatePushEvent>("Gameplay");
            }
            else if (item == "QUIT") {
                if (m_logger) m_logger->info("Quitting...");
                context.m_dispatcher->enqueue<engine::RequestStateClearEvent>();
            }
        }
    }

    int MainMenuState::getItemAt(float x, float y) const {
        const float stripX = 1420.0f;
        const float anchorX = stripX + 40.0f;
        const float textX = anchorX + 40.0f;
        const float startY = 400.0f;
        const float gap = 60.0f;
        const float logoY = 100.0f;
        const float endY = startY + (m_menuItems.size() * gap);

        // Check animation progress
        float fullHeight = endY - logoY;
        float progress = std::min(m_timeInState / 1.5f, 1.0f);
        float currentHeight = fullHeight * progress;

        for (size_t i = 0; i < m_menuItems.size(); ++i) {
            float itemY = startY + (i * gap);
            
            // Check visibility (Draw Down effect)
            if (logoY + currentHeight < itemY) continue;
            
            // Hitbox: [textX - 10, itemY] to [textX - 10 + 230, itemY + 40]
            float boxX = textX - 10.0f;
            float boxY = itemY;
            float boxW = 230.0f;
            float boxH = 40.0f;

            if (x >= boxX && x <= boxX + boxW && y >= boxY && y <= boxY + boxH) {
                return static_cast<int>(i);
            }
        }
        return -1;
    }

} // namespace game
