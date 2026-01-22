#pragma once

#include <vector>
#include <memory>
#include <string>
#include <SFML/Audio.hpp>
#include <entt/entt.hpp>
#include "engine/core/IResourceManager.hpp"
#include "engine/events/AudioEvents.hpp"
#include "engine/core/ILogger.hpp"

namespace engine {

    class AudioSystem {
    public:
        AudioSystem(std::shared_ptr<IResourceManager> resourceManager,
                    std::shared_ptr<entt::dispatcher> dispatcher,
                    std::shared_ptr<ILogger> logger);
        ~AudioSystem();

        void update(float dt);

        std::string getCurrentMusicId() const { return m_currentMusicId; }

    private:
        void onPlaySound(const PlaySoundEvent& event);
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

        std::vector<entt::scoped_connection> m_connections;
    };

} // namespace engine
