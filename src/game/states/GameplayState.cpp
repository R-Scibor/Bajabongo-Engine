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
    }

    void GameplayState::onEnter(engine::EngineContext& context) {
        if (m_logger) m_logger->info("Entering GameplayState.");
        
        // --- Phase 5A: Load Resources & Register Sprites ---
        if (context.m_resourceManager) {
            context.m_resourceManager->loadTexture("box_texture", "../../assets/textures/box.png");
            context.m_resourceManager->loadTexture("ground_texture", "../../assets/textures/ground.png");
            context.m_resourceManager->loadTexture("testanim_texture", "../../assets/textures/testanim.png");
        }

        if (context.m_spriteManager) {
            // Register "box_sprite"
            engine::SpriteDesc boxSprite;
            boxSprite.textureId = "box_texture";
            boxSprite.uvRect = sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(32, 32));
            boxSprite.origin = sf::Vector2f(16.f, 16.f);
            context.m_spriteManager->registerSprite("box_sprite", boxSprite);

            // Register "ground_sprite"
            engine::SpriteDesc groundSprite;
            groundSprite.textureId = "ground_texture";
            groundSprite.uvRect = sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(32, 32));
            groundSprite.origin = sf::Vector2f(16.f, 16.f);
            context.m_spriteManager->registerSprite("ground_sprite", groundSprite);

            // Register 10 animation frames (each frame is 600x250, stacked vertically)
            for (int i = 0; i < 10; ++i) {
                engine::SpriteDesc frameSprite;
                frameSprite.textureId = "testanim_texture";
                frameSprite.uvRect = sf::IntRect(sf::Vector2i(0, i * 250), sf::Vector2i(600, 250));
                frameSprite.origin = sf::Vector2f(300.f, 125.f);
                context.m_spriteManager->registerSprite("testanim_frame_" + std::to_string(i), frameSprite);
            }
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
        }

        auto& registry = *context.m_registry;

        // Box – dynamic body (static sprite, no animation)
        auto box = registry.create();
        registry.emplace<engine::PendingPhysicsBodyComponent>(
            box,
            engine::Vector2f{ 100.f, 100.f },
            engine::Vector2f{ 20.f, 20.f },
            false,
            0.5f
        );
        registry.emplace<engine::TransformComponent>(box);
        registry.emplace<engine::RenderableComponent>(box, "box_sprite", 1, sf::Color::White);

        // Animated test entity
        auto animatedEntity = registry.create();
        engine::TransformComponent animTransform;
        animTransform.position = engine::Vector2f{ 400.f, 300.f };
        registry.emplace<engine::TransformComponent>(animatedEntity, animTransform);
        registry.emplace<engine::RenderableComponent>(animatedEntity, "testanim_frame_0", 2, sf::Color::White);
        
        engine::AnimationComponent animComp;
        animComp.currentClipId = "test_anim";
        animComp.isPlaying = true;
        registry.emplace<engine::AnimationComponent>(animatedEntity, animComp);

        // Ground – static body
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