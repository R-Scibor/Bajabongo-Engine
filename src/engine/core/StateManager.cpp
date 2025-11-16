#include "engine/pch.h"
#include "StateManager.hpp"
#include "engine/core/EngineContext.hpp"
#include "engine/core/ILoggerManager.hpp"
#include <entt/entt.hpp>
#include "engine/core/ILogger.hpp"

namespace engine {

    StateManager::StateManager(EngineContext& context) : m_context(context) {
        auto logger = context.m_logManager->GetLogger("StateManager");
        logger->info("Initializing StateManager and connecting to dispatcher.");

        // Subscribe to all state-related events from the dispatcher
        m_context.m_dispatcher->sink<RequestStatePushEvent>().connect<&StateManager::onPushRequest>(this);
        m_context.m_dispatcher->sink<RequestStatePopEvent>().connect<&StateManager::onPopRequest>(this);
        m_context.m_dispatcher->sink<RequestStateSwapEvent>().connect<&StateManager::onSwapRequest>(this);
        m_context.m_dispatcher->sink<RequestStateClearEvent>().connect<&StateManager::onClearRequest>(this);
    }

    StateManager::~StateManager() {
        auto logger = m_context.m_logManager->GetLogger("StateManager");
        logger->info("Shutting down StateManager and disconnecting from dispatcher.");

        // It's crucial to disconnect to avoid dangling pointers if the dispatcher outlives the manager
        m_context.m_dispatcher->sink<RequestStatePushEvent>().disconnect(this);
        m_context.m_dispatcher->sink<RequestStatePopEvent>().disconnect(this);
        m_context.m_dispatcher->sink<RequestStateSwapEvent>().disconnect(this);
        m_context.m_dispatcher->sink<RequestStateClearEvent>().disconnect(this);
    }

    void StateManager::requestPush(const std::string& stateName) {
        m_pendingActions.push_back({ PendingActionType::Push, stateName });
    }

    void StateManager::requestPop() {
        m_pendingActions.push_back({ PendingActionType::Pop });
    }

    void StateManager::requestSwap(const std::string& stateName) {
        m_pendingActions.push_back({ PendingActionType::Swap, stateName });
    }

    void StateManager::requestClear() {
        m_pendingActions.push_back({ PendingActionType::Clear });
    }

    void StateManager::onPushRequest(const RequestStatePushEvent& event) {
        requestPush(event.stateName);
    }

    void StateManager::onPopRequest(const RequestStatePopEvent& event) {
        requestPop();
    }

    void StateManager::onSwapRequest(const RequestStateSwapEvent& event) {
        requestSwap(event.stateName);
    }

    void StateManager::onClearRequest(const RequestStateClearEvent& event) {
        requestClear();
    }

    void StateManager::handleEvent(const sf::Event& event) {
        if (!m_states.empty()) {
            m_states.back()->handleEvent(m_context, event);
        }
    }

    void StateManager::update(float fixedDeltaTime) {
        if (!m_states.empty()) {
            m_states.back()->update(m_context, fixedDeltaTime);
        }
    }

    void StateManager::render() {
        for (auto& state : m_states) {
            state->render(m_context);
        }
    }

    void StateManager::processTransitions() {
        auto logger = m_context.m_logManager->GetLogger("StateManager");

        // Take a copy of the pending actions and clear the original queue.
        // This prevents new transitions requested during onEnter/onExit from
        // interfering with the current batch.
        auto actionsToProcess = std::move(m_pendingActions);
        m_pendingActions.clear();

        for (const auto& action : actionsToProcess) {
            switch (action.type) {
            case PendingActionType::Push: {
                if (m_stateFactory.count(action.stateName)) {
                    auto newState = m_stateFactory.at(action.stateName)();
                    logger->info("Pushing state: {}", action.stateName);
                    newState->onEnter(m_context);
                    m_states.push_back(std::move(newState));
                } else {
                    logger->error("Unknown state requested for push: {}", action.stateName);
                }
                break;
            }

            case PendingActionType::Pop: {
                if (!m_states.empty()) {
                    logger->info("Popping state.");
                    m_states.back()->onExit(m_context);
                    m_states.pop_back();
                    if (m_states.empty()) {
                        logger->warn("State stack is empty. Application may need to close.");
                    }
                } else {
                    logger->warn("Request to pop from an empty state stack.");
                }
                break;
            }

            case PendingActionType::Swap: {
                if (m_stateFactory.count(action.stateName)) {
                    // Pop current state first
                    if (!m_states.empty()) {
                        logger->info("Swapping state.");
                        m_states.back()->onExit(m_context);
                        m_states.pop_back();
                    }

                    // Then push the new one
                    auto newState = m_stateFactory.at(action.stateName)();
                    logger->info("Pushing swapped state: {}", action.stateName);
                    newState->onEnter(m_context);
                    m_states.push_back(std::move(newState));
                } else {
                    logger->error("Unknown state requested for swap: {}", action.stateName);
                }
                break;
            }

            case PendingActionType::Clear: {
                logger->info("Clearing all states.");
                while (!m_states.empty()) {
                    m_states.back()->onExit(m_context);
                    m_states.pop_back();
                }
                logger->warn("State stack is empty. Application may need to close.");
                break;
            }
            }
        }
    }

} // namespace engine