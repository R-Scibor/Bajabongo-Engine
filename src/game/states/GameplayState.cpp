#include "engine/pch.h"
#include "GameplayState.hpp"
#include "engine/core/EngineContext.hpp"
#include "engine/core/ILogger.hpp"
#include "engine/core/ILoggerManager.hpp"
#include "engine/core/IResourceManager.hpp"
#include "engine/components/PendingPhysicsBodyComponent.hpp"
#include "engine/components/PhysicsBodyComponent.hpp"
#include "engine/components/TransformComponent.hpp"
#include "engine/components/RenderableComponent.hpp"
#include "engine/core/math/Vector2.hpp"
#include "engine/events/StateEvents.hpp"
#include "engine/events/PhysicsEvents.hpp"
#include "engine/physics/PhysicsBodyCreationSystem.hpp"
#include "engine/physics/PhysicsEventSystem.hpp"
#include "engine/physics/PhysicsSyncSystem.hpp"
#include "engine/rendering/RenderSystem.hpp"
#include "engine/rendering/Sprite.hpp"
#include "engine/rendering/AnimationClip.hpp"
#include "engine/components/AnimationComponent.hpp"
#include "engine/ecs/ArchetypeManager.hpp"
#include "engine/ecs/EntityFactory.hpp"
#include "engine/ecs/EditorSystem.hpp"
#include <box2d/box2d.h>
#include <entt/entt.hpp>
#include <SFML/Window/Event.hpp>

namespace game {

    GameplayState::GameplayState(engine::EngineContext& context)
        : m_physicsBodyCreationSystem(context)
        , m_physicsEventSystem(context)
        , m_physicsSyncSystem(context)
        , m_renderSystem(context)
        , m_animationSystem(context)
    {
        m_logger = context.m_logManager->GetLogger("GameplayState");

        // Note: Create a non-owning shared_ptr for EditorSystem since it expects shared_ptr<EngineContext>.
        // This is safe because GameplayState and EditorSystem are owned by Application which owns the Context.
        auto sharedContext = std::shared_ptr<engine::EngineContext>(&context, [](engine::EngineContext*){});
        m_editorSystem = std::make_unique<engine::EditorSystem>(sharedContext);
    }

    void GameplayState::onEnter(engine::EngineContext& context) {
        if (m_logger) m_logger->info("Entering GameplayState.");
        
        // --- Phase 6: Data-Driven Systems Init ---
        if (!context.m_archetypeManager) {
            context.m_archetypeManager = std::make_shared<engine::ArchetypeManager>(context.m_logManager);
        }

        if (!context.m_entityFactory) {
            context.m_entityFactory = std::make_shared<engine::EntityFactory>(context, context.m_archetypeManager);
        }

        // Load archetypes
        if (context.m_archetypeManager->loadArchetypes("../../assets/data/archetypes.json")) {
            if (m_logger) m_logger->info("Archetypes loaded successfully.");
        } else {
            if (m_logger) m_logger->error("Failed to load archetypes!");
        }

        // Spawn entities
        if (context.m_entityFactory) {
            // Player (using testanim_frame_0 and test_anim via JSON update)
            // Spawn player above the sensor to force collision
            context.m_entityFactory->spawn("player", {500.f, 100.f});

            // Boxes
            context.m_entityFactory->spawn("wooden_crate", {100.f, 100.f});
            context.m_entityFactory->spawn("heavy_crate", {300.f, 500.f}); // Demonstration of density
        }

        auto& registry = *context.m_registry;

        // Ground – static body (keeping manual for now as it's unique/level geometry, or could be an archetype)
        // Let's keep ground manual for simplicity or move to archetype if needed.
        // The prompt example showed entities like player/enemies/crates being spawned.
        // Ground usually belongs to tilemap or level data.
        auto ground = registry.create();
        registry.emplace<engine::PendingPhysicsBodyComponent>(
            ground,
            engine::Vector2f{ 400.f, 600.f },
            engine::Vector2f{ 800.f, 40.f },
            true,
            0.0f
        );
        registry.emplace<engine::TransformComponent>(ground);
        registry.emplace<engine::RenderableComponent>(ground, "ground_sprite", 0, sf::Color::White);

        m_physicsCleanupHook =
            registry.on_destroy<engine::PhysicsBodyComponent>()
                    .connect<&GameplayState::onPhysicsBodyDestroyed>(this);
        if (m_logger) m_logger->info("Physics cleanup hook registered.");

        // Test: Listen for Contact Events
        context.m_dispatcher->sink<engine::PhysicsContactBeginEvent>().connect<&GameplayState::onContactBegin>(this);
        context.m_dispatcher->sink<engine::PhysicsSensorBeginEvent>().connect<&GameplayState::onSensorBegin>(this);

        // Test: Spawn a Sensor Trigger
        // We'll put it near the player so they can walk into it.
        auto trigger = registry.create();
        registry.emplace<engine::PendingPhysicsBodyComponent>(
            trigger,
            engine::Vector2f{ 500.f, 300.f }, // Right of the player
            engine::Vector2f{ 50.f, 50.f },
            true, // static
            0.0f,
            true // isSensor
        );
        
        // Add a visual so we can see it
        registry.emplace<engine::TransformComponent>(trigger, engine::Vector2f{500.f, 300.f});
        registry.emplace<engine::RenderableComponent>(trigger, "box_sprite", 1, sf::Color::Green);
    }

    void GameplayState::onExit(engine::EngineContext& context) {
        if (m_logger) m_logger->info("Exiting GameplayState.");
        
        if (m_physicsCleanupHook) {
            context.m_registry->on_destroy<engine::PhysicsBodyComponent>().disconnect(this);
            if (m_logger) m_logger->info("Physics cleanup hook disconnected.");
        }
        
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
            b2World_Step(context.m_physicsWorld, fixedDeltaTime, 8);
            m_physicsEventSystem.update();
            context.m_dispatcher->update();
            m_physicsSyncSystem.update();
            m_animationSystem.update(fixedDeltaTime);
        }
    }

    void GameplayState::render(engine::EngineContext& context) {
        m_renderSystem.setDebugDraw(context.debugFlags.showPhysics);
        m_renderSystem.update();

        if (m_editorSystem && context.debugFlags.showEditor) {
            // DT is currently not used for UI logic but passed for API consistency
            m_editorSystem->Update(0.0f);
            m_editorSystem->Render();
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