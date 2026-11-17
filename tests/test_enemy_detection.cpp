#include "HeadlessScene.hpp"
#include "engine/components/TransformComponent.hpp"
#include "engine/physics/PhysicsConstants.hpp"
#include "game/ai/DetectionMath.hpp"
#include "game/ai/EnemyDetectionSystem.hpp"
#include "game/components/EnemyComponent.hpp"
#include "game/components/HealthComponent.hpp"
#include "game/components/PlayerComponent.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <entt/entt.hpp>

using Catch::Approx;

TEST_CASE("isWithinRadius is a strict less-than on squared distance") {
    engine::Vector2f origin{0.0f, 0.0f};
    CHECK(game::ai::isWithinRadius(origin, {9.0f, 0.0f}, 10.0f));
    CHECK_FALSE(game::ai::isWithinRadius(origin, {10.0f, 0.0f}, 10.0f));
}

TEST_CASE("hasForgottenTarget is true only after the memory duration") {
    CHECK_FALSE(game::ai::hasForgottenTarget(1.0f, 1.0f));
    CHECK(game::ai::hasForgottenTarget(1.01f, 1.0f));
    CHECK_FALSE(game::ai::hasForgottenTarget(0.0f, 1.0f));
}

TEST_CASE("isEnemyDefeated is true at zero HP and ignores a missing component") {
    game::HealthComponent health;
    health.currentHp = 0.0f;
    CHECK(game::ai::isEnemyDefeated(&health));
    health.currentHp = 1.0f;
    CHECK_FALSE(game::ai::isEnemyDefeated(&health));
    CHECK_FALSE(game::ai::isEnemyDefeated(nullptr));
}

TEST_CASE("EnemyDetectionSystem does nothing when there is no player") {
    baja::test::HeadlessScene scene;
    game::ai::EnemyDetectionSystem system(scene.context);

    auto enemy = scene.registry->create();
    scene.registry->emplace<game::EnemyComponent>(enemy);
    scene.registry->emplace<engine::TransformComponent>(enemy);

    system.update(0.016f);
    CHECK(scene.registry->get<game::EnemyComponent>(enemy).targetEntity == entt::null);
}

TEST_CASE("EnemyDetectionSystem marks a zero-HP enemy Dead") {
    baja::test::HeadlessScene scene;
    game::ai::EnemyDetectionSystem system(scene.context);

    auto player = scene.registry->create();
    scene.registry->emplace<game::PlayerComponent>(player);
    scene.registry->emplace<engine::TransformComponent>(player);

    auto enemy = scene.registry->create();
    scene.registry->emplace<game::EnemyComponent>(enemy);
    scene.registry->emplace<engine::TransformComponent>(enemy);
    auto& health = scene.registry->emplace<game::HealthComponent>(enemy);
    health.currentHp = 0.0f;

    system.update(0.016f);
    CHECK(scene.registry->get<game::EnemyComponent>(enemy).currentState == game::EnemyState::Dead);
}

TEST_CASE("EnemyDetectionSystem ignores a player outside the detection radius") {
    baja::test::HeadlessScene scene;
    game::ai::EnemyDetectionSystem system(scene.context);

    auto player = scene.registry->create();
    scene.registry->emplace<game::PlayerComponent>(player);
    scene.registry->emplace<engine::TransformComponent>(player).position = {500.0f, 0.0f};

    auto enemy = scene.registry->create();
    auto& enemyComp = scene.registry->emplace<game::EnemyComponent>(enemy);
    enemyComp.detectionRadius = 100.0f;
    scene.registry->emplace<engine::TransformComponent>(enemy).position = {0.0f, 0.0f};

    system.update(0.016f);
    CHECK(enemyComp.targetEntity == entt::null);
    CHECK_FALSE(enemyComp.hasLineOfSight);
    CHECK_FALSE(enemyComp.isAlerted);
}

