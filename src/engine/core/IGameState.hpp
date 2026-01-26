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
     * Game states represent distinct modes of the application, such as the Main Menu,
     * Gameplay, Pause Screen, or Credits. The `StateManager` manages a stack of
     * these states, allowing for overlays (e.g., Pause over Gameplay).
     */
    class IGameState {
    public:
        virtual ~IGameState() = default;

        /**
         * @brief Called once when the state is pushed onto the stack and becomes active.
         * * Use this to initialize resources, load level data, or set up UI elements.
         * @param context A reference to the shared engine context (access to systems).
         */
        virtual void onEnter(EngineContext& context) = 0;

        /**
         * @brief Called once when the state is popped from the stack.
         * * Use this to clean up resources unique to this state (e.g., unload textures).
         * @param context A reference to the shared engine context.
         */
        virtual void onExit(EngineContext& context) = 0;

        /**
         * @brief Handles SFML events for the active state.
         * * This is where you handle key presses, mouse clicks, or window events specific to this state.
         * @param context A reference to the shared engine context.
         * @param event The SFML event to process.
         */
        virtual void handleEvent(EngineContext& context, const sf::Event& event) = 0;

        /**
         * @brief Updates the state's logic with a fixed time step.
         * * Used for physics integration, AI, and deterministic game logic.
         * @param context A reference to the shared engine context.
         * @param fixedDeltaTime The fixed time step (usually 1/60th of a second).
         */
        virtual void update(EngineContext& context, float fixedDeltaTime) = 0;

        /**
         * @brief Renders the state to the screen.
         * * States are rendered from bottom to top of the stack (Painter's Algorithm).
         * @param context A reference to the shared engine context (access to Renderer).
         */
        virtual void render(EngineContext& context) = 0;
    };

} // namespace engine