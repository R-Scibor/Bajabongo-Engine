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
#include "engine/rendering/CameraSystem.hpp"
#include "engine/systems/LifetimeSystem.hpp"

#include "game/systems/PlayerControllerSystem.hpp"
#include "game/systems/DamageSystem.hpp"
#include "game/systems/WeaponSystem.hpp"
#include "game/systems/WeaponSwitchSystem.hpp"
#include "game/systems/HudSystem.hpp"
#include "game/systems/VisibilitySystem.hpp"
#include "game/systems/VisibilityTaggingSystem.hpp"
#include "game/systems/FogRenderSystem.hpp"
#include "game/systems/EnemyAISystem.hpp"
#include "game/systems/PickupSystem.hpp"
#include "game/systems/HitFlashSystem.hpp"

#include "engine/systems/AnimationStateMachineSystem.hpp"
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

        engine::PhysicsBodyCreationSystem m_physicsBodyCreationSystem;
        engine::PhysicsEventSystem m_physicsEventSystem;
        engine::PhysicsSyncSystem m_physicsSyncSystem;
        engine::RenderSystem m_renderSystem;
        engine::AnimationStateMachineSystem m_animationStateMachineSystem;
        engine::AnimationSystem m_animationSystem;
        engine::CameraSystem m_cameraSystem;
        DamageSystem m_damageSystem;
        PlayerControllerSystem m_playerControllerSystem;
        WeaponSystem m_weaponSystem;
        WeaponSwitchSystem m_weaponSwitchSystem;
        engine::LifetimeSystem m_lifetimeSystem;
        std::unique_ptr<engine::EditorSystem> m_editorSystem;
        engine::HierarchySystem m_hierarchySystem;
        VisibilitySystem m_visibilitySystem;
        VisibilityTaggingSystem m_visibilityTaggingSystem;
        FogRenderSystem m_fogRenderSystem;
        EnemyAISystem m_enemyAISystem;
        PickupSystem m_pickupSystem;
        HitFlashSystem m_hitFlashSystem;
        std::unique_ptr<HudSystem> m_hudSystem;

        std::shared_ptr<engine::ILogger> m_logger;
        entt::connection m_physicsCleanupHook;
        entt::connection m_hierarchyCleanupHook;
    };

} // namespace game