#pragma once

// Forward declarations to avoid including heavy headers
namespace sf {
    class Event;
}
namespace engine {
    struct EngineContext;
}

namespace engine {

    /**
     * @brief Abstract interface for a single game state.
     *
     * Game states represent distinct parts of the game, such as the main menu,
     * gameplay, pause screen, or credits. The StateManager manages a stack of
     * these states.
     */
    class IGameState {
    public:
        virtual ~IGameState() = default;

        /**
         * @brief Called once when the state is pushed onto the stack and becomes active.
         * Use this to initialize resources, set up UI, etc.
         * @param context A reference to the shared engine context.
         */
        virtual void onEnter(EngineContext& context) = 0;

        /**
         * @brief Called once when the state is popped from the stack.
         * Use this to clean up resources.
         * @param context A reference to the shared engine context.
         */
        virtual void onExit(EngineContext& context) = 0;

        /**
         * @brief Handles SFML events for the active state.
         * @param context A reference to the shared engine context.
         * @param event The SFML event to process.
         */
        virtual void handleEvent(EngineContext& context, const sf::Event& event) = 0;

        /**
         * @brief Updates the state's logic with a fixed time step.
         * @param context A reference to the shared engine context.
         * @param fixedDeltaTime The fixed time step for physics and game logic.
         */
        virtual void update(EngineContext& context, float fixedDeltaTime) = 0;

        /**
         * @brief Renders the state.
         * States lower in the stack are rendered before states higher up.
         * @param context A reference to the shared engine context.
         */
        virtual void render(EngineContext& context) = 0;
    };

} // namespace engine