#pragma once

#include "engine/core/EngineContext.hpp"
#include "engine/ecs/ArchetypeManager.hpp"
#include "engine/core/math/MathAliases.hpp"
#include <entt/entt.hpp>
#include <nlohmann/json.hpp>
#include <string>
#include <memory>
#include <functional>
#include <unordered_map>

namespace engine {

    class ILogger;

    class EntityFactory {
    public:
        using ComponentLoaderFn = std::function<void(entt::registry&, entt::entity, const nlohmann::json&)>;

        EntityFactory(EngineContext& context, std::shared_ptr<ArchetypeManager> archetypeManager);

        entt::entity spawn(const std::string& archetypeId, const Vector2f& position);

        void registerComponentLoader(const std::string& componentName, ComponentLoaderFn loader);

    private:
        EngineContext& m_context;
        std::shared_ptr<ArchetypeManager> m_archetypeManager;
        std::shared_ptr<ILogger> m_logger;

        std::unordered_map<std::string, ComponentLoaderFn> m_componentRegistry;

        void registerDefaultLoaders();
    };

} // namespace engine