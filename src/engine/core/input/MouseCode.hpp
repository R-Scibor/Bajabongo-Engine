#pragma once

namespace engine {

    /**
     * @brief Identifiers for mouse buttons.
     */
    enum class MouseCode {
        Left,       ///< The primary mouse button (usually left).
        Right,      ///< The secondary mouse button (usually right).
        Middle,     ///< The middle mouse button (scroll wheel click).

        /**
         * @brief First extra mouse button.
         * * Often mapped to "Back" in web browsers. Side button 1.
         */
        XButton1,

        /**
         * @brief Second extra mouse button.
         * * Often mapped to "Forward" in web browsers. Side button 2.
         */
        XButton2,

        /**
         * @brief Total count of supported mouse buttons.
         * * Useful for sizing arrays or iterating over buttons.
         */
        ButtonCount
    };

} // namespace engine