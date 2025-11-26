#include "engine/pch.h"
#include "PhysicsEventSystem.hpp"

#include "engine/core/EngineContext.hpp"
#include "engine/core/ILoggerManager.hpp"
#include "engine/core/ILogger.hpp"
#include "engine/events/PhysicsEvents.hpp"

#include <box2d/box2d.h>
#include <entt/entt.hpp>

namespace engine
{
    PhysicsEventSystem::PhysicsEventSystem(EngineContext& context)
        : m_worldId{ context.m_physicsWorld }
        , m_dispatcher{ context.m_dispatcher }
        , m_registry{ context.m_registry }
    {
        m_logger = context.m_logManager->GetLogger("PhysicsEventSystem");
        m_logger->info("PhysicsEventSystem initialized.");
    }

    void PhysicsEventSystem::update()
    {
        pollContacts();
        pollSensors();
    }

    void PhysicsEventSystem::pollContacts()
    {
        b2ContactEvents events = b2World_GetContactEvents(m_worldId);

        for (int i = 0; i < events.beginCount; ++i)
        {
            b2ContactBeginTouchEvent* beginEvent = events.beginEvents + i;
            
            b2ShapeId shapeIdA = beginEvent->shapeIdA;
            b2ShapeId shapeIdB = beginEvent->shapeIdB;

            auto entityA = (entt::entity)(uintptr_t)b2Shape_GetUserData(shapeIdA);
            auto entityB = (entt::entity)(uintptr_t)b2Shape_GetUserData(shapeIdB);

            if (m_registry->valid(entityA) && m_registry->valid(entityB))
            {
                m_dispatcher->enqueue<PhysicsContactBeginEvent>(entityA, entityB);
            }
        }

        for (int i = 0; i < events.endCount; ++i)
        {
            b2ContactEndTouchEvent* endEvent = events.endEvents + i;

            b2ShapeId shapeIdA = endEvent->shapeIdA;
            b2ShapeId shapeIdB = endEvent->shapeIdB;

            auto entityA = (entt::entity)(uintptr_t)b2Shape_GetUserData(shapeIdA);
            auto entityB = (entt::entity)(uintptr_t)b2Shape_GetUserData(shapeIdB);

            // Even if entities are destroyed, we might want to signal end contact? 
            // But if they are destroyed, the registry check will fail.
            // For now, only dispatch if valid to be safe.
            if (m_registry->valid(entityA) && m_registry->valid(entityB))
            {
                m_dispatcher->enqueue<PhysicsContactEndEvent>(entityA, entityB);
            }
        }
    }

    void PhysicsEventSystem::pollSensors()
    {
        b2SensorEvents events = b2World_GetSensorEvents(m_worldId);

        if (events.beginCount > 0) {
             m_logger->debug("PhysicsEventSystem: Found {} SENSOR begin events", events.beginCount);
        }

        for (int i = 0; i < events.beginCount; ++i)
        {
            b2SensorBeginTouchEvent* beginEvent = events.beginEvents + i;

            b2ShapeId sensorShapeId = beginEvent->sensorShapeId;
            b2ShapeId visitorShapeId = beginEvent->visitorShapeId;

            auto sensorEntity = (entt::entity)(uintptr_t)b2Shape_GetUserData(sensorShapeId);
            auto visitorEntity = (entt::entity)(uintptr_t)b2Shape_GetUserData(visitorShapeId);

            if (m_registry->valid(sensorEntity) && m_registry->valid(visitorEntity))
            {
                m_dispatcher->enqueue<PhysicsSensorBeginEvent>(sensorEntity, visitorEntity);
            }
        }

        for (int i = 0; i < events.endCount; ++i)
        {
            b2SensorEndTouchEvent* endEvent = events.endEvents + i;

            b2ShapeId sensorShapeId = endEvent->sensorShapeId;
            b2ShapeId visitorShapeId = endEvent->visitorShapeId;

            auto sensorEntity = (entt::entity)(uintptr_t)b2Shape_GetUserData(sensorShapeId);
            auto visitorEntity = (entt::entity)(uintptr_t)b2Shape_GetUserData(visitorShapeId);

            if (m_registry->valid(sensorEntity) && m_registry->valid(visitorEntity))
            {
                m_dispatcher->enqueue<PhysicsSensorEndEvent>(sensorEntity, visitorEntity);
            }
        }
    }

} // namespace engine