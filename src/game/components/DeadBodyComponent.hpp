#pragma once

namespace game {

    /// @brief Tag component to mark an entity as a dead body.
    /// Used to distinguish visual-only corpses from active entities.
    struct DeadBodyComponent {
        bool dummy = true; // Placeholder to ensure struct has size > 0 if needed, though empty structs are valid
    };

} // namespace game
