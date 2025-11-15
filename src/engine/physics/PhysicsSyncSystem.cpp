#include "engine/pch.h"
#include "PhysicsSyncSystem.hpp"

#include "engine/core/EngineContext.hpp"
#include "engine/components/TransformComponent.hpp"
#include "engine/components/PhysicsBodyComponent.hpp"
#include "engine/core/math/Vector2.hpp"
#include "engine/core/ILoggerManager.hpp"
#include "engine/core/ILogger.hpp"

#include <box2d/box2d.h>

namespace engine
{

    PhysicsSyncSystem::PhysicsSyncSystem(const EngineContext& context)
        : m_registry(*context.registry)
    {
        m_logger = context.loggerManager->GetLogger("Physics");
        m_logger->info("PhysicsSyncSystem initialized.");
    }

    void PhysicsSyncSystem::update()
    {
        auto view = m_registry.view<TransformComponent, const PhysicsBodyComponent>();
        
        m_logger->trace("Syncing transforms for physics bodies.");

        view.each([this](auto entity, auto& transform, const auto& physicsBody)
        {
            if (!b2Body_IsValid(physicsBody.bodyId)) {
                m_logger->warn("Attempted to sync transform for an invalid bodyId on entity {}", entt::to_integral(entity));
                return; // Use return in a lambda
            }

            b2Transform bodyTransform = b2Body_GetTransform(physicsBody.bodyId);
            b2Vec2 position = bodyTransform.p;
            float angle = b2Rot_GetAngle(bodyTransform.q);

            transform.position = { position.x, position.y };
            transform.rotation = angle;

            m_logger->trace("Synced entity {}: transform.position.y = {}", entt::to_integral(entity), transform.position.y);
        });
    }

} // namespace engine