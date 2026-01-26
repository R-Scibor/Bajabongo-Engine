#pragma once
#include <string>
#include <entt/entt.hpp>
#include <optional>
#include "engine/core/math/MathAliases.hpp"

namespace engine {

    /**
     * @brief Event to request a one-shot sound effect playback.
     * * Can be global (2D) or spatial (positioned in the world).
     * * Handled by the AudioSystem.
     */
    struct PlaySoundEvent {
        /// @brief Resource ID of the sound buffer (e.g., "gunshot_01").
        std::string id;
        
        /// @brief Playback volume (0.0 to 100.0).
        float volume = 100.0f;
        
        /// @brief Pitch multiplier (1.0 = normal speed/pitch).
        float pitch = 1.0f;
        
        /// @brief Whether the sound should repeat indefinitely until stopped.
        bool loop = false;
        
        /**
         * @brief Optional owner entity.
         * * Required if you want to stop this specific sound later (via StopSoundEvent)
         * or if the sound should move with the entity (if spatial audio implementation supports tracking).
         */
        entt::entity sourceEntity = entt::null; 
        
        // --- Spatial Audio Parameters ---

        /**
         * @brief World position for the sound source.
         * * If std::nullopt, the sound is played as a generic 2D sound (e.g., UI click).
         * * If set, the volume/pan is calculated based on listener distance.
         */
        std::optional<Vector2f> position = std::nullopt; 
        
        /// @brief The distance within which the sound is at maximum volume.
        float minDistance = 300.0f;  
        
        /// @brief Rolloff factor. Higher values make the sound drop off more sharply with distance.
        float attenuation = 1.0f;    
    };

    /**
     * @brief Event to stop a specifically looping sound attached to an entity.
     */
    struct StopSoundEvent {
        /// @brief The entity that "owns" the playing sound instance.
        entt::entity sourceEntity;
        
        /// @brief Time in seconds to fade out before stopping (0.0 = instant cut).
        float fadeOutDuration = 0.0f; 
    };

    /**
     * @brief Event to change the background music track.
     * * Automatically handles crossfading from the previous track.
     */
    struct PlayMusicEvent {
        /// @brief Resource ID or file path for the music stream.
        std::string id;
        
        /// @brief Target volume (0.0 to 100.0).
        float volume = 100.0f;
        
        /// @brief Whether the track should restart when it ends.
        bool loop = true;
        
        /// @brief Duration of the crossfade transition in seconds (0.0 = instant switch).
        float crossfadeDuration = 0.0f; 
    };

    /**
     * @brief Event to stop the currently playing music.
     */
    struct StopMusicEvent {
        /// @brief Time in seconds to fade out to silence.
        float fadeOutDuration = 0.0f;
    };

    /**
     * @brief Categorization for audio mixer channels.
     */
    enum class AudioCategory {
        Master, ///< Global volume affecting everything.
        Music,  ///< Background music volume.
        SFX     ///< Sound effects volume.
    };

    /**
     * @brief Event to update global volume settings.
     * * Useful for Options Menu sliders.
     */
    struct SetVolumeEvent {
        AudioCategory category;
        float volume; // 0.0 to 100.0
    };

    /**
     * @brief Notification dispatched when a non-looping music track ends naturally.
     * * Useful for playlists or triggering gameplay events after a song.
     */
    struct MusicFinishedEvent {
        std::string id;
    };

} // namespace engine