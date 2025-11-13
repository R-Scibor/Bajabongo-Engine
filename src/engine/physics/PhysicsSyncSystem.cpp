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

        view.each([this](auto entity, auto& transform, const auto& physicsBody)
        {
            if (!b2Body_IsValid(physicsBody.bodyId)) {
                std::stringstream err_ss;
                err_ss << "Attempted to sync transform for an invalid bodyId on entity " << entt::to_integral(entity);
                m_logger->warn(err_ss.str());
                return; // Use return in a lambda
            }

            b2Transform bodyTransform = b2Body_GetTransform(physicsBody.bodyId);
            b2Vec2 position = bodyTransform.p;
            float angle = b2Rot_GetAngle(bodyTransform.q);

            transform.position = { position.x, position.y };
            transform.rotation = angle;

            std::stringstream log_ss;
            log_ss << "Synced entity " << entt::to_integral(entity) << ": transform.position.y = " << transform.position.y;
            m_logger->trace(log_ss.str());
        });
    }

} // namespace engine