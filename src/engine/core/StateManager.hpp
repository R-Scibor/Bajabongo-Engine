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
     * Implements a deferred transition system. State changes (push, pop, swap) are
     * requested and queued. They are only processed at a safe point in the main
     * loop when processTransitions() is called. This prevents states from being
     * modified while they are still executing.
     */
    class StateManager {
    public:
        explicit StateManager(EngineContext& context);
        ~StateManager();

        // --- Direct Transition Requests ---
        void requestPush(const std::string& stateName);
        void requestPop();
        void requestSwap(const std::string& stateName);
        void requestClear();

        // --- Game Loop Integration ---
        void handleEvent(const sf::Event& event);
        void update(float fixedDeltaTime);
        void render();

        /**
         * @brief Processes all pending state transitions.
         * This is the core of the deferred transition system.
         */
        void processTransitions();

        /**
         * @brief Checks if the state stack is empty.
         * @return True if there are no active states.
         */
        bool isEmpty() const;

        /**
         * @brief Registers a state with the factory.
         *
         * @tparam T The concrete state class (must inherit from IGameState).
         * @param stateName The unique string identifier for this state.
         */
        template<typename T>
        void registerState(const std::string& stateName) {
            static_assert(std::is_base_of_v<IGameState, T>, "T must be a descendant of IGameState");
            static_assert(std::is_constructible_v<T, EngineContext&> || std::is_constructible_v<T>,
                "State must be constructible with EngineContext& or default constructible.");

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
        std::vector<std::unique_ptr<IGameState>> m_states; // Defines the active state stack
        std::vector<PendingAction> m_pendingActions;
        std::map<std::string, std::function<std::unique_ptr<IGameState>()>> m_stateFactory;
    };

} // namespace engine