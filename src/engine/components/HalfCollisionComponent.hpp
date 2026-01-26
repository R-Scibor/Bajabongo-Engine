#pragma once

namespace engine {

    /**
     * @brief Tag component for entities that act as semi-solid obstacles.
     * * Entities with this component are typically treated as "low obstacles" (like sandbags or fences)
     * which block character movement but allow projectiles to pass through (shoot-through walls).
     * This logic is enforced by the PhysicsSystem or Game Logic filtering.
     */
    struct HalfCollisionComponent {
        /// @brief Dummy variable to ensure the struct has a non-zero size (standard C++ requirement), though mostly unused.
        bool dummy = true; 
    };

    /// @brief Alias for HalfCollisionComponent to improve code readability in gameplay logic.
    using LowObstacleComponent = HalfCollisionComponent;

}