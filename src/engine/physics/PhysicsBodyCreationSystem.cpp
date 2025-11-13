#include "engine/pch.h"
#include "PhysicsBodyCreationSystem.hpp"

#include <sstream>
#include <entt/entt.hpp>
#include "engine/components/PendingPhysicsBodyComponent.hpp"
#include "engine/components/PhysicsBodyComponent.hpp"
#include "engine/core/ILoggerManager.hpp"
#include "engine/core/ILogger.hpp"

#include <box2d/box2d.h>

namespace engine
{

    PhysicsBodyCreationSystem::PhysicsBodyCreationSystem(entt::registry& registry, b2WorldId worldId, ILoggerManager& logManager)
        : m_registry(registry)
        , m_worldId(worldId)
    {
        m_logger = logManager.GetLogger("Physics");
        m_logger->info("PhysicsBodyCreationSystem initialized.");
    }

    void PhysicsBodyCreationSystem::update()
    {
        auto view = m_registry.view<PendingPhysicsBodyComponent>();
        if (view.empty()) {
            return;
        }

        std::stringstream ss;
        ss << "Creating " << view.size() << " new physics bodies.";
        m_logger->debug(ss.str());

        for (auto entity : view)
        {
            const auto& pending = view.get<PendingPhysicsBodyComponent>(entity);

            b2BodyDef bodyDef = b2DefaultBodyDef();
            bodyDef.type = pending.isStatic ? b2_staticBody : b2_dynamicBody;
            bodyDef.position = { pending.position.x, pending.position.y };
            
            b2BodyId bodyId = b2CreateBody(m_worldId, &bodyDef);
            if (!b2Body_IsValid(bodyId)) {
                std::stringstream err_ss;
                err_ss << "Failed to create physics body for entity " << entt::to_integral(entity);
                m_logger->error(err_ss.str());
                continue;
            }
            b2Body_SetUserData(bodyId, (void*)(uintptr_t)entity);

            b2Polygon box = b2MakeBox(pending.size.x / 2.0f, pending.size.y / 2.0f);
            b2ShapeDef shapeDef = b2DefaultShapeDef();
            shapeDef.density = pending.density;
            b2ShapeId shapeId = b2CreatePolygonShape(bodyId, &shapeDef, &box);

            if (!b2Shape_IsValid(shapeId)) {
                std::stringstream err_ss;
                err_ss << "Failed to create shape for physics body on entity " << entt::to_integral(entity);
                m_logger->error(err_ss.str());
                b2DestroyBody(bodyId); // Clean up the created body
                continue;
            }

            m_registry.emplace<PhysicsBodyComponent>(entity, bodyId);
            m_registry.remove<PendingPhysicsBodyComponent>(entity);

            std::stringstream trace_ss;
            trace_ss << "Created physics body for entity " << entt::to_integral(entity);
            m_logger->trace(trace_ss.str());
        }
    }

} // namespace engine