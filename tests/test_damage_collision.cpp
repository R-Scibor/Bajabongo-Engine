#include "HeadlessScene.hpp"
#include "game/components/DamageComponent.hpp"
#include "game/components/DeadBodyComponent.hpp"
#include "game/components/EnemyComponent.hpp"
#include "game/components/HealthComponent.hpp"
#include "game/components/HitFlashComponent.hpp"
#include "game/components/ProjectileComponent.hpp"
#include "game/systems/DamageCollision.hpp"
#include "game/systems/DamageSystem.hpp"
#include "engine/components/AnimationStateComponent.hpp"
#include "engine/components/RenderableComponent.hpp"
#include "engine/components/TransformComponent.hpp"
#include "engine/events/PhysicsEvents.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("isProjectileOwnedByTarget is true only when owner matches") {
    game::ProjectileComponent projectile;
    auto owner = static_cast<entt::entity>(1);
    auto other = static_cast<entt::entity>(2);
    projectile.owner = owner;

    CHECK(game::isProjectileOwnedByTarget(&projectile, owner));
    CHECK_FALSE(game::isProjectileOwnedByTarget(&projectile, other));
    CHECK_FALSE(game::isProjectileOwnedByTarget(nullptr, owner));
}

TEST_CASE("shouldDestroyProjectileOnNonDamageHit is false for sensors") {
    CHECK_FALSE(game::shouldDestroyProjectileOnNonDamageHit(true));
    CHECK(game::shouldDestroyProjectileOnNonDamageHit(false));
}

TEST_CASE("contactProjectileToDestroy prefers entity A when both are projectiles") {
    CHECK(game::contactProjectileToDestroy(true, false) == game::ContactProjectile::EntityA);
    CHECK(game::contactProjectileToDestroy(false, true) == game::ContactProjectile::EntityB);
    CHECK(game::contactProjectileToDestroy(true, true) == game::ContactProjectile::EntityA);
    CHECK(game::contactProjectileToDestroy(false, false) == game::ContactProjectile::None);
}

TEST_CASE("DamageSystem onContactBegin destroys only the projectile") {
    baja::test::HeadlessScene scene;
    game::DamageSystem system(scene.context);

    auto projectile = scene.registry->create();
    scene.registry->emplace<game::ProjectileComponent>(projectile);
    auto wall = scene.registry->create();

    system.onContactBegin({.entityA = projectile, .entityB = wall});
    CHECK_FALSE(scene.registry->valid(projectile));
    CHECK(scene.registry->valid(wall));
}

TEST_CASE("DamageSystem onContactBegin leaves two non-projectiles alone") {
    baja::test::HeadlessScene scene;
    game::DamageSystem system(scene.context);

    auto a = scene.registry->create();
    auto b = scene.registry->create();
    system.onContactBegin({.entityA = a, .entityB = b});
    CHECK(scene.registry->valid(a));
    CHECK(scene.registry->valid(b));
}

TEST_CASE("DamageSystem ignores a projectile hitting its owner") {
    baja::test::HeadlessScene scene;
    game::DamageSystem system(scene.context);

    auto owner = scene.registry->create();
    auto& health = scene.registry->emplace<game::HealthComponent>(owner);
    health.currentHp = 50.0f;
    health.maxHp = 50.0f;

    auto projectile = scene.registry->create();
    scene.registry->emplace<game::ProjectileComponent>(projectile).owner = owner;
    scene.registry->emplace<game::DamageComponent>(projectile).damageValue = 10.0f;

    system.onSensorBegin({.sensorEntity = projectile, .visitorEntity = owner, .isVisitorSensor = false});
    CHECK(scene.registry->valid(projectile));
    CHECK(health.currentHp == 50.0f);
}

