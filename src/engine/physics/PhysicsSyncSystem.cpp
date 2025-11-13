#include "engine/pch.h"
#include "PhysicsSyncSystem.hpp"

#include <entt/entt.hpp>
#include "engine/components/TransformComponent.hpp"
#include "engine/components/PhysicsBodyComponent.hpp"
#include "engine/core/math/Vector2.hpp"

#include <box2d/box2d.h>

namespace engine
{

    PhysicsSyncSystem::PhysicsSyncSystem(entt::registry& registry)
        : m_registry(registry)
    {
    }

    void PhysicsSyncSystem::update()
    {
        auto view = m_registry.view<TransformComponent, const PhysicsBodyComponent>();
        for (auto entity : view)
        {
            auto& transform = view.get<TransformComponent>(entity);
            const auto& physicsBody = view.get<const PhysicsBodyComponent>(entity);

            b2Vec2 position = b2Body_GetPosition(physicsBody.bodyId);
            transform.position.x = position.x;
            transform.position.y = position.y;
        }
    }

} // namespace engine