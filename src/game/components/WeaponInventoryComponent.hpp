#pragma once
#include <vector>
#include <string>

struct WeaponInventoryComponent {
    std::vector<std::string> weaponArchetypes; // e.g., ["Pistol", "Rifle", "Shotgun"]
    int activeSlot = 0;
    float switchCooldown = 0.0f;  // Prevent spam
    float switchDelay = 0.3f;     // Equip time
};
