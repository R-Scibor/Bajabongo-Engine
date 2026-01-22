#pragma once
#include <string>
#include <entt/entt.hpp>

namespace engine {

    // One-shot sound event (gunshot, click, etc.)
    struct PlaySoundEvent {
        std::string id;
        float volume = 100.0f;
        float pitch = 1.0f;
        bool loop = false;
        entt::entity sourceEntity = entt::null; // Optional: Entity that produced the sound (for looping control)
    };

    // Stop a looping sound associated with an entity
    struct StopSoundEvent {
        entt::entity sourceEntity;
        float fadeOutDuration = 0.0f; // Optional fade out (not yet implemented fully for SFX, but good to have)
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
