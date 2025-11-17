Decouple state machine from SFML by introducing engine::InputEvent and translating from backend events in IInputManager.
Write event-based InputManager.
Write events for collision system in Box2d.
Animation System: [
        Complex animation state machines (idle/run/shoot/reload layers, blend trees).

        Directional sprite selection (N/S/E/W facing) beyond maybe a simple hack if needed.

        Networking/determinism concerns around animation (they can be purely cosmetic for now).

        Any coupling to gameplay logic (e.g., “fire on frame 3” events) — that belongs in a more advanced “Gameplay & Combat” phase.
    ]