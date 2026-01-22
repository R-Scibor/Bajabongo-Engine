#pragma once
#include <string>

namespace engine {

    // One-shot sound event (gunshot, click, etc.)
    struct PlaySoundEvent {
        std::string id;
        float volume = 100.0f;
        float pitch = 1.0f;
        bool loop = false;
    };

    // Background music event
    struct PlayMusicEvent {
        std::string id;
        float volume = 100.0f;
        bool loop = true;
        float crossfadeDuration = 0.0f; // 0 for instant
    };

    // Stop music event
    struct StopMusicEvent {
        float fadeOutDuration = 0.0f;
    };

    enum class AudioCategory {
        Master,
        Music,
        SFX
    };

    // Volume control event
    struct SetVolumeEvent {
        AudioCategory category;
        float volume; // 0.0 to 100.0
    };

    // Event dispatched when a music track finishes playing (if loop=false)
    struct MusicFinishedEvent {
        std::string id;
    };

} // namespace engine
