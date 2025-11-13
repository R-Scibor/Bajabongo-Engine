#include "engine/pch.h"
#include "PhysicsSyncSystem.hpp"

#include <sstream>
#include <entt/entt.hpp>
#include "engine/components/TransformComponent.hpp"
#include "engine/components/PhysicsBodyComponent.hpp"
#include "engine/core/math/Vector2.hpp"
#include "engine/core/ILoggerManager.hpp"
#include "engine/core/ILogger.hpp"

#include <box2d/box2d.h>

namespace engine
{

    PhysicsSyncSystem::PhysicsSyncSystem(entt::registry& registry, ILoggerManager& logManager)
        : m_registry(registry)
    {
        m_logger = logManager.GetLogger("Physics");
        m_logger->info("PhysicsSyncSystem initialized.");
    }

    void PhysicsSyncSystem::update()
    {
        auto view = m_registry.view<TransformComponent, const PhysicsBodyComponent>();
        
        m_logger->trace("Syncing transforms for physics bodies.");

        for (auto entity : view)
        {
            auto& transform = view.get<TransformComponent>(entity);
            const auto& physicsBody = view.get<const PhysicsBodyComponent>(entity);

            if (!b2Body_IsValid(physicsBody.bodyId)) {
                std::stringstream err_ss;
                err_ss << "Attempted to sync transform for an invalid bodyId on entity " << entt::to_integral(entity);
                m_logger->warn(err_ss.str());
                continue;
            }

            b2Vec2 position = b2Body_GetPosition(physicsBody.bodyId);
            transform.position.x = position.x;
            transform.position.y = position.y;
        }
    }

} // namespace engine