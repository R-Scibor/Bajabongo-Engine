#pragma once

namespace engine {

    /**
     * @brief Platform-independent representation of keyboard keys.
     *
     * This enum abstracts specific hardware or library key codes (e.g., SFML, GLFW)
     * into a uniform engine-level format. It covers standard US-layout keyboards.
     */
    enum class KeyCode {
        Unknown, ///< Unhandled or unrecognized key.

        // --- Letters ---
        A, B, C, D, E, F, G, H, I, J, K, L, M,
        N, O, P, Q, R, S, T, U, V, W, X, Y, Z,

        // --- Top-row Numbers ---
        Num0, Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9,

        // --- Control & System ---
        Escape,
        LControl, LShift, LAlt,
        LSystem,  ///< Left System key (Windows key on PC, Command on Mac, Super on Linux).
        RControl, RShift, RAlt,
        RSystem,  ///< Right System key.
        Menu,     ///< The Menu key (usually to the right of the Spacebar).

        // --- Brackets & Punctuation ---
        LBracket, ///< [
        RBracket, ///< ]
        Semicolon, ///< ;
        Comma,    ///< ,
        Period,   ///< .
        Quote,    ///< '
        Slash,    ///< /
        Backslash,///< \ (Backslash)
        Tilde,    ///< ~
        Equal,    ///< =
        Hyphen,   ///< -

        // --- Whitespace & Editing ---
        Space,
        Enter,    ///< Return / Enter key.
        Backspace,
        Tab,

        // --- Navigation ---
        PageUp, PageDown,
        End, Home,
        Insert, Delete,

        // --- Arithmetic ---
        Add,      ///< +
        Subtract, ///< - (Numpad or standard)
        Multiply, ///< *
        Divide,   ///< /

        // --- Arrows ---
        Left, Right, Up, Down,

        // --- Numpad (Numeric Keypad) ---
        Numpad0, Numpad1, Numpad2, Numpad3, Numpad4,
        Numpad5, Numpad6, Numpad7, Numpad8, Numpad9,

        // --- Function Keys ---
        F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12, F13, F14, F15,

        Pause ///< Pause/Break key.
    };

} // namespace engine