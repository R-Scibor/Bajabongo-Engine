#pragma once

namespace game {

    struct HealthComponent {
        float currentHp = 100.0f;
        float maxHp = 100.0f;
    };

    /// Subtracts damage from current HP. Does not clamp; defeat is currentHp <= 0.
    inline void applyDamage(HealthComponent& health, float damageValue) {
        health.currentHp -= damageValue;
    }

    inline bool isDefeated(const HealthComponent& health) {
        return health.currentHp <= 0.0f;
    }

} // namespace game
