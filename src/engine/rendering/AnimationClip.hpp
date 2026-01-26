#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

namespace engine {

    /**
     * @brief Defines a single animation sequence.
     * * Acts as a resource or blueprint containing the list of sprite frames
     * and playback settings.
     */
    struct AnimationClip {
        /// @brief Unique name of the clip (e.g., "Player_Walk_Down").
        std::string name;
        
        /// @brief Ordered sequence of sprite resource IDs (keys into SpriteManager).
        std::vector<std::string> spriteIds; 
        
        /// @brief Duration of each frame in seconds. (e.g., 0.1f = 10 FPS).
        float frameDuration = 0.1f;         
        
        /// @brief Whether the animation should restart automatically when it ends.
        bool loop = true;                   
    };

    /**
     * @brief Resource container for storing and retrieving animation clips.
     * * Acts as a central repository so multiple entities can share the same animation data.
     */
    class AnimationLibrary {
    public:
        /**
         * @brief Registers a new animation clip.
         * @param name The unique identifier for the clip.
         * @param clip The animation data structure.
         */
        void registerClip(const std::string& name, const AnimationClip& clip) {
            m_clips[name] = clip;
        }

        /**
         * @brief Retrieves a pointer to a stored clip.
         * @param name The unique identifier.
         * @return Const pointer to the clip, or nullptr if not found.
         */
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