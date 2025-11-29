#include "engine/pch.h"
#include "PhysicsBodyCreationSystem.hpp"

#include "engine/core/EngineContext.hpp"
#include "engine/components/PendingPhysicsBodyComponent.hpp"
#include "engine/components/PhysicsBodyComponent.hpp"
#include "engine/core/ILoggerManager.hpp"
#include "engine/core/ILogger.hpp"

#include <box2d/box2d.h>

namespace engine
{

    PhysicsBodyCreationSystem::PhysicsBodyCreationSystem(const EngineContext& context)
        : m_registry{ *context.m_registry }
        , m_worldId{ context.m_physicsWorld }
    {
        m_logger = context.m_logManager->GetLogger("Physics");
        m_logger->info("PhysicsBodyCreationSystem initialized.");
    }

    void PhysicsBodyCreationSystem::update()
    {
        auto view = m_registry.view<PendingPhysicsBodyComponent>();
        if (view.empty()) {
            return;
        }

        m_logger->debug("Creating {} new physics bodies.", view.size());

        for (auto entity : view)
        {
            const auto& pending = view.get<PendingPhysicsBodyComponent>(entity);

            b2BodyDef bodyDef = b2DefaultBodyDef();
            bodyDef.type = pending.isStatic ? b2_staticBody : b2_dynamicBody;
            bodyDef.position = { pending.position.x, pending.position.y };
            bodyDef.fixedRotation = pending.fixedRotation;
            bodyDef.linearDamping = pending.linearDamping;

            b2BodyId bodyId = b2CreateBody(m_worldId, &bodyDef);
            if (!b2Body_IsValid(bodyId)) {
                m_logger->error(
                    "Failed to create physics body for entity {}",
                    entt::to_integral(entity)
                );
                continue;
            }
            b2Body_SetUserData(bodyId, (void*)(uintptr_t)entity);

            b2Polygon box = b2MakeBox(pending.size.x / 2.0f, pending.size.y / 2.0f);
            b2ShapeDef shapeDef = b2DefaultShapeDef();
            shapeDef.density = pending.density;
            shapeDef.isSensor = pending.isSensor;
            
            // Enable events so we can poll them later
            // Enable sensor events for all shapes to ensure reliable detection
            shapeDef.enableSensorEvents = true;
            
            if (pending.isSensor) {
                m_logger->debug("Enabling SENSOR events for entity {}", entt::to_integral(entity));
            } else {
                shapeDef.enableContactEvents = true;
            }

            b2ShapeId shapeId = b2CreatePolygonShape(bodyId, &shapeDef, &box);

            if (!b2Shape_IsValid(shapeId)) {
                m_logger->error(
                    "Failed to create shape for physics body on entity {}",
                    entt::to_integral(entity)
                );
                b2DestroyBody(bodyId);
                continue;
            }

            b2Shape_SetUserData(shapeId, (void*)(uintptr_t)entity);

            m_registry.emplace<PhysicsBodyComponent>(entity, bodyId);
            m_registry.remove<PendingPhysicsBodyComponent>(entity);

            m_logger->trace(
                "Created physics body for entity {}",
                entt::to_integral(entity)
            );
        }
    }

} // namespace engine
