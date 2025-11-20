#pragma once
#include <string>

namespace engine {

    /**
     * @brief Component that holds the state of an active animation.
     */
    struct AnimationComponent {
        std::string currentClipId;
        float timer = 0.0f;
        int currentFrameIndex = 0;
        bool isPlaying = true;
        bool isFinished = false;

        void reset() {
            timer = 0.0f;
            currentFrameIndex = 0;
            isFinished = false;
            isPlaying = true;
        }
    };

} // namespace engine
