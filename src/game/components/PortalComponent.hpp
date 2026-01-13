#pragma once

namespace game {

    struct PortalComponent {
        bool isOpen = false;
        bool isLocked = false;
        // Optionally, we could add a key ID required to open it
        // int keyId = 0; 
    };

} // namespace game
