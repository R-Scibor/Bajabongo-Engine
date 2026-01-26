#pragma once

#include <string>

namespace engine {

    /**
     * @brief Event published to request pushing a new state onto the stack.
     * * The StateManager listens for this and queues a 'Push' action to be executed
     * at the end of the frame.
     */
    struct RequestStatePushEvent {
        /// @brief The ID of the state to create (must be registered in StateManager).
        std::string stateName;
    };

    /**
     * @brief Event published to request popping the top-most state.
     * * The StateManager listens for this and queues a 'Pop' action.
     * Use this to close the current menu or pause screen.
     */
    struct RequestStatePopEvent {
        // No data needed
    };

    /**
     * @brief Event published to request swapping the current state with a new one.
     * * This acts as an atomic "Pop current + Push new".
     * Useful for transitioning between distinct game modes (e.g., MainMenu -> Gameplay).
     */
    struct RequestStateSwapEvent {
        /// @brief The ID of the new state to switch to.
        std::string stateName;
    };

    /**
     * @brief Event published to request clearing the entire state stack.
     * * Useful for "Quit to Desktop" or hard resets where you want to ensure 
     * no previous states remain in memory.
     */
    struct RequestStateClearEvent {
        // No data needed
    };

} // namespace engine