#pragma once

#include "engine/core/IGameState.hpp"
#include "engine/events/PhysicsEvents.hpp"
#include "engine/physics/PhysicsBodyCreationSystem.hpp"
#include "engine/physics/PhysicsSyncSystem.hpp"
#include "engine/physics/PhysicsEventSystem.hpp"
#include "engine/rendering/RenderSystem.hpp"
#include "engine/rendering/AnimationSystem.hpp"
#include "engine/ecs/EditorSystem.hpp"
#include "engine/ecs/HierarchySystem.hpp"

#include <entt/fwd.hpp>
#include <memory>

namespace engine {
class ILogger;
} // namespace engine

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
        void onPhysicsBodyDestroyed(entt::registry& registry, entt::entity entity);
        void onContactBegin(const engine::PhysicsContactBeginEvent& event);
        void onSensorBegin(const engine::PhysicsSensorBeginEvent& event);

        engine::PhysicsBodyCreationSystem m_physicsBodyCreationSystem;
        engine::PhysicsEventSystem m_physicsEventSystem;
        engine::PhysicsSyncSystem m_physicsSyncSystem;
        engine::RenderSystem m_renderSystem;
        engine::AnimationSystem m_animationSystem;
        std::unique_ptr<engine::EditorSystem> m_editorSystem;
        engine::HierarchySystem m_hierarchySystem;

        std::shared_ptr<engine::ILogger> m_logger;
        entt::connection m_physicsCleanupHook;
    };

} // namespace game