TEST_CASE("DamageSystem applies non-lethal damage and starts a hit flash") {
    baja::test::HeadlessScene scene;
    game::DamageSystem system(scene.context);

    auto target = scene.registry->create();
    auto& health = scene.registry->emplace<game::HealthComponent>(target);
    health.currentHp = 40.0f;
    health.maxHp = 40.0f;
    auto& flash = scene.registry->emplace<game::HitFlashComponent>(target);
    auto& renderable = scene.registry->emplace<engine::RenderableComponent>(target);
    renderable.color = sf::Color::Green;

    auto projectile = scene.registry->create();
    scene.registry->emplace<game::ProjectileComponent>(projectile);
    scene.registry->emplace<game::DamageComponent>(projectile).damageValue = 15.0f;

    system.onSensorBegin({.sensorEntity = projectile, .visitorEntity = target, .isVisitorSensor = false});
    CHECK(health.currentHp == 25.0f);
    CHECK(flash.isFlashing);
    CHECK(flash.currentTimer == 0.0f);
    CHECK(flash.originalColor == sf::Color::Green);
    CHECK(renderable.color == flash.flashColor);
    CHECK_FALSE(scene.registry->valid(projectile));
    CHECK(scene.registry->valid(target));
}

TEST_CASE("DamageSystem lets a projectile pass through a sensor without health") {
    baja::test::HeadlessScene scene;
    game::DamageSystem system(scene.context);

    auto sensor = scene.registry->create();
    auto projectile = scene.registry->create();
    scene.registry->emplace<game::ProjectileComponent>(projectile);
    scene.registry->emplace<game::DamageComponent>(projectile).damageValue = 5.0f;

    system.onSensorBegin({.sensorEntity = projectile, .visitorEntity = sensor, .isVisitorSensor = true});
    CHECK(scene.registry->valid(projectile));
    CHECK(scene.registry->valid(sensor));
}

TEST_CASE("DamageSystem destroys a projectile that hits a solid with no health") {
    baja::test::HeadlessScene scene;
    game::DamageSystem system(scene.context);

    auto wall = scene.registry->create();
    auto projectile = scene.registry->create();
    scene.registry->emplace<game::ProjectileComponent>(projectile);
    scene.registry->emplace<game::DamageComponent>(projectile).damageValue = 5.0f;

    system.onSensorBegin({.sensorEntity = projectile, .visitorEntity = wall, .isVisitorSensor = false});
    CHECK_FALSE(scene.registry->valid(projectile));
    CHECK(scene.registry->valid(wall));
}

TEST_CASE("DamageSystem spawns a DeadBody when a lethal hit defeats an enemy") {
    baja::test::HeadlessScene scene;
    game::DamageSystem system(scene.context);

    auto target = scene.registry->create();
    auto& health = scene.registry->emplace<game::HealthComponent>(target);
    health.currentHp = 8.0f;
    health.maxHp = 8.0f;
    scene.registry->emplace<engine::TransformComponent>(target).position = {12.0f, 4.0f};
    scene.registry->emplace<engine::RenderableComponent>(target);
    auto& anim = scene.registry->emplace<engine::AnimationStateComponent>(target);
    anim.animationSetId = "unit";
    scene.registry->emplace<game::EnemyComponent>(target);

    auto projectile = scene.registry->create();
    scene.registry->emplace<game::ProjectileComponent>(projectile);
    scene.registry->emplace<game::DamageComponent>(projectile).damageValue = 10.0f;

    system.onSensorBegin({.sensorEntity = projectile, .visitorEntity = target, .isVisitorSensor = false});
    CHECK_FALSE(scene.registry->valid(target));
    CHECK_FALSE(scene.registry->valid(projectile));

    int bodies = 0;
    auto view = scene.registry->view<game::DeadBodyComponent, engine::AnimationStateComponent,
                                     engine::TransformComponent, engine::RenderableComponent>();
    for (auto entity : view) {
        ++bodies;
        CHECK(view.get<engine::AnimationStateComponent>(entity).state == engine::AnimationState::Dead);
        CHECK(view.get<engine::AnimationStateComponent>(entity).animationSetId == "unit");
        CHECK(view.get<engine::TransformComponent>(entity).position.x == 12.0f);
        CHECK(view.get<engine::RenderableComponent>(entity).layer == 0);
    }
    CHECK(bodies == 1);
}
