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

namespace engine
{
    RenderSystem::RenderSystem(const EngineContext& context)
        : m_registry{ *context.m_registry }
        , m_renderer{ *context.m_renderer }
        , m_spriteManager{ *context.m_spriteManager }
    {
        if (context.m_logManager) {
            m_logger = context.m_logManager->GetLogger("RenderSystem");
        }
    }

    void RenderSystem::update()
    {
        auto view = m_registry.view<TransformComponent, RenderableComponent>();

        struct RenderItem {
            int layer;
            float y;
            entt::entity entity;
        };

        std::vector<RenderItem> items;
        items.reserve(view.size_hint());

        view.each([&](auto entity, const auto& transform, const auto& renderable) {
            items.push_back({ renderable.layer, transform.position.y, entity });
        });

        std::sort(items.begin(), items.end(), [](const RenderItem& a, const RenderItem& b) {
            if (a.layer != b.layer) {
                return a.layer < b.layer;
            }
            return a.y < b.y;
        });

        for (const auto& item : items)
        {
            auto& transform = view.get<TransformComponent>(item.entity);
            // We already have layer, but we need other renderable data (spriteId)
            // Re-fetching renderable is cheap/fast reference
            auto& renderable = view.get<RenderableComponent>(item.entity);

            // Fetch sprite description
            const auto* spriteDesc = m_spriteManager.getSprite(renderable.spriteId);
            if (spriteDesc) {
                // Draw using the renderer
                m_renderer.drawSprite(*spriteDesc, transform, { renderable.color.r, renderable.color.g, renderable.color.b, renderable.color.a });
            } else {
                static std::unordered_set<std::string> missingSprites;
                if (missingSprites.find(renderable.spriteId) == missingSprites.end()) {
                    if (m_logger) {
                        m_logger->warn("Missing sprite definition: {}", renderable.spriteId);
                    }
                    missingSprites.insert(renderable.spriteId);
                }
            }
        }

        // Debug drawing for physics bodies
        if (m_debugDraw)
        {
            auto physicsView = m_registry.view<TransformComponent, PhysicsBodyComponent>();
            physicsView.each([this](const auto& transform, const auto& physicsBody) {
                b2BodyId bodyId = physicsBody.bodyId;
                if (!b2Body_IsValid(bodyId)) return;

                int shapeCount = b2Body_GetShapeCount(bodyId);
                // Since we don't have easy access to a dynamic array for shapes here and want to avoid allocation,
                // we'll just handle the first few shapes or iterate if Box2D allows.
                // Actually, Box2D 3.0 uses ids. Let's get all shapes.
                std::vector<b2ShapeId> shapes(shapeCount);
                b2Body_GetShapes(bodyId, shapes.data(), shapeCount);

                for (b2ShapeId shapeId : shapes)
                {
                    b2ShapeType type = b2Shape_GetType(shapeId);
                    if (type == b2_circleShape) {
                        b2Circle circle = b2Shape_GetCircle(shapeId);
                        // Transform local center to world position
                        b2Vec2 worldCenter = b2Body_GetWorldPoint(bodyId, circle.center);
                        m_renderer.drawCircle({worldCenter.x, worldCenter.y}, circle.radius);
                    } else if (type == b2_polygonShape) {
                         b2Polygon polygon = b2Shape_GetPolygon(shapeId);
                         // Assuming it's a box (AABB-like) for this specific task requirement of "Rectangle/Circle"
                         // A generic polygon drawer would be better but requires more IRenderer features.
                         // For a box created via MakeBox, we can approximate the bounding box or use the vertices.
                         // But the prompt asked specifically to "draw a hollow rectangle/circle (depending on box2d shape and size)".
                         // Let's use the polygon AABB for simplicity if it's a rect, or just the bounds.
                         // However, to be accurate to the "Box" shape:
                         // Reconstruct the box from vertices or AABB.
                         b2AABB aabb = b2Shape_GetAABB(shapeId);
                         float width = aabb.upperBound.x - aabb.lowerBound.x;
                         float height = aabb.upperBound.y - aabb.lowerBound.y;
                         float centerX = (aabb.upperBound.x + aabb.lowerBound.x) * 0.5f;
                         float centerY = (aabb.upperBound.y + aabb.lowerBound.y) * 0.5f;
                         
                         m_renderer.drawRect({centerX, centerY}, {width, height});
                    } else if (type == b2_capsuleShape) {
                        // Fallback for capsule - draw as two circles + rect? or just a rect of bounds
                        b2AABB aabb = b2Shape_GetAABB(shapeId);
                        float width = aabb.upperBound.x - aabb.lowerBound.x;
                        float height = aabb.upperBound.y - aabb.lowerBound.y;
                        float centerX = (aabb.upperBound.x + aabb.lowerBound.x) * 0.5f;
                        float centerY = (aabb.upperBound.y + aabb.lowerBound.y) * 0.5f;
                        m_renderer.drawRect({centerX, centerY}, {width, height});
                    }
                }
            });
        }
    }
}
