#pragma once

#include <vector>
#include <memory>
#include <string>
#include <functional>
#include <map>

#include "engine/core/IGameState.hpp"
#include "engine/events/StateEvents.hpp"

// Forward declarations
namespace sf {
    class Event;
}
namespace engine {
    struct EngineContext;
}

namespace engine {

    /**
     * @brief Manages a stack of IGameStates to control the flow of the application.
     *
     * This class implements a Push-Down Automaton (Stack Machine).
     * It uses a **Deferred Transition System**:
     * State changes (push, pop, swap) are requested and queued, but strictly executed
     * at a safe point in the main loop (`processTransitions()`).
     * This prevents crashes caused by a state trying to delete itself while its `update()` method is still running.
     */
    class StateManager {
    public:
        explicit StateManager(EngineContext& context);
        ~StateManager();

        // --- Direct Transition Requests ---

        /**
         * @brief Queues a request to push a new state onto the stack.
         * * The new state will be created using the registered factory function.
         * * It will become the active state (receiving input/updates) on the next frame.
         * @param stateName The ID of the state to create (must be registered first).
         */
        void requestPush(const std::string& stateName);

        /**
         * @brief Queues a request to remove the current top state.
         */
        void requestPop();

        /**
         * @brief Queues a request to replace the current state with a new one.
         * * Equivalent to Pop() followed immediately by Push().
         * @param stateName The ID of the new state.
         */
        void requestSwap(const std::string& stateName);

        /**
         * @brief Queues a request to remove ALL states from the stack.
         */
        void requestClear();

        // --- Game Loop Integration ---

        /**
         * @brief Forwards a system event to the active (top) state.
         */
        void handleEvent(const sf::Event& event);

        /**
         * @brief Updates the active (top) state.
         * @param fixedDeltaTime Fixed time step for physics/logic.
         */
        void update(float fixedDeltaTime);

        /**
         * @brief Renders all states from bottom to top.
         * * Allows for transparent overlays (e.g., Pause Menu over Gameplay).
         */
        void render();

        /**
         * @brief Processes all pending state transitions.
         * * WARNING: This should only be called by the Application at the end of the frame.
         * * Creates/destroys state objects based on the queued requests.
         */
        void processTransitions();

        /**
         * @brief Checks if the state stack is empty.
         * @return True if there are no active states (application should probably close).
         */
        bool isEmpty() const;

        /**
         * @brief Registers a state class with the internal factory.
         * * Must be called during engine initialization (e.g., in `Game::init`).
         *
         * @tparam T The concrete state class (must inherit from IGameState).
         * @param stateName The unique string identifier for this state (e.g., "MainMenu").
         */
        template<typename T>
        void registerState(const std::string& stateName) {
            static_assert(std::is_base_of_v<IGameState, T>, "T must be a descendant of IGameState");
            static_assert(std::is_constructible_v<T, EngineContext&> || std::is_constructible_v<T>,
                "State must be constructible with EngineContext& or default constructible.");

            // Lamda that creates a new instance of T when invoked
            m_stateFactory[stateName] = [this]() -> std::unique_ptr<IGameState> {
                if constexpr (std::is_constructible_v<T, EngineContext&>) {
                    return std::make_unique<T>(m_context);
                } else {
                    return std::make_unique<T>();
                }
            };
        }

    private:
        // --- Event Handlers for dispatcher requests ---
        void onPushRequest(const RequestStatePushEvent& event);
        void onPopRequest(const RequestStatePopEvent& event);
        void onSwapRequest(const RequestStateSwapEvent& event);
        void onClearRequest(const RequestStateClearEvent& event);

        // --- Internal transition action representation ---
        enum class PendingActionType { Push, Pop, Swap, Clear };
        
        struct PendingAction {
            PendingActionType type;
            std::string stateName; // Only used for Push and Swap
        };

        EngineContext& m_context;
        
        /// @brief The stack of active states. Index 0 is bottom, back() is active.
        std::vector<std::unique_ptr<IGameState>> m_states; 
        
        /// @brief Queue of transitions to apply at the end of the frame.
        std::vector<PendingAction> m_pendingActions;
        
        /// @brief Map of "StateName" -> "Function that creates this State".
        std::map<std::string, std::function<std::unique_ptr<IGameState>()>> m_stateFactory;
    };

} // namespace engine