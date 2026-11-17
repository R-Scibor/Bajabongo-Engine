#pragma once

#include "engine/core/ILogger.hpp"
#include "engine/core/ILoggerManager.hpp"
#include "engine/core/IResourceManager.hpp"

#include <memory>
#include <string>
#include <unordered_map>

namespace baja::test {

    class NullLogger : public engine::ILogger {
    public:
        void log(spdlog::level::level_enum, std::string_view) override {}
    };

    class NullLoggerManager : public engine::ILoggerManager {
    public:
        std::shared_ptr<engine::ILogger> getLogger(const std::string&) override {
            if (!m_logger) {
                m_logger = std::make_shared<NullLogger>();
            }
            return m_logger;
        }

    private:
        std::shared_ptr<engine::ILogger> m_logger;
    };

    // Records load requests without reading files or opening a window.
    class FakeResourceManager : public engine::IResourceManager {
    public:
        std::unordered_map<std::string, std::string> textures;
        std::unordered_map<std::string, std::string> sounds;
        std::unordered_map<std::string, std::string> music;

        std::shared_ptr<engine::TextureHandle> loadTexture(const std::string& id, const std::string& path) override {
            textures[id] = path;
            // Don't construct sf::Texture: it requires a display/GL context.
            return nullptr;
        }

        std::shared_ptr<engine::TextureHandle> getTexture(const std::string&) const override { return nullptr; }

        std::shared_ptr<engine::SoundBufferHandle> loadSoundBuffer(const std::string& id,
                                                                   const std::string& path) override {
            sounds[id] = path;
            return nullptr;
        }

        std::shared_ptr<engine::SoundBufferHandle> getSoundBuffer(const std::string&) const override { return nullptr; }

        void registerMusicPath(const std::string& id, const std::string& path) override { music[id] = path; }

        std::string getMusicPath(const std::string& id) const override {
            auto it = music.find(id);
            return it == music.end() ? std::string{} : it->second;
        }
    };

} // namespace baja::test
