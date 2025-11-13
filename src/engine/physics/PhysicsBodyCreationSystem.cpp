#include "engine/pch.h"
#include "PhysicsBodyCreationSystem.hpp"

#include <entt/entt.hpp>
#include "engine/components/PendingPhysicsBodyComponent.hpp"
#include "engine/components/PhysicsBodyComponent.hpp"

#include <box2d/box2d.h>

namespace engine
{

    PhysicsBodyCreationSystem::PhysicsBodyCreationSystem(entt::registry& registry, b2WorldId worldId)
        : m_registry(registry)
        , m_worldId(worldId)
    {
    }

    void PhysicsBodyCreationSystem::update()
    {
        auto view = m_registry.view<PendingPhysicsBodyComponent>();
        for (auto entity : view)
        {
            const auto& pending = view.get<PendingPhysicsBodyComponent>(entity);

            b2BodyDef bodyDef = b2DefaultBodyDef();
            bodyDef.type = pending.isStatic ? b2_staticBody : b2_dynamicBody;
            bodyDef.position = { pending.position.x, pending.position.y };
            
            b2BodyId bodyId = b2CreateBody(m_worldId, &bodyDef);
            b2Body_SetUserData(bodyId, (void*)(uintptr_t)entity);

            b2Polygon box = b2MakeBox(pending.size.x / 2.0f, pending.size.y / 2.0f);
            b2ShapeDef shapeDef = b2DefaultShapeDef();
            shapeDef.density = pending.density;
            b2CreatePolygonShape(bodyId, &shapeDef, &box);

            m_registry.emplace<PhysicsBodyComponent>(entity, bodyId);
            m_registry.remove<PendingPhysicsBodyComponent>(entity);
        }
    }

} // namespace engine