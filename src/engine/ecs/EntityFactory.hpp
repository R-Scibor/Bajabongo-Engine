#pragma once

#include "engine/core/EngineContext.hpp"
#include "engine/ecs/ArchetypeManager.hpp"
#include "engine/core/math/MathAliases.hpp"
#include <entt/entt.hpp>
#include <nlohmann/json.hpp>
#include <string>
#include <memory>

namespace engine {

    class ILogger;

    class EntityFactory {
    public:
        EntityFactory(EngineContext& context, std::shared_ptr<ArchetypeManager> archetypeManager);

        entt::entity spawn(const std::string& archetypeId, const Vector2f& position);

    private:
        EngineContext& m_context;
        std::shared_ptr<ArchetypeManager> m_archetypeManager;
        std::shared_ptr<ILogger> m_logger;

        // Helper methods for components
        void addTransform(entt::entity entity, const nlohmann::json& data, const Vector2f& position);
        void addRenderable(entt::entity entity, const nlohmann::json& data);
        void addPhysics(entt::entity entity, const nlohmann::json& data, const Vector2f& position);
        void addAnimation(entt::entity entity, const nlohmann::json& data);
    };

} // namespace engine