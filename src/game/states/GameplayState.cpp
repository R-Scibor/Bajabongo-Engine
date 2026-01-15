#include "engine/pch.h"
#include "GameplayState.hpp"
#include "game/setup/GameComponentRegistry.hpp"
#include "engine/core/EngineContext.hpp"
#include "engine/core/ILogger.hpp"
#include "engine/core/ILoggerManager.hpp"
#include "engine/core/IResourceManager.hpp"
#include "engine/components/PendingPhysicsBodyComponent.hpp"
#include "engine/components/PhysicsBodyComponent.hpp"
#include "engine/components/TransformComponent.hpp"
#include "engine/components/RenderableComponent.hpp"
#include "engine/components/ParentComponent.hpp"
#include "engine/components/ChildComponent.hpp"
#include "engine/core/math/Vector2.hpp"
#include "engine/events/StateEvents.hpp"
#include "engine/events/PhysicsEvents.hpp"
#include "engine/physics/PhysicsBodyCreationSystem.hpp"
#include "engine/physics/PhysicsEventSystem.hpp"
#include "engine/physics/PhysicsSyncSystem.hpp"
#include "engine/rendering/RenderSystem.hpp"
#include "engine/rendering/CameraSystem.hpp"
#include "engine/rendering/IRenderer.hpp"
#include "engine/rendering/Sprite.hpp"
#include "engine/rendering/AnimationClip.hpp"
#include "engine/components/AnimationComponent.hpp"
#include "engine/components/CameraFocusComponent.hpp"
#include "game/components/PlayerComponent.hpp"
#include "game/components/WeaponComponent.hpp"
#include "game/systems/FogRenderSystem.hpp"
#include "game/components/VisibilityComponent.hpp"
#include "game/systems/VisibilitySystem.hpp"
#include "game/systems/VisibilityTaggingSystem.hpp"
#include "engine/ecs/ArchetypeManager.hpp"
#include "engine/ecs/EntityFactory.hpp"
#include "engine/ecs/EditorSystem.hpp"
#include "engine/assets/MapLoader.hpp"
#include <box2d/box2d.h>
#include <entt/entt.hpp>
#include <SFML/Window/Event.hpp>

namespace game {

    GameplayState::GameplayState(engine::EngineContext& context)
        : m_physicsBodyCreationSystem(context)
        , m_physicsEventSystem(context)
        , m_physicsSyncSystem(context)
        , m_renderSystem(context)
        , m_animationStateMachineSystem(context)
        , m_animationSystem(context)
        , m_cameraSystem(context)
        , m_playerControllerSystem(context)
        , m_damageSystem(context)
        , m_weaponSystem(context)
        , m_lifetimeSystem(context)
        , m_hierarchySystem(context)
        , m_visibilitySystem(context)
        , m_visibilityTaggingSystem(context)
        , m_fogRenderSystem(context)
        , m_enemyAISystem(context)
        , m_hudSystem(std::make_unique<HudSystem>(context))
    {
        m_logger = context.m_logManager->GetLogger("GameplayState");

        // Note: Create a non-owning shared_ptr for EditorSystem since it expects shared_ptr<EngineContext>.
        // This is safe because GameplayState and EditorSystem are owned by Application which owns the Context.
        auto sharedContext = std::shared_ptr<engine::EngineContext>(&context, [](engine::EngineContext*){});
        m_editorSystem = std::make_unique<engine::EditorSystem>(sharedContext);
    }

    void GameplayState::onEnter(engine::EngineContext& context) {
        if (m_logger) m_logger->info("Entering GameplayState.");
        
        if (!context.m_archetypeManager) {
            context.m_archetypeManager = std::make_shared<engine::ArchetypeManager>(context.m_logManager);
        }

        if (!context.m_entityFactory) {
            context.m_entityFactory = std::make_shared<engine::EntityFactory>(context, context.m_archetypeManager);
            
            game::RegisterGameComponents(context);
        }

        // Load archetypes
        if (context.m_archetypeManager->loadArchetypes("../../assets/data/archetypes.json")) {
            if (m_logger) m_logger->info("Archetypes loaded successfully.");
        } else {
            if (m_logger) m_logger->error("Failed to load archetypes!");
        }

        // Spawn entities
        auto& registry = *context.m_registry;

        if (context.m_entityFactory) {
            // NEW: Spawn composite archetype (player)
            entt::entity player = context.m_entityFactory->spawn("player", {155.f, 615.f});
            entt::entity target_1 = context.m_entityFactory->spawn("enemy_1", {786.f, 213.f});
            entt::entity target_2 = context.m_entityFactory->spawn("target_2", {882.f, 213.f});
            entt::entity target_3 = context.m_entityFactory->spawn("target_3", {960.f, 213.f});
            entt::entity target_4 = context.m_entityFactory->spawn("target_4", {1048.f, 213.f});
            // CameraFocus is now loaded from archetype
            
        }
        // Physics cleanup hook
        m_physicsCleanupHook =
            registry.on_destroy<engine::PhysicsBodyComponent>()
                    .connect<&GameplayState::onPhysicsBodyDestroyed>(this);

        // Hierarchy cleanup hook
        m_hierarchyCleanupHook =
            registry.on_destroy<engine::ChildComponent>()
                    .connect<&engine::HierarchySystem::onParentDestroyed>(&m_hierarchySystem);

        if (m_logger) m_logger->info("Physics and Hierarchy cleanup hooks registered.");

        // Test: Listen for Contact Events
        context.m_dispatcher->sink<engine::PhysicsContactBeginEvent>().connect<&GameplayState::onContactBegin>(this);
        context.m_dispatcher->sink<engine::PhysicsSensorBeginEvent>().connect<&GameplayState::onSensorBegin>(this);

        // --- Map Loader Test ---
        if (m_logger) m_logger->info("Loading Map...");
        engine::MapLoader mapLoader(context);
        // Load map.json which is relative to assets folder, but MapLoader uses ifstream directly.
        mapLoader.load("../../assets/data/map.json");
    }

