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
        
        virtual void drawSprite(const struct SpriteDesc& sprite, const struct TransformComponent& transform) = 0;
        virtual void drawShape(engine::Vector2f position, float radius) = 0;
        
        virtual void endFrame() = 0;

    protected:
        IWindow& m_window;
    };
}