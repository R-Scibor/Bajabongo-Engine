#pragma once

namespace game
{
    // Tag component: Entity is currently in at least one player's vision cone
    struct VisibleToPlayerComponent
    {
        bool wasVisibleLastFrame = false;  // For "last seen" logic later
        bool alwaysVisible = false;        // If true, overrides visibility check
    };
}