    void GameplayState::onExit(engine::EngineContext& context) {
        if (m_logger) m_logger->info("Exiting GameplayState.");
        
        // [HACK] Brute-force Scene Clear
        // We clear the registry to remove all entities.
        // Hooks are still active, so on_destroy events will fire (cleaning up Physics bodies).
        if (m_logger) m_logger->warn("Executing Brute-force Scene Clear (registry.clear()). Global entities will be lost.");
        context.m_registry->clear();

        if (m_physicsCleanupHook) {
            context.m_registry->on_destroy<engine::PhysicsBodyComponent>().disconnect(this);
        }
        if (m_hierarchyCleanupHook) {
            m_hierarchyCleanupHook.release();
        }
        if (m_logger) m_logger->info("Cleanup hooks disconnected.");
        
        // Clean up test listeners
        context.m_dispatcher->sink<engine::PhysicsContactBeginEvent>().disconnect(this);
        context.m_dispatcher->sink<engine::PhysicsSensorBeginEvent>().disconnect(this);
    }

    void GameplayState::handleEvent(engine::EngineContext& context, const sf::Event& event) {
        if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
            if (keyPressed->code == sf::Keyboard::Key::Escape) {
                if (m_logger) m_logger->info("Escape key pressed. Requesting pop to previous state.");
                context.m_dispatcher->enqueue<engine::RequestStatePopEvent>();
            } else if (keyPressed->code == sf::Keyboard::Key::F1) {
                context.debugFlags.showEditor = !context.debugFlags.showEditor;
                if (m_logger) m_logger->info("Toggled Editor: {}", context.debugFlags.showEditor);
            }
        }
    }

    void GameplayState::update(engine::EngineContext& context, float fixedDeltaTime) {
        if (!context.debugFlags.pauseGame) {
            m_physicsBodyCreationSystem.update();
            m_playerControllerSystem.update(fixedDeltaTime);
            m_enemyAISystem.update(fixedDeltaTime);
            m_weaponSystem.update(fixedDeltaTime);
            b2World_Step(context.m_physicsWorld, fixedDeltaTime, 8);
            m_physicsEventSystem.update();
            context.m_dispatcher->update();
            m_physicsSyncSystem.update();
            m_visibilitySystem.update();
            m_visibilityTaggingSystem.update();
            m_hierarchySystem.update();
            m_lifetimeSystem.update(fixedDeltaTime);
            m_animationStateMachineSystem.update();
            m_animationSystem.update(fixedDeltaTime);
            m_cameraSystem.update(fixedDeltaTime);
        }
    }

    void GameplayState::render(engine::EngineContext& context) {
        // =================================================================
        // STEP 1: Capture Game World into Scene Texture (World-Space)
        // =================================================================
        
        m_fogRenderSystem.beginSceneCapture();
        
        // Get the scene texture as a render target
        sf::RenderTexture& sceneTarget = m_fogRenderSystem.getSceneRenderTexture();
        
        // Render all game entities to the scene texture
        m_renderSystem.setDebugDraw(context.debugFlags.showPhysics);
        m_renderSystem.update(sceneTarget);  // Inject scene texture
        
        m_fogRenderSystem.endSceneCapture();
        
        
        // =================================================================
        // STEP 2: Calculate Fog Masks (World-Space)
        // =================================================================
        
        // This updates m_sightTexture and m_explorationTexture based on
        // entities with VisibilityComponent
        m_fogRenderSystem.update();
        
        
        // =================================================================
        // STEP 3: Composite Final Image with Shader (World -> Screen)
        // =================================================================
        
        // Applies shader to combine scene + sight + exploration
        // Camera view transformation happens automatically here
        m_fogRenderSystem.renderFinal();
        
        
        // =================================================================
        // STEP 4: Render UI Overlay (Screen-Space, Always Visible)
        // =================================================================
        
        // Editor UI
        if (m_editorSystem && context.debugFlags.showEditor) {
            m_editorSystem->Update(0.0f);
            m_editorSystem->Render();
        }
        
        // HUD (ammo, health, etc.)
        if (m_hudSystem) {
            m_hudSystem->render();
        }
    }

    void GameplayState::onPhysicsBodyDestroyed(entt::registry& registry, entt::entity entity) {
        if (!m_logger) return;

        const auto& bodyComp = registry.get<engine::PhysicsBodyComponent>(entity);
        if (b2Body_IsValid(bodyComp.bodyId)) {
            m_logger->info("Destroying physics body for entity {}", static_cast<uint32_t>(entity));
            b2DestroyBody(bodyComp.bodyId);
        } else {
            m_logger->warn("Attempted to destroy an invalid physics body for entity {}", static_cast<uint32_t>(entity));
        }
    }

    void GameplayState::onContactBegin(const engine::PhysicsContactBeginEvent& event)
    {
        if (m_logger)
        {
            m_logger->debug("Contact Begin: Entity {} -> Entity {}",
                entt::to_integral(event.entityA),
                entt::to_integral(event.entityB));
        }
    }

    void GameplayState::onSensorBegin(const engine::PhysicsSensorBeginEvent& event)
    {
        if (m_logger)
        {
            m_logger->info("SENSOR ACTIVATED: Entity {} entered Sensor {}",
                entt::to_integral(event.visitorEntity),
                entt::to_integral(event.sensorEntity));
        }
    }

} // namespace game