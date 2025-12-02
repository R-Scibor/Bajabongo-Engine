#pragma once
#include "engine/core/math/MathAliases.hpp"
#include <cstdint>

namespace engine
{
    class IWindow;
    struct SpriteDesc;
    struct TransformComponent;

    struct Color {
        uint8_t r, g, b, a;
    };

    /**
     * @brief Defines a pure abstract interface for a renderer.
     */
    class IRenderer {
    public:
        explicit IRenderer(IWindow& window) : m_window(window) {}
        virtual ~IRenderer() = default;

        virtual void beginFrame() = 0;
        virtual void clear(Color color) = 0;
        
        virtual void drawSprite(const struct SpriteDesc& sprite, const struct TransformComponent& transform, const Color& color) = 0;
        virtual void drawCircle(engine::Vector2f position, float radius) = 0;
        virtual void drawRect(engine::Vector2f position, engine::Vector2f size) = 0;
        virtual void drawLine(engine::Vector2f start, engine::Vector2f end, Color color) = 0;
        
        virtual engine::Vector2f screenToWorld(engine::Vector2f screenPos) = 0;
        virtual void setViewCenter(engine::Vector2f center) = 0;
        virtual engine::Vector2f getViewCenter() const = 0;
        virtual void setViewSize(engine::Vector2f size) = 0;
        virtual engine::Vector2u getWindowSize() const = 0;

        virtual void endFrame() = 0;

    protected:
        IWindow& m_window;
    };
}