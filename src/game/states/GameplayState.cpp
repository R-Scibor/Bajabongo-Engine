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
#include "engine/physics/PhysicsBodyCreationSystem.hpp"
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
        , m_physicsSyncSystem(context)
        , m_renderSystem(context)
        , m_animationSystem(context)
    {
        m_logger = context.m_logManager->GetLogger("GameplayState");
        // Initialize EditorSystem. We need to pass a shared_ptr to context, but here we have reference.
        // EngineContext is usually managed as shared_ptr in Application.
        // We can create a shared_ptr alias if needed or EditorSystem should take reference?
        // EditorSystem constructor takes shared_ptr.
        // We don't have access to the shared_ptr that owns 'context' here directly passed in constructor.
        // But wait, Application passes reference.
        
        // Workaround: Create a shared_ptr that doesn't own the pointer (aliasing) or change EditorSystem to take reference.
        // Changing EditorSystem to take shared_ptr was my design decision earlier.
        // But checking Application.cpp:
        // Application(std::shared_ptr<EngineContext> context...)
        // It passes *m_context to states.
        
        // Let's fix EditorSystem to take reference or pointer, or just use the raw pointer since State outlives it?
        // No, EditorSystem stores m_context.
        
        // Ideally, GameplayState should get shared_ptr<EngineContext>.
        // But IGameState interface takes EngineContext&.
        
        // I will change EditorSystem to take shared_ptr, but I need to get it.
        // Actually, for now I can just store the reference or use a null deleter shared_ptr if I really want to stick to shared_ptr,
        // but that's risky if context dies (which it shouldn't).
        
        // Better: I'll modify EditorSystem to take shared_ptr, but since I don't have it here,
        // I'll construct it with a trick or change EditorSystem to take raw pointer/reference.
        // Given the architecture, EngineContext is a singleton-like struct passed around.
        // I'll make a null-deleter shared_ptr for now as a quick fix, OR better:
        // Since I am in Architect/Code mode, I can change EditorSystem.hpp to take weak_ptr or reference.
        
        // Actually, let's look at how other systems work.
        // PhysicsBodyCreationSystem takes reference.
        // EditorSystem should probably take reference too.
        // I'll check EditorSystem.hpp again.
        // "explicit EditorSystem(std::shared_ptr<EngineContext> context);"
        
        // I will change EditorSystem to take std::shared_ptr in the implementation,
        // BUT I'll pass a shared_ptr created from the reference with a null deleter to avoid double free.
        // auto sharedContext = std::shared_ptr<engine::EngineContext>(&context, [](engine::EngineContext*){});
        // This is safe as long as GameplayState lives shorter than Application (which it does).
        
        auto sharedContext = std::shared_ptr<engine::EngineContext>(&context, [](engine::EngineContext*){});
        m_editorSystem = std::make_unique<engine::EditorSystem>(sharedContext);
    }

    void GameplayState::onEnter(engine::EngineContext& context) {
        if (m_logger) m_logger->info("Entering GameplayState.");
        
        // --- Phase 5A: Load Resources & Register Sprites ---
        if (context.m_resourceManager) {
            context.m_resourceManager->loadTexture("box_texture", "../../assets/textures/box.png");
            context.m_resourceManager->loadTexture("ground_texture", "../../assets/textures/ground.png");
            context.m_resourceManager->loadTexture("testanim_texture", "../../assets/textures/testanim.png");
        } else if (m_logger) {
             m_logger->error("ResourceManager is missing in EngineContext! Textures will not be loaded.");
        }

        if (context.m_spriteManager) {
            // Register "box_sprite"
            engine::SpriteDesc boxSprite;
            boxSprite.textureId = "box_texture";
            boxSprite.uvRect = sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(32, 32));
            boxSprite.origin = sf::Vector2f(16.f, 32.f);
            context.m_spriteManager->registerSprite("box_sprite", boxSprite);

            // Register "ground_sprite"
            engine::SpriteDesc groundSprite;
            groundSprite.textureId = "ground_texture";
            groundSprite.uvRect = sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(32, 32));
            groundSprite.origin = sf::Vector2f(16.f, 32.f);
            context.m_spriteManager->registerSprite("ground_sprite", groundSprite);

            // Register 10 animation frames (each frame is 600x250, stacked vertically)
            for (int i = 0; i < 10; ++i) {
                engine::SpriteDesc frameSprite;
                frameSprite.textureId = "testanim_texture";
                frameSprite.uvRect = sf::IntRect(sf::Vector2i(0, i * 250), sf::Vector2i(600, 250));
                frameSprite.origin = sf::Vector2f(300.f, 250.f);
                context.m_spriteManager->registerSprite("testanim_frame_" + std::to_string(i), frameSprite);
            }
        } else if (m_logger) {
            m_logger->error("SpriteManager is missing in EngineContext! Sprites cannot be registered.");
        }

        // --- Animation Setup ---
        if (context.m_animationLibrary) {
            engine::AnimationClip testAnimClip;
            testAnimClip.name = "test_anim";
            testAnimClip.spriteIds = {
                "testanim_frame_0", "testanim_frame_1", "testanim_frame_2", "testanim_frame_3", "testanim_frame_4",
                "testanim_frame_5", "testanim_frame_6", "testanim_frame_7", "testanim_frame_8", "testanim_frame_9"
            };
            testAnimClip.frameDuration = 0.1f;
            testAnimClip.loop = true;
            context.m_animationLibrary->registerClip("test_anim", testAnimClip);
        } else if (m_logger) {
            m_logger->error("AnimationLibrary is missing in EngineContext! Animations cannot be registered.");
        }

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
            context.m_entityFactory->spawn("player", {400.f, 300.f});

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
    }

    void GameplayState::onExit(engine::EngineContext& context) {
        if (m_logger) m_logger->info("Exiting GameplayState.");
        
        if (m_physicsCleanupHook) {
            context.m_registry->on_destroy<engine::PhysicsBodyComponent>().disconnect(this);
            if (m_logger) m_logger->info("Physics cleanup hook disconnected.");
        }
    }

    void GameplayState::handleEvent(engine::EngineContext& context, const sf::Event& event) {
        if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
            if (keyPressed->code == sf::Keyboard::Key::Escape) {
                if (m_logger) m_logger->info("Escape key pressed. Requesting pop to previous state.");
                context.m_dispatcher->enqueue<engine::RequestStatePopEvent>();
            }
        }
    }

    void GameplayState::update(engine::EngineContext& context, float fixedDeltaTime) {
        m_physicsBodyCreationSystem.update();
        b2World_Step(context.m_physicsWorld, fixedDeltaTime, 8);
        m_physicsSyncSystem.update();
        m_animationSystem.update(fixedDeltaTime);
    }

    void GameplayState::render(engine::EngineContext& context) {
        m_renderSystem.setDebugDraw(true);
        m_renderSystem.update();

        if (m_editorSystem) {
            m_editorSystem->Update(0.0f); // dt not strictly needed for ImGui draw calls if we don't animate UI manually
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

} // namespace game