#pragma once

#include "engine/core/IGameState.hpp"
#include "engine/physics/PhysicsBodyCreationSystem.hpp"
#include "engine/physics/PhysicsSyncSystem.hpp"
#include "engine/rendering/RenderSystem.hpp"

namespace game {

    /**
     * @brief The main gameplay state.
     *
     * This state is responsible for running the core game loop, including
     * updating physics, handling player input, and managing game entities.
     */
    class GameplayState : public engine::IGameState {
    public:
        explicit GameplayState(engine::EngineContext& context);

        void onEnter(engine::EngineContext& context) override;
        void onExit(engine::EngineContext& context) override;
        void handleEvent(engine::EngineContext& context, const sf::Event& event) override;
        void update(engine::EngineContext& context, float fixedDeltaTime) override;
        void render(engine::EngineContext& context) override;

    private:
        engine::PhysicsBodyCreationSystem m_physicsBodyCreationSystem;
        engine::PhysicsSyncSystem m_physicsSyncSystem;
        engine::RenderSystem m_renderSystem;
    };

} // namespace game