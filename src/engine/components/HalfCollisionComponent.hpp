#pragma once

namespace engine {

    /**
     * @brief Tag component for entities that should block movement but allow projectiles.
     * 
     * Entities with this component will be filtered by the PhysicsSystem (or Game Logic)
     * to act as "low obstacles" or "shoot-through" walls.
     */
    struct HalfCollisionComponent {
        bool dummy = true; // Empty struct, just a tag
    };

    // Alias for LowObstacle for clarity in new code
    using LowObstacleComponent = HalfCollisionComponent;

}
