#include "SplashScreenState.hpp"
#include "engine/core/EngineContext.hpp"
#include "engine/core/ILoggerManager.hpp"
#include "engine/core/ILogger.hpp"
#include "engine/core/IResourceManager.hpp"
#include "engine/rendering/IRenderer.hpp"
#include "engine/rendering/SFMLRenderer.hpp"
#include "engine/assets/AssetManifestLoader.hpp"
#include "engine/events/StateEvents.hpp"
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <entt/entt.hpp>
#include <cmath>
#include <algorithm> // For std::clamp
#include <cstdint>

namespace game {

SplashScreenState::SplashScreenState(engine::EngineContext& context)
    : m_logger(context.m_logManager->GetLogger("SplashScreen"))
{
}

void SplashScreenState::onEnter(engine::EngineContext& context) {
    m_logger->info("Entering SplashScreenState.");
    
    // Pre-load the splash logo manually
    if (context.m_resourceManager->loadTexture("splash_logo", "../../assets/textures/splash_logo.jpg")) {
        m_logger->info("Loaded splash logo.");
    } else {
        m_logger->error("Failed to load splash logo!");
    }

    m_phase = Phase::ShowLogo;
    m_logoTimer = 0.f;
    m_assetsLoaded = false;
    m_renderFrames = 0;
}

void SplashScreenState::onExit(engine::EngineContext& context) {
    m_logger->info("Exiting SplashScreenState.");
}

void SplashScreenState::handleEvent(engine::EngineContext& context, const sf::Event& event) {
    // SFML 3 Event handling
    if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
        if (keyPressed->code == sf::Keyboard::Key::Enter || keyPressed->code == sf::Keyboard::Key::Space) {
            if (m_assetsLoaded) {
                context.m_dispatcher->enqueue<engine::RequestStateSwapEvent>("MainMenu");
            }
        }
    }
}

void SplashScreenState::update(engine::EngineContext& context, float fixedDeltaTime) {
    switch (m_phase) {
        case Phase::ShowLogo:
            // Only increment timer if we have rendered a new frame
            // This prevents the timer from fast-forwarding during the "catch-up" phase after blocking load
            if (m_renderFrames != m_lastRenderFrame) {
                m_logoTimer += fixedDeltaTime;
                m_lastRenderFrame = m_renderFrames;
            }

            // Trigger asset loading after fade-in (0.5s) to ensure logo is fully visible
            if (m_logoTimer >= 0.5f && !m_assetsLoaded) {
                m_logger->info("Starting asset load (blocking)...");
                if (context.m_assetLoader->load("../../assets/data/resources.json")) {
                    m_logger->info("Assets loaded successfully.");
                    m_assetsLoaded = true;
                } else {
                    m_logger->error("Failed to load assets!");
                }
            }

            // Wait for both duration and asset loading
            if (m_logoTimer >= m_logoDuration && m_assetsLoaded) {
                m_phase = Phase::Transitioning;
            }
            break;

        case Phase::Loading:
            // Deprecated phase, but keeping for safety if logic drifts
            m_phase = Phase::Transitioning;
            break;

        case Phase::Transitioning:
            // Give one frame delay so render shows "Done", then swap
            context.m_dispatcher->enqueue<engine::RequestStateSwapEvent>("MainMenu");
            break;
    }
}

void SplashScreenState::render(engine::EngineContext& context) {
    m_renderFrames++;

    // Application::render() already calls beginFrame/clear(Black)/endFrame.
    // We just need to overwrite the clear with White and draw content.
    context.m_renderer->clear({255, 255, 255, 255}); // White background
    
    auto sfmlRenderer = std::dynamic_pointer_cast<engine::SFMLRenderer>(context.m_renderer);
    if (sfmlRenderer) {
        sf::RenderWindow& window = sfmlRenderer->getNativeRenderWindow();
        auto size = window.getSize();

        auto logoTex = context.m_resourceManager->getTexture("splash_logo");
        if (logoTex) {
            sf::Sprite logo(*logoTex);
            auto texSize = logoTex->getSize();
            logo.setOrigin({static_cast<float>(texSize.x) / 2.f, static_cast<float>(texSize.y) / 2.f});
            logo.setPosition({static_cast<float>(size.x) / 2.f, static_cast<float>(size.y) / 2.f});

            // Animation Logic: Fade In + Scale Pulse
            float alpha = 255.f;
            float scale = 1.0f;

            if (m_logoTimer < 0.5f) {
                // Fade in + scale up
                alpha = 255.f * (m_logoTimer / 0.5f);
                scale = 0.9f + (m_logoTimer / 0.5f) * 0.1f; // 0.9 -> 1.0
            } else if (m_logoTimer < m_logoDuration - 0.5f) {
                // Gentle pulse (sine wave)
                float pulseTime = m_logoTimer - 0.5f;
                scale = 1.0f + 0.02f * std::sin(pulseTime * 2.0f); // ±2% scale
            } else {
                // Fade out
                float fadeOutTime = m_logoTimer - (m_logoDuration - 0.5f);
                alpha = 255.f * (1.0f - fadeOutTime / 0.5f);
            }

            // Clamp alpha to safe range
            alpha = std::clamp(alpha, 0.f, 255.f);

            logo.setScale({scale, scale});
            logo.setColor(sf::Color(255, 255, 255, static_cast<std::uint8_t>(alpha)));

            window.draw(logo);
        } else {
            // Fallback if logo failed to load
            sf::RectangleShape logoRect({200.f, 100.f});
            logoRect.setFillColor(sf::Color(255, 100, 0)); // Orange
            logoRect.setOrigin({100.f, 50.f});
            logoRect.setPosition({static_cast<float>(size.x) / 2.f, static_cast<float>(size.y) / 2.f});
            window.draw(logoRect);
        }
    }
}

}
