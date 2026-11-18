#pragma once

#include "HeadlessScene.hpp"
#include "engine/components/LifetimeComponent.hpp"
#include "engine/components/PendingPhysicsBodyComponent.hpp"
#include "engine/components/TransformComponent.hpp"
#include "engine/ecs/HierarchySystem.hpp"
#include "engine/physics/PhysicsBodyCreationSystem.hpp"
#include "engine/physics/PhysicsEventSystem.hpp"
#include "engine/physics/PhysicsSyncSystem.hpp"
#include "engine/systems/LifetimeSystem.hpp"
#include "game/ai/EnemyDetectionSystem.hpp"
#include "game/ai/EnemyStuckRecoverySystem.hpp"
#include "game/ai/EnemyStateTransitions.hpp"
#include "game/ai/EnemyMovementSystem.hpp"
#include "game/systems/DamageSystem.hpp"
#include "game/systems/VisibilitySystem.hpp"

#include <box2d/box2d.h>
#include <cmath>

namespace baja::test {

    inline int countEntities(entt::registry& registry) {
        int n = 0;
        for (auto entity : registry.storage<entt::entity>()) {
            if (registry.valid(entity)) {
                ++n;
            }
        }
        return n;
    }

    inline bool allTransformsFinite(entt::registry& registry) {
        for (auto entity : registry.view<engine::TransformComponent>()) {
            const auto& transform = registry.get<engine::TransformComponent>(entity);
            if (!std::isfinite(transform.position.x) || !std::isfinite(transform.position.y) ||
                !std::isfinite(transform.rotation) || !std::isfinite(transform.scale.x) ||
                !std::isfinite(transform.scale.y)) {
                return false;
            }
        }
        return true;
    }

    /// Fixed-dt gameplay systems that do not need a window, input, or audio.
    struct HeadlessGameplayLoop {
        HeadlessScene& scene;
        engine::PhysicsBodyCreationSystem bodyCreation;
        engine::PhysicsEventSystem physicsEvents;
        engine::PhysicsSyncSystem physicsSync;
        engine::HierarchySystem hierarchy;
        engine::LifetimeSystem lifetime;
        game::VisibilitySystem visibility;
        game::ai::EnemyDetectionSystem detection;
        game::ai::EnemyStateTransitions state;
        game::ai::EnemyMovementSystem movement;
        game::ai::EnemyStuckRecoverySystem stuck;
        game::DamageSystem damage;

        explicit HeadlessGameplayLoop(HeadlessScene& headlessScene)
            : scene(headlessScene)
            , bodyCreation(headlessScene.context)
            , physicsEvents(headlessScene.context)
            , physicsSync(headlessScene.context)
            , hierarchy(headlessScene.context)
            , lifetime(headlessScene.context)
            , visibility(headlessScene.context)
            , detection(headlessScene.context)
            , state(headlessScene.context)
            , movement(headlessScene.context)
            , stuck(headlessScene.context)
            , damage(headlessScene.context) {}

        void step(float dt) {
            bodyCreation.update();
            detection.update(dt);
            state.update(dt);
            movement.update(dt);
            stuck.update(dt);
            b2World_Step(scene.context.m_physicsWorld, dt, 8);
            physicsEvents.update();
            scene.dispatcher->update();
            physicsSync.update();
            visibility.update();
            hierarchy.update();
            lifetime.update(dt);
        }

        int entityCount() const { return countEntities(*scene.registry); }
        bool transformsFinite() const { return allTransformsFinite(*scene.registry); }
    };

} // namespace baja::test
