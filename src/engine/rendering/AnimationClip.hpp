#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

namespace engine {

    /**
     * @brief Defines a sequence of sprites that make up an animation.
     */
    struct AnimationClip {
        std::string name;
        std::vector<std::string> spriteIds; // Sequence of sprite IDs to play
        float frameDuration = 0.1f;         // Duration of each frame in seconds
        bool loop = true;                   // Whether the animation should loop
    };

    /**
     * @brief Manages a collection of animation clips.
     */
    class AnimationLibrary {
    public:
        void registerClip(const std::string& name, const AnimationClip& clip) {
            m_clips[name] = clip;
        }

        const AnimationClip* getClip(const std::string& name) const {
            auto it = m_clips.find(name);
            if (it != m_clips.end()) {
                return &it->second;
            }
            return nullptr;
        }

    private:
        std::unordered_map<std::string, AnimationClip> m_clips;
    };

} // namespace engine
