#include "engine/pch.h"
#include "AudioSystem.hpp"
#include <algorithm>
#include "engine/components/TransformComponent.hpp"

namespace engine {

    AudioSystem::AudioSystem(std::shared_ptr<IResourceManager> resourceManager,
                             std::shared_ptr<entt::dispatcher> dispatcher,
                             std::shared_ptr<ILogger> logger)
        : m_resourceManager(std::move(resourceManager))
        , m_dispatcher(std::move(dispatcher))
        , m_logger(std::move(logger))
    {
        // Initialize pool
        m_soundPool.reserve(MAX_SOUNDS);
        for (size_t i = 0; i < MAX_SOUNDS; ++i) {
            m_soundPool.push_back(std::make_unique<sf::Sound>(m_dummyBuffer));
        }

        // Subscribe to events
        m_connections.push_back(m_dispatcher->sink<PlaySoundEvent>().connect<&AudioSystem::onPlaySound>(this));
        m_connections.push_back(m_dispatcher->sink<StopSoundEvent>().connect<&AudioSystem::onStopSound>(this));
        m_connections.push_back(m_dispatcher->sink<PlayMusicEvent>().connect<&AudioSystem::onPlayMusic>(this));
        m_connections.push_back(m_dispatcher->sink<StopMusicEvent>().connect<&AudioSystem::onStopMusic>(this));
        m_connections.push_back(m_dispatcher->sink<SetVolumeEvent>().connect<&AudioSystem::onSetVolume>(this));

        if (m_logger) {
            m_logger->info("AudioSystem initialized with {} sound sources.", MAX_SOUNDS);
        }
    }

    AudioSystem::~AudioSystem() {
        // Stop everything
        if (m_music) {
            m_music->stop();
        }
        for (auto& sound : m_soundPool) {
            if (sound) sound->stop();
        }
    }

    void AudioSystem::setListenerPosition(const Vector2f& position) {
        m_listenerPosition = position;
        sf::Listener::setPosition({position.x, position.y, 0.0f});
    }

    void AudioSystem::update(float dt, entt::registry* registry) {
        // Handle music fading
        if (m_isFading && m_music) {
            m_fadeTimer += dt;
            if (m_fadeTimer >= m_fadeDuration) {
                m_isFading = false;
                m_music->setVolume(m_targetFadeVolume);
                if (m_targetFadeVolume <= 0.01f) {
                    m_music->stop();
                    m_currentMusicId.clear();
                }
            } else {
                float t = m_fadeTimer / m_fadeDuration;
                // Linear fade
                float newVol = m_startFadeVolume + (m_targetFadeVolume - m_startFadeVolume) * t;
                m_music->setVolume(newVol);
            }
        }

        // Check for natural finish
        if (m_music && m_music->getStatus() == sf::SoundSource::Status::Stopped && !m_currentMusicId.empty()) {
            std::string finishedId = m_currentMusicId;
            m_currentMusicId.clear(); // Prevent multiple events
            m_dispatcher->enqueue<MusicFinishedEvent>({ finishedId });
            if (m_logger) m_logger->info("Music finished naturally: {}", finishedId);
        }

        // Clean up tracking map for stopped sounds
        for (auto it = m_loopingSounds.begin(); it != m_loopingSounds.end(); ) {
            size_t index = it->second;
            if (index < m_soundPool.size() && m_soundPool[index]->getStatus() == sf::SoundSource::Status::Stopped) {
                it = m_loopingSounds.erase(it);
            } else {
                // Update position for looping sounds attached to entities
                if (registry && registry->valid(it->first)) {
                    if (auto* transform = registry->try_get<TransformComponent>(it->first)) {
                        m_soundPool[index]->setPosition({transform->position.x, transform->position.y, 0.0f});
                    }
                }
                ++it;
            }
        }
    }

    void AudioSystem::onPlaySound(const PlaySoundEvent& event) {
        // Check if we are already looping this sound for this entity
        if (event.loop && event.sourceEntity != entt::null) {
            auto it = m_loopingSounds.find(event.sourceEntity);
            if (it != m_loopingSounds.end()) {
                size_t index = it->second;
                // Verify the sound is still valid and playing
                if (index < m_soundPool.size() && m_soundPool[index]->getStatus() == sf::SoundSource::Status::Playing) {
                    // Update parameters if needed? For now just ignore duplicate start
                    return;
                } else {
                    // It stopped, so remove from map and start fresh
                    m_loopingSounds.erase(it);
                }
            }
        }

        auto buffer = m_resourceManager->getSoundBuffer(event.id);
        if (!buffer) {
             if (m_logger) m_logger->warn("AudioSystem: Sound buffer not found: {}", event.id);
             return;
        }

        // Find free sound source
        sf::Sound* candidate = nullptr;
        size_t chosenIndex = 0;

        for (size_t i = 0; i < MAX_SOUNDS; ++i) {
            if (m_soundPool[i]->getStatus() == sf::SoundSource::Status::Stopped) {
                candidate = m_soundPool[i].get();
                chosenIndex = i;
                break;
            }
        }

        if (!candidate) {
            // Pool full, steal next index (round robin)
            chosenIndex = m_nextSoundIndex;
            candidate = m_soundPool[chosenIndex].get();
            m_nextSoundIndex = (m_nextSoundIndex + 1) % MAX_SOUNDS;
            
            // If we stole a sound, check if it was being tracked as a loop and remove it
            for (auto it = m_loopingSounds.begin(); it != m_loopingSounds.end(); ) {
                if (it->second == chosenIndex) {
                    it = m_loopingSounds.erase(it);
                } else {
                    ++it;
                }
            }
        }

        candidate->setBuffer(*buffer);
        float vol = event.volume * (getEffectiveVolume(AudioCategory::SFX) / 100.0f);
        candidate->setVolume(vol);
        candidate->setPitch(event.pitch);
        candidate->setLooping(event.loop);

        if (event.position.has_value()) {
            candidate->setPosition({event.position->x, event.position->y, 0.0f});
            candidate->setRelativeToListener(false);
            candidate->setMinDistance(event.minDistance);
            candidate->setAttenuation(event.attenuation);
        } else {
            candidate->setRelativeToListener(true);
            candidate->setPosition({0.0f, 0.0f, 0.0f});
            candidate->setMinDistance(1.0f);
            candidate->setAttenuation(1.0f);
        }

        candidate->play();

        // Track looping sounds
        if (event.loop && event.sourceEntity != entt::null) {
            m_loopingSounds[event.sourceEntity] = chosenIndex;
        }
    }

