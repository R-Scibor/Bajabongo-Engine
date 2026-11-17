#include "game/components/WeaponComponent.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("weaponCanFire requires a trigger, ammo, and a ready cooldown") {
    game::WeaponComponent weapon;
    weapon.currentAmmo = 10;
    weapon.cooldownTimer = 0.0f;
    weapon.wantsToShoot = false;
    weapon.isBursting = false;
    CHECK_FALSE(game::weaponCanFire(weapon));

    weapon.wantsToShoot = true;
    CHECK(game::weaponCanFire(weapon));

    weapon.cooldownTimer = 0.1f;
    CHECK_FALSE(game::weaponCanFire(weapon));
    weapon.cooldownTimer = 0.0f;
    weapon.currentAmmo = 0;
    CHECK_FALSE(game::weaponCanFire(weapon));

    weapon.currentAmmo = 1;
    weapon.wantsToShoot = false;
    weapon.isBursting = true;
    CHECK(game::weaponCanFire(weapon));
}

TEST_CASE("finishReload fills the magazine from the reserve") {
    game::WeaponComponent weapon;
    weapon.magSize = 30;
    weapon.currentAmmo = 5;
    weapon.totalAmmo = 90;
    weapon.isReloading = true;

    game::finishReload(weapon);
    CHECK(weapon.currentAmmo == 30);
    CHECK(weapon.totalAmmo == 65);
    CHECK_FALSE(weapon.isReloading);
}

TEST_CASE("finishReload uses the remaining reserve when it is smaller than the mag gap") {
    game::WeaponComponent weapon;
    weapon.magSize = 30;
    weapon.currentAmmo = 10;
    weapon.totalAmmo = 7;
    weapon.isReloading = true;

    game::finishReload(weapon);
    CHECK(weapon.currentAmmo == 17);
    CHECK(weapon.totalAmmo == 0);
    CHECK_FALSE(weapon.isReloading);
}

TEST_CASE("canStartReload and startReload follow mag and reserve rules") {
    game::WeaponComponent weapon;
    weapon.magSize = 30;
    weapon.currentAmmo = 30;
    weapon.totalAmmo = 90;
    weapon.reloadDuration = 2.0f;
    CHECK_FALSE(game::canStartReload(weapon));

    weapon.currentAmmo = 10;
    CHECK(game::canStartReload(weapon));
    game::startReload(weapon);
    CHECK(weapon.isReloading);
    CHECK(weapon.reloadTimer == 2.0f);

    weapon = {};
    weapon.magSize = 30;
    weapon.currentAmmo = 0;
    weapon.totalAmmo = 0;
    CHECK_FALSE(game::canStartReload(weapon));
}
