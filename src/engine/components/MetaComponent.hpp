#pragma once
#include <string>

namespace engine {

    /**
     * @brief Component holding metadata and debugging information.
     * * This component allows assigning human-readable names to entities, which is
     * essential for level editors, logs, and debugging tools. It does not affect gameplay logic.
     */
    struct MetaComponent {
        /// @brief The human-readable name of the entity (e.g., "Player", "Enemy_01").
        std::string name;
    };

} // namespace engine