#pragma once

#include "engine/core/IResourceManager.hpp"
#include "engine/core/ILogger.hpp" // Changed from ILoggerManager.hpp
#include <unordered_map>
#include <memory>
#include <string>

namespace engine {
    class ILoggerManager; // Forward decl

    class SFMLResourceManager : public IResourceManager {
    public:
        explicit SFMLResourceManager(std::shared_ptr<ILoggerManager> loggerManager);
        ~SFMLResourceManager() override = default;

        std::shared_ptr<TextureHandle> loadTexture(const std::string& id, const std::string& path) override;
        std::shared_ptr<TextureHandle> getTexture(const std::string& id) const override;

    private:
        std::shared_ptr<ILogger> m_logger; // Changed to ILogger
        std::unordered_map<std::string, std::shared_ptr<TextureHandle>> m_textures;
    };

} // namespace engine
