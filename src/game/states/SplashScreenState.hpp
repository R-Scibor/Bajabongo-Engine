#pragma once
#include "engine/core/IGameState.hpp"
#include <memory>
#include <SFML/Window/Event.hpp>

namespace engine {
    class ILogger;
}

namespace game {

class SplashScreenState : public engine::IGameState {
public:
    explicit SplashScreenState(engine::EngineContext& context);

    void onEnter(engine::EngineContext& context) override;
    void onExit(engine::EngineContext& context) override;
    void handleEvent(engine::EngineContext& context, const sf::Event& event) override;
    void update(engine::EngineContext& context, float fixedDeltaTime) override;
    void render(engine::EngineContext& context) override;

private:
    std::shared_ptr<engine::ILogger> m_logger;
    
    enum class Phase { ShowLogo, Loading, Transitioning };
    Phase m_phase = Phase::ShowLogo;
    
    float m_logoTimer = 0.f;
    float m_logoDuration = 2.0f; // Show logo for 2s
    
    bool m_assetsLoaded = false;
};

}
