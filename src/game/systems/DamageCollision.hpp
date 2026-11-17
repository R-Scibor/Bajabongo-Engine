#pragma once

#include "game/components/ProjectileComponent.hpp"

#include <entt/entity/entity.hpp>

namespace game {

    inline bool isProjectileOwnedByTarget(const ProjectileComponent* projectile, entt::entity target) {
        return projectile != nullptr && projectile->owner == target;
    }

    /// Solid (non-sensor) obstacles destroy a projectile that dealt no damage.
    inline bool shouldDestroyProjectileOnNonDamageHit(bool isTargetSensor) {
        return !isTargetSensor;
    }

    enum class ContactProjectile { None, EntityA, EntityB };

    /// Prefer entity A when both sides are projectiles, matching onContactBegin.
    inline ContactProjectile contactProjectileToDestroy(bool entityAIsProjectile, bool entityBIsProjectile) {
        if (entityAIsProjectile) {
            return ContactProjectile::EntityA;
        }
        if (entityBIsProjectile) {
            return ContactProjectile::EntityB;
        }
        return ContactProjectile::None;
    }

} // namespace game
