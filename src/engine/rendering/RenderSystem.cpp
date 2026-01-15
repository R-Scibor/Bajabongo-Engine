#include "engine/pch.h"
#include "RenderSystem.hpp"

#include <vector>
#include <algorithm>
#include <entt/entt.hpp>

#include "engine/core/EngineContext.hpp"
#include "engine/components/TransformComponent.hpp"
#include "engine/components/RenderableComponent.hpp"
#include "engine/rendering/IRenderer.hpp"
#include "engine/rendering/Sprite.hpp"
#include "engine/components/PhysicsBodyComponent.hpp"
#include "engine/core/ILoggerManager.hpp"
#include "engine/core/ILogger.hpp"
#include <unordered_set>

#include <box2d/box2d.h>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/ConvexShape.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <SFML/Graphics/PrimitiveType.hpp>
#include "engine/core/IResourceManager.hpp"

namespace engine
{
    RenderSystem::RenderSystem(engine::EngineContext& context)
        : m_context(context)
        , m_registry{ *context.m_registry }
        , m_renderer{ context.m_renderer }
        , m_spriteManager{ context.m_spriteManager }
        , m_resourceManager{ context.m_resourceManager }
    {
    }

    void RenderSystem::setDebugDraw(bool enable)
    {
        m_debugDraw = enable;
    }

    void RenderSystem::update(sf::RenderTarget& target)
    {
        auto view = m_registry.view<TransformComponent, RenderableComponent>();
        
        // Collect and sort by layer (existing logic)
        struct RenderItem {
            entt::entity entity;
            int layer;
            float yPos;
        };
        
        std::vector<RenderItem> items;
        items.reserve(view.size_hint());
        
        view.each([&](auto entity, const auto& transform, const auto& renderable) {
            items.push_back({entity, renderable.layer, transform.position.y});
        });
        
        // Sort: layer first, then Y position
        std::sort(items.begin(), items.end(), [](const auto& a, const auto& b) {
            if (a.layer != b.layer) return a.layer < b.layer;
            return a.yPos < b.yPos;
        });
        
        // Render each entity to the target
        for (const auto& item : items) {
            auto& transform = view.get<TransformComponent>(item.entity);
            auto& renderable = view.get<RenderableComponent>(item.entity);
            
            const auto* spriteDesc = m_spriteManager->getSprite(renderable.spriteId);
            if (!spriteDesc) continue;
            
            auto texture = m_resourceManager->getTexture(spriteDesc->textureId);
            if (!texture) continue;
            
            // Create SFML sprite
            // SFML 3.x: Constructor requires texture
            sf::Sprite sprite(*texture);
            sprite.setTextureRect(spriteDesc->uvRect);
            
            // SFML 3.x: All transforms take Vector2f (or Angle for rotation)
            // Use initializer lists for Vector2f construction
            sprite.setOrigin({static_cast<float>(spriteDesc->origin.x), static_cast<float>(spriteDesc->origin.y)});
            sprite.setPosition({static_cast<float>(transform.position.x), static_cast<float>(transform.position.y)});
            sprite.setRotation(sf::radians(transform.rotation)); 
            sprite.setScale({static_cast<float>(transform.scale.x), static_cast<float>(transform.scale.y)});
            sprite.setColor(renderable.color);
            
            // Draw to the injected render target
            target.draw(sprite);
        }
        
        // Debug physics rendering (if enabled)
        if (m_debugDraw) {
            // Physics debug shapes also draw to target
            auto physicsView = m_registry.view<PhysicsBodyComponent, TransformComponent>();
            physicsView.each([&](entt::entity entity, const auto& bodyComp, const auto& transform) {
                 b2BodyId bodyId = bodyComp.bodyId;
                 if (!b2Body_IsValid(bodyId)) return;

                 int shapeCount = b2Body_GetShapeCount(bodyId);
                 std::vector<b2ShapeId> shapes(shapeCount);
                 b2Body_GetShapes(bodyId, shapes.data(), shapeCount);

                 for (b2ShapeId shapeId : shapes)
                 {
                     b2ShapeType type = b2Shape_GetType(shapeId);
                     if (type == b2_circleShape) {
                         b2Circle circle = b2Shape_GetCircle(shapeId);
                         b2Vec2 worldCenter = b2Body_GetWorldPoint(bodyId, circle.center);
                         
                         sf::CircleShape debugShape(circle.radius);
                         debugShape.setOrigin({circle.radius, circle.radius});
                         debugShape.setPosition({worldCenter.x, worldCenter.y});
                         debugShape.setFillColor(sf::Color::Transparent);
                         debugShape.setOutlineColor(sf::Color::Green);
                         debugShape.setOutlineThickness(1.f);
                         target.draw(debugShape);
                     } else if (type == b2_polygonShape) {
                         b2Polygon polygon = b2Shape_GetPolygon(shapeId);
                         sf::ConvexShape debugShape(polygon.count);
                         
                         for (int i = 0; i < polygon.count; ++i) {
                             b2Vec2 worldPoint = b2Body_GetWorldPoint(bodyId, polygon.vertices[i]);
                             debugShape.setPoint(i, {worldPoint.x, worldPoint.y});
                         }
                         
                         debugShape.setFillColor(sf::Color::Transparent);
                         debugShape.setOutlineColor(sf::Color::Green);
                         debugShape.setOutlineThickness(1.f);
                         target.draw(debugShape);
                     } else if (type == b2_capsuleShape) {
                         b2Capsule capsule = b2Shape_GetCapsule(shapeId);
                         b2Vec2 p1 = b2Body_GetWorldPoint(bodyId, capsule.center1);
                         b2Vec2 p2 = b2Body_GetWorldPoint(bodyId, capsule.center2);
                         
                         sf::CircleShape c1(capsule.radius);
                         c1.setOrigin({capsule.radius, capsule.radius});
                         c1.setPosition({p1.x, p1.y});
                         c1.setFillColor(sf::Color::Transparent);
                         c1.setOutlineColor(sf::Color::Green);
                         c1.setOutlineThickness(1.f);
                         target.draw(c1);
                         
                         sf::CircleShape c2(capsule.radius);
                         c2.setOrigin({capsule.radius, capsule.radius});
                         c2.setPosition({p2.x, p2.y});
                         c2.setFillColor(sf::Color::Transparent);
                         c2.setOutlineColor(sf::Color::Green);
                         c2.setOutlineThickness(1.f);
                         target.draw(c2);
                         
                         // Draw lines connecting circles
                         // Simplified: just a line between centers
                         sf::Vertex line[] = {
                             sf::Vertex({p1.x, p1.y}, sf::Color::Green),
                             sf::Vertex({p2.x, p2.y}, sf::Color::Green)
                         };
                         target.draw(line, 2, sf::PrimitiveType::Lines);
                     } else if (type == b2_chainSegmentShape) {
                         b2ChainSegment segment = b2Shape_GetChainSegment(shapeId);
                         b2Vec2 p1 = b2Body_GetWorldPoint(bodyId, segment.segment.point1);
                         b2Vec2 p2 = b2Body_GetWorldPoint(bodyId, segment.segment.point2);
                         
                         sf::Vertex line[] = {
                             sf::Vertex({p1.x, p1.y}, sf::Color::Blue),
                             sf::Vertex({p2.x, p2.y}, sf::Color::Blue)
                         };
                         target.draw(line, 2, sf::PrimitiveType::Lines);
                     }
                 }
            });
        }
    }
}
