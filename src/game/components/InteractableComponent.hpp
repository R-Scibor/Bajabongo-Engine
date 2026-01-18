#pragma once

namespace game {

    struct InteractableComponent {
        enum class Type {
            MissionTable
        };

        Type type = Type::MissionTable;
        float interactionRadius = 100.0f;
    };

} // namespace game
