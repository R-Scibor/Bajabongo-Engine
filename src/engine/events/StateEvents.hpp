#pragma once

#include <string>

namespace engine {

    /**
     * @brief Event published to request pushing a new state onto the stack.
     * The StateManager listens for this and adds a pending 'Push' action.
     * The state is identified by the string name it was registered with.
     */
    struct RequestStatePushEvent {
        std::string stateName;
    };

    /**
     * @brief Event published to request popping the top-most state.
     * The StateManager listens for this and adds a pending 'Pop' action.
     */
    struct RequestStatePopEvent {
        // No data needed
    };

    /**
     * @brief Event published to request swapping the current state
     * with a new one. This is an atomic 'Pop' then 'Push'.
     * The StateManager listens for this and adds a pending 'Swap' action.
     */
    struct RequestStateSwapEvent {
        std::string stateName;
    };

    /**
     * @brief Event published to request clearing the entire state stack.
     * The StateManager listens for this and adds a pending 'Clear' action.
     */
    struct RequestStateClearEvent {
        // No data needed
    };

} // namespace engine