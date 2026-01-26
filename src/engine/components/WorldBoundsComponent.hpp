#pragma once

namespace engine {
    /**
     * @brief Component defining the boundaries of the game world or current level.
     * * Often used as a singleton or attached to a specific 'LevelSettings' entity.
     * Systems use this to clamp camera movement or destroy entities that leave the map.
     */
    struct WorldBoundsComponent {
        float width = 0.0f;  ///< Total width of the world in world units.
        float height = 0.0f; ///< Total height of the world in world units.
    };
}