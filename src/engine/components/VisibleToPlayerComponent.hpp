#pragma once

namespace engine
{
    // Tag component: entity is in at least one viewer's vision cone (or is marked always visible).
    struct VisibleToPlayerComponent
    {
        bool wasVisibleLastFrame = false;  // For "last seen" logic later
        bool alwaysVisible = false;        // If true, overrides visibility check
    };
}
