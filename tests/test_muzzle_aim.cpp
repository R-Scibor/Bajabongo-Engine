#include "game/components/MuzzleComponent.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <cmath>

namespace {
    /// Distance from `point` to the infinite line through `origin` with direction `angle`.
    float distanceToRay(const engine::Vector2f& origin, float angle, const engine::Vector2f& point) {
        const float dx = point.x - origin.x;
        const float dy = point.y - origin.y;
        return std::abs(dx * std::sin(angle) - dy * std::cos(angle));
    }
} // namespace

TEST_CASE("muzzleHeightOffset falls back to the center-origin default") {
    CHECK(game::muzzleHeightOffset(nullptr) == game::DEFAULT_MUZZLE_HEIGHT_OFFSET);
    game::MuzzleComponent muzzle{.heightOffset = -96.0f};
    CHECK(game::muzzleHeightOffset(&muzzle) == -96.0f);
}

TEST_CASE("A projectile spawned at the muzzle travels through the aimed point") {
    const engine::Vector2f body{500.0f, 800.0f};
    const float heights[] = {-96.0f, -35.0f, 0.0f};
    const engine::Vector2f targets[] = {{900.0f, 800.0f},  {100.0f, 800.0f}, {500.0f, 300.0f},
                                        {500.0f, 1300.0f}, {760.0f, 590.0f}, {420.0f, 760.0f}};

    for (float height : heights) {
        const auto pivot = game::muzzlePivot(body, height);
        for (const auto& target : targets) {
            const float angle = game::aimAngleFrom(pivot, target);
            const auto spawn = game::projectileSpawnPoint(pivot, angle);
            INFO("height=" << height << " target=(" << target.x << ", " << target.y << ")");
            CHECK(distanceToRay(spawn, angle, target) < 0.01f);
            // The spawn sits between the pivot and the target, not behind the shooter.
            const float toTarget = (target.x - spawn.x) * std::cos(angle) + (target.y - spawn.y) * std::sin(angle);
            CHECK(toTarget > 0.0f);
        }
    }
}

TEST_CASE("Aiming from a different height than the spawn misses by that height difference") {
    // Old player bug: aim from body - 35, fire from body - 96. A horizontal shot
    // then flies 61 units above the cursor for its whole path.
    const engine::Vector2f body{0.0f, 0.0f};
    const engine::Vector2f target{400.0f, -35.0f};
    const float wrongAngle = game::aimAngleFrom(game::muzzlePivot(body, -35.0f), target);
    const auto spawn = game::projectileSpawnPoint(game::muzzlePivot(body, -96.0f), wrongAngle);
    CHECK(distanceToRay(spawn, wrongAngle, target) == Catch::Approx(61.0f).margin(0.01f));
}

#include "HeadlessScene.hpp"
#include "engine/components/PendingPhysicsBodyComponent.hpp"
#include "engine/components/PhysicsBodyComponent.hpp"
#include "game/components/ProjectileComponent.hpp"
#include "game/components/WeaponComponent.hpp"
#include "game/systems/WeaponSystem.hpp"

namespace {
    entt::entity makeShooter(baja::test::HeadlessScene& scene, const engine::Vector2f& position) {
        auto entity = scene.registry->create();
        b2BodyDef bodyDef = b2DefaultBodyDef();
        bodyDef.type = b2_dynamicBody;
        bodyDef.position = {position.x, position.y};
        scene.registry->emplace<engine::PhysicsBodyComponent>(entity, b2CreateBody(scene.worldId, &bodyDef));

        auto& weapon = scene.registry->emplace<game::WeaponComponent>(entity);
        weapon.baseSpreadDeg = 0.0f;
        weapon.currentSpreadDeg = 0.0f;
        weapon.spreadPerShotDeg = 0.0f;
        weapon.recoilKickDeg = 0.0f;
        weapon.projectilesPerShot = 1;
        weapon.currentAmmo = 10;
        weapon.cooldownTimer = 0.0f;
        weapon.wantsToShoot = true;
        weapon.shootSoundId.clear();
        return entity;
    }

    /// Aims like PlayerControllerSystem / EnemyCombatSystem, fires once, returns the miss distance.
    float fireAndMeasureMiss(baja::test::HeadlessScene& scene, entt::entity shooter, const engine::Vector2f& body,
                             const engine::Vector2f& target) {
        auto& weapon = scene.registry->get<game::WeaponComponent>(shooter);
        const auto pivot =
            game::muzzlePivot(body, game::muzzleHeightOffset(scene.registry->try_get<game::MuzzleComponent>(shooter)));
        weapon.aimAngle = game::aimAngleFrom(pivot, target);

        game::WeaponSystem system(scene.context);
        system.update(1.0f / 60.0f);

        auto view = scene.registry->view<game::ProjectileComponent, engine::PendingPhysicsBodyComponent>();
        for (auto projectile : view) {
            if (view.get<game::ProjectileComponent>(projectile).owner != shooter) continue;
            const auto& pending = view.get<engine::PendingPhysicsBodyComponent>(projectile);
            const float angle = std::atan2(pending.initialVelocity.y, pending.initialVelocity.x);
            return distanceToRay(pending.position, angle, target);
        }
        return -1.0f;
    }
} // namespace

TEST_CASE("WeaponSystem fires an enemy (no MuzzleComponent) shot through its aim point") {
    baja::test::HeadlessScene scene;
    const engine::Vector2f body{300.0f, 300.0f};
    auto shooter = makeShooter(scene, body);
    // Player hurtbox center about 250 units to the right, roughly level with the enemy.
    const float miss = fireAndMeasureMiss(scene, shooter, body, {550.0f, 320.0f});
    REQUIRE(miss >= 0.0f);
    CHECK(miss < 0.5f);
}

TEST_CASE("WeaponSystem fires a player (feet origin, MuzzleComponent) shot through the cursor") {
    baja::test::HeadlessScene scene;
    const engine::Vector2f body{300.0f, 300.0f};
    auto shooter = makeShooter(scene, body);
    scene.registry->emplace<game::MuzzleComponent>(shooter, game::MuzzleComponent{.heightOffset = -96.0f});
    const float miss = fireAndMeasureMiss(scene, shooter, body, {700.0f, 240.0f});
    REQUIRE(miss >= 0.0f);
    CHECK(miss < 0.5f);
}
