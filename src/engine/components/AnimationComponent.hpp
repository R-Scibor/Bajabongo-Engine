#pragma once
#include <string>

namespace engine {

    /**
     * @brief Component that holds the runtime state of an active animation.
     * * This component tracks *how* the animation is playing (time, frame, status),
     * distinct from the logic state (which is handled by AnimationStateComponent).
     */
    struct AnimationComponent {
        /// @brief The identifier of the specific animation clip currently being played.
        std::string currentClipId;

        /// @brief Accumulator for time elapsed since the last frame switch (in seconds).
        float timer = 0.0f;

        /// @brief The index of the current frame within the animation clip.
        int currentFrameIndex = 0;

        /// @brief Flag indicating if the animation logic should update this frame.
        bool isPlaying = true;

        /// @brief Flag indicating if a non-looping animation has reached its end.
        bool isFinished = false;

        /**
         * @brief Resets the animation state to the beginning.
         * * Useful when switching to a new animation clip to ensure it starts
         * from the first frame with a zeroed timer.
         */
        void reset() {
            timer = 0.0f;
            currentFrameIndex = 0;
            isFinished = false;
            isPlaying = true;
        }
    };

} // namespace engine