TEST_CASE("EnemyDetectionSystem acquires a visible player in range and alerts neighbors") {
    baja::test::HeadlessScene scene;
    game::ai::EnemyDetectionSystem system(scene.context);

    auto player = scene.registry->create();
    scene.registry->emplace<game::PlayerComponent>(player);
    scene.registry->emplace<engine::TransformComponent>(player).position = {50.0f, 0.0f};

    auto enemy = scene.registry->create();
    auto& enemyComp = scene.registry->emplace<game::EnemyComponent>(enemy);
    enemyComp.detectionRadius = 100.0f;
    enemyComp.alertRadius = 80.0f;
    scene.registry->emplace<engine::TransformComponent>(enemy).position = {0.0f, 0.0f};

    auto nearEnemy = scene.registry->create();
    auto& nearComp = scene.registry->emplace<game::EnemyComponent>(nearEnemy);
    nearComp.detectionRadius = 20.0f;
    scene.registry->emplace<engine::TransformComponent>(nearEnemy).position = {10.0f, 0.0f};

    auto farEnemy = scene.registry->create();
    auto& farComp = scene.registry->emplace<game::EnemyComponent>(farEnemy);
    farComp.detectionRadius = 50.0f;
    scene.registry->emplace<engine::TransformComponent>(farEnemy).position = {200.0f, 0.0f};

    system.update(0.016f);
    CHECK(enemyComp.targetEntity == player);
    CHECK(enemyComp.hasLineOfSight);
    CHECK(enemyComp.isAlerted);
    CHECK(enemyComp.lastKnownPosition.x == 50.0f);
    CHECK(nearComp.currentState == game::EnemyState::Alert);
    CHECK(nearComp.isAlerted);
    CHECK(nearComp.lastKnownPosition.x == 50.0f);
    CHECK(farComp.currentState == game::EnemyState::Idle);
    CHECK_FALSE(farComp.isAlerted);
}

TEST_CASE("EnemyDetectionSystem forgets a target after the memory duration") {
    baja::test::HeadlessScene scene;
    game::ai::EnemyDetectionSystem system(scene.context);

    auto player = scene.registry->create();
    scene.registry->emplace<game::PlayerComponent>(player);
    scene.registry->emplace<engine::TransformComponent>(player).position = {500.0f, 0.0f};

    auto enemy = scene.registry->create();
    auto& enemyComp = scene.registry->emplace<game::EnemyComponent>(enemy);
    enemyComp.detectionRadius = 100.0f;
    enemyComp.targetEntity = player;
    enemyComp.lastSeenTimer = 0.9f;
    enemyComp.memoryDuration = 1.0f;
    scene.registry->emplace<engine::TransformComponent>(enemy).position = {0.0f, 0.0f};

    system.update(0.05f);
    CHECK(enemyComp.targetEntity == player);
    CHECK(enemyComp.lastSeenTimer == Approx(0.95f).margin(0.0001f));

    system.update(0.10f);
    CHECK(enemyComp.targetEntity == entt::null);
    CHECK(enemyComp.lastSeenTimer == Approx(1.05f).margin(0.0001f));
}

TEST_CASE("EnemyDetectionSystem does not acquire a player behind a wall") {
    baja::test::HeadlessScene scene;
    scene.addStaticBox(25.0f, 0.0f, 2.0f, 20.0f,
                       engine::PhysicsCategory::VisibilityBlocker | engine::PhysicsCategory::Wall);
    game::ai::EnemyDetectionSystem system(scene.context);

    auto player = scene.registry->create();
    scene.registry->emplace<game::PlayerComponent>(player);
    scene.registry->emplace<engine::TransformComponent>(player).position = {50.0f, 0.0f};

    auto enemy = scene.registry->create();
    auto& enemyComp = scene.registry->emplace<game::EnemyComponent>(enemy);
    enemyComp.detectionRadius = 100.0f;
    scene.registry->emplace<engine::TransformComponent>(enemy).position = {0.0f, 0.0f};

    system.update(0.016f);
    CHECK_FALSE(enemyComp.hasLineOfSight);
    CHECK(enemyComp.targetEntity == entt::null);
}
