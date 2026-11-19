#pragma once
#include <string>

namespace game {
    struct WeaponComponent {
        // Audio
        std::string shootSoundId;
        bool shootSoundLooping = false;
        bool isFiringAudioLoop = false; // Runtime state

        float cooldownTimer = 0.0f;
        float fireRate = 0.2f; // Time between shots in seconds
        float projectileSpeed = 500.0f;
        float projectileLifetime = 2.0f;
        float damage = 10.0f;

        // Spread (shot randomness)
        float baseSpreadDeg = 0.0f;
        float currentSpreadDeg = 0.0f; // Runtime bloom
        float spreadPerShotDeg = 0.0f;
        float spreadDecayDegPerSec = 0.0f;
        float maxSpreadDeg = 0.0f;

        // Recoil (aim kick)
        float recoilKickDeg = 0.0f;
        float recoilReturnDegPerSec = 0.0f;
        float recoilAngleOffset = 0.0f; // Runtime offset

        // Multi-projectile (shotgun)
        int projectilesPerShot = 1;

        // Burst Fire
        int shotsPerBurst = 1;
        int burstShotsFired = 0;
        float burstCooldown = 0.0f;
        bool isBursting = false;

        // Ammo & Reload
        int currentAmmo = 30;
        int magSize = 30;
        int totalAmmo = 90;
        float reloadDuration = 2.0f;
        float reloadTimer = 0.0f;
        bool isReloading = false;

        // Control State (Set by Controller/AI)
        bool wantsToShoot = false;
        bool wantsToReload = false;
        float aimAngle = 0.0f;
    };

    // Muzzle height lives in MuzzleComponent.hpp (per shooter).
    // Target center/chest height relative to entity position, used when the target has no hurtbox.
    constexpr float TARGET_CHEST_HEIGHT_OFFSET = -35.0f;

    inline bool weaponCanFire(const WeaponComponent& weapon) {
        const bool trigger = weapon.wantsToShoot || weapon.isBursting;
        return trigger && weapon.cooldownTimer <= 0.0f && weapon.currentAmmo > 0;
    }

    inline bool canStartReload(const WeaponComponent& weapon) {
        return weapon.currentAmmo < weapon.magSize && weapon.totalAmmo > 0;
    }

    inline void startReload(WeaponComponent& weapon) {
        weapon.isReloading = true;
        weapon.reloadTimer = weapon.reloadDuration;
    }

    inline void finishReload(WeaponComponent& weapon) {
        int ammoNeeded = weapon.magSize - weapon.currentAmmo;
        int ammoToLoad = ammoNeeded;
        if (weapon.totalAmmo < ammoToLoad) {
            ammoToLoad = weapon.totalAmmo;
        }
        weapon.currentAmmo += ammoToLoad;
        weapon.totalAmmo -= ammoToLoad;
        weapon.isReloading = false;
    }
} // namespace game