    void AudioSystem::onStopSound(const StopSoundEvent& event) {
        if (event.sourceEntity == entt::null) return;

        auto it = m_loopingSounds.find(event.sourceEntity);
        if (it != m_loopingSounds.end()) {
            size_t index = it->second;
            if (index < m_soundPool.size()) {
                m_soundPool[index]->stop();
            }
            m_loopingSounds.erase(it);
        }
    }

    void AudioSystem::onPlayMusic(const PlayMusicEvent& event) {
        std::string path = m_resourceManager->getMusicPath(event.id);
        if (path.empty()) {
            if (m_logger) m_logger->warn("AudioSystem: Music path not found for id: {}", event.id);
            return;
        }

        if (m_music && m_music->getStatus() == sf::SoundSource::Status::Playing && m_currentMusicId == event.id) {
            // Already playing this track
            return;
        }

        // Setup new music
        auto newMusic = std::make_unique<sf::Music>();
        if (!newMusic->openFromFile(path)) {
            if (m_logger) m_logger->error("AudioSystem: Failed to open music file: {}", path);
            return;
        }

        float targetVol = event.volume * (getEffectiveVolume(AudioCategory::Music) / 100.0f);

        if (event.crossfadeDuration > 0.0f) {
            // If we have existing music, stop it (for now, single channel)
            if (m_music) m_music->stop();
            
            m_music = std::move(newMusic);
            m_music->setLooping(event.loop);
            m_music->setVolume(0.0f); // Start at 0
            m_music->play();
            
            // Start fade in
            m_isFading = true;
            m_fadeTimer = 0.0f;
            m_fadeDuration = event.crossfadeDuration;
            m_startFadeVolume = 0.0f;
            m_targetFadeVolume = targetVol;
            
        } else {
            if (m_music) m_music->stop();
            m_music = std::move(newMusic);
            m_music->setLooping(event.loop);
            m_music->setVolume(targetVol);
            m_music->play();
            m_isFading = false;
        }
        
        m_currentMusicId = event.id;
        if (m_logger) m_logger->info("AudioSystem: Playing music: {}", event.id);
    }

    void AudioSystem::onStopMusic(const StopMusicEvent& event) {
        if (!m_music || m_music->getStatus() == sf::SoundSource::Status::Stopped) return;

        if (event.fadeOutDuration > 0.0f) {
            m_isFading = true;
            m_fadeTimer = 0.0f;
            m_fadeDuration = event.fadeOutDuration;
            m_startFadeVolume = m_music->getVolume();
            m_targetFadeVolume = 0.0f;
        } else {
            m_music->stop();
            m_isFading = false;
            m_currentMusicId.clear();
        }
    }

    void AudioSystem::onSetVolume(const SetVolumeEvent& event) {
        float* targetVar = nullptr;

        switch (event.category) {
            case AudioCategory::Master: targetVar = &m_masterVolume; break;
            case AudioCategory::Music: targetVar = &m_musicVolume; break;
            case AudioCategory::SFX: targetVar = &m_sfxVolume; break;
        }

        if (targetVar) {
            *targetVar = std::clamp(event.volume, 0.0f, 100.0f);
        }
        
        // Update currently playing music immediately
        if (m_music && !m_isFading) {
             float vol = getEffectiveVolume(AudioCategory::Music);
             m_music->setVolume(vol);
        }
        
        if (m_logger) {
            m_logger->info("AudioSystem: Volume set for category {} to {}", (int)event.category, event.volume);
        }
    }

    float AudioSystem::getEffectiveVolume(AudioCategory category) const {
        switch (category) {
            case AudioCategory::Master: return m_masterVolume;
            case AudioCategory::Music: return m_musicVolume * (m_masterVolume / 100.0f);
            case AudioCategory::SFX: return m_sfxVolume * (m_masterVolume / 100.0f);
        }
        return 0.0f;
    }

} // namespace engine
