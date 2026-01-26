#include "engine/pch.h"
#include "PhysicsBodyCreationSystem.hpp"

#include "engine/core/EngineContext.hpp"

/**
 * @file PhysicsBodyCreationSystem.cpp
 * @brief Implementation of the PhysicsBodyCreationSystem class.
 */
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
            bodyDef.isBullet = pending.isBullet;
            bodyDef.linearVelocity = { pending.initialVelocity.x, pending.initialVelocity.y };
            bodyDef.rotation = b2MakeRot(pending.rotation);

            b2BodyId bodyId = b2CreateBody(m_worldId, &bodyDef);
            if (!b2Body_IsValid(bodyId)) {
                m_logger->error(
                    "Failed to create physics body for entity {}",
                    entt::to_integral(entity)
                );
                continue;
            }
            b2Body_SetUserData(bodyId, (void*)(uintptr_t)entity);

            // Use fixture list if available
            if (!pending.fixtures.empty()) {
                for (const auto& fixDef : pending.fixtures) {
                    b2Polygon box = b2MakeOffsetBox(
                        fixDef.size.x / 2.0f, 
                        fixDef.size.y / 2.0f, 
                        {fixDef.offset.x, fixDef.offset.y}, 
                        b2MakeRot(0.0f) // No local rotation for now
                    );
                    
                    b2ShapeDef shapeDef = b2DefaultShapeDef();
                    shapeDef.density = fixDef.density;
                    shapeDef.isSensor = fixDef.isSensor;
                    shapeDef.filter.categoryBits = fixDef.categoryBits;
                    shapeDef.filter.maskBits = fixDef.maskBits;
                    shapeDef.enableSensorEvents = true; // Always enable for sensors
                    
                    if (!fixDef.isSensor) {
                        shapeDef.enableContactEvents = true;
                    }

                    b2ShapeId shapeId = b2CreatePolygonShape(bodyId, &shapeDef, &box);
                     if (b2Shape_IsValid(shapeId)) {
                        b2Shape_SetUserData(shapeId, (void*)(uintptr_t)entity);
                    } else {
                         m_logger->error("Failed to create fixture for entity {}", entt::to_integral(entity));
                    }
                }
            } else {
                m_logger->warn("Entity {} has PendingPhysicsBodyComponent but no fixtures defined.", entt::to_integral(entity));
            }



            m_registry.emplace<PhysicsBodyComponent>(entity, bodyId);
            m_registry.remove<PendingPhysicsBodyComponent>(entity);

            m_logger->trace(
                "Created physics body for entity {}",
                entt::to_integral(entity)
            );
        }
    }

} // namespace engine
