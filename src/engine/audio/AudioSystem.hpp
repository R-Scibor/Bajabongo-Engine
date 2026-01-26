/**
 * @file AudioSystem.hpp
 * @brief Defines the AudioSystem class, responsible for managing sound effects and music.
 */

#pragma once

#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include <SFML/Audio.hpp>
#include <entt/entt.hpp>
#include "engine/core/IResourceManager.hpp"
#include "engine/events/AudioEvents.hpp"
#include "engine/core/ILogger.hpp"
#include "engine/core/math/MathAliases.hpp"

namespace engine {

    /**
     * @brief Manages audio playback, including sound effects and background music.
     *
     * Handles sound pooling, spatial audio (listener position), volume control,
     * and music cross-fading. Listens for audio events from the event dispatcher.
     */
    class AudioSystem {
    public:
        /**
         * @brief Constructs the AudioSystem.
         * @param resourceManager Shared pointer to the resource manager (for loading audio buffers).
         * @param dispatcher Shared pointer to the event dispatcher (for receiving audio events).
         * @param logger Shared pointer to the logger.
         */
        AudioSystem(std::shared_ptr<IResourceManager> resourceManager,
                    std::shared_ptr<entt::dispatcher> dispatcher,
                    std::shared_ptr<ILogger> logger);
        
        ~AudioSystem();

        /**
         * @brief Updates the audio system state.
         *
         * Handles music fading and cleans up stopped sounds.
         *
         * @param dt Delta time since the last frame.
         * @param registry Optional pointer to the ECS registry (used for updating spatial sound positions).
         */
        void update(float dt, entt::registry* registry = nullptr);

        /**
         * @brief Sets the global listener position for spatial audio.
         * @param position The position of the listener (usually the camera or player center).
         */
        void setListenerPosition(const Vector2f& position);

        /**
         * @brief Gets the ID of the currently playing music track.
         * @return The string ID of the current music, or empty string if none.
         */
        std::string getCurrentMusicId() const { return m_currentMusicId; }

    private:
        // Event handlers
        void onPlaySound(const PlaySoundEvent& event);
        void onStopSound(const StopSoundEvent& event);
        void onPlayMusic(const PlayMusicEvent& event);
        void onStopMusic(const StopMusicEvent& event);
        void onSetVolume(const SetVolumeEvent& event);

        // Helpers
        float getEffectiveVolume(AudioCategory category) const;

        std::shared_ptr<IResourceManager> m_resourceManager;
        std::shared_ptr<entt::dispatcher> m_dispatcher;
        std::shared_ptr<ILogger> m_logger;

        // Dummy buffer for initialization (must be declared before pool for lifetime)
        sf::SoundBuffer m_dummyBuffer;

        // Sound Pool
        static constexpr size_t MAX_SOUNDS = 64;
        std::vector<std::unique_ptr<sf::Sound>> m_soundPool;
        size_t m_nextSoundIndex = 0;

        // Music
        std::unique_ptr<sf::Music> m_music;
        std::string m_currentMusicId;
        
        // Volume (0.0 - 100.0)
        float m_masterVolume = 100.0f;
        float m_musicVolume = 100.0f;
        float m_sfxVolume = 100.0f;

        // Fading Logic
        bool m_isFading = false;
        float m_fadeTimer = 0.0f;
        float m_fadeDuration = 0.0f;
        float m_targetFadeVolume = 0.0f;
        float m_startFadeVolume = 0.0f;

        // Active Looping Sounds (Entity -> Sound Pool Index)
        std::unordered_map<entt::entity, size_t> m_loopingSounds;

        Vector2f m_listenerPosition = {0.0f, 0.0f};

        std::vector<entt::scoped_connection> m_connections;
    };

} // namespace engine
