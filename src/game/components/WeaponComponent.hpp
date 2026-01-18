#pragma once

namespace game
{
    struct WeaponComponent
    {
        float cooldownTimer = 0.0f;
        float fireRate = 0.2f;      // Time between shots in seconds
        float projectileSpeed = 300.0f;
        float projectileLifetime = 2.0f;
        float damage = 10.0f;

        // Spread (shot randomness)
        float baseSpreadDeg = 0.0f;
        float currentSpreadDeg = 0.0f;  // Runtime bloom
        float spreadPerShotDeg = 0.0f;
        float spreadDecayDegPerSec = 0.0f;
        float maxSpreadDeg = 0.0f;

        // Recoil (aim kick)
        float recoilKickDeg = 0.0f;
        float recoilReturnDegPerSec = 0.0f;
        float recoilAngleOffset = 0.0f;  // Runtime offset

        // Multi-projectile (shotgun)
        int projectilesPerShot = 1;

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
}