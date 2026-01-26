#pragma once
#include "engine/core/math/MathAliases.hpp"
#include <cstdint>
#include <vector>
#include <string>

namespace engine
{
    class IWindow;
    struct SpriteDesc;
    struct TransformComponent;

    /// @brief Simple structure representing an RGBA color.
    struct Color {
        uint8_t r, g, b, a;
    };

    /**
     * @brief Pure abstract interface for the rendering backend.
     * * Applies the **Dependency Inversion Principle**. The game logic (RenderSystem)
     * depends on this abstraction, not on concrete SFML classes.
     * This allows swapping the rendering engine (e.g., to OpenGL, Vulkan, or a headless test renderer)
     * without changing the game logic.
     */
    class IRenderer {
    public:
        explicit IRenderer(IWindow& window) : m_window(window) {}
        virtual ~IRenderer() = default;

        /**
         * @brief Prepares the render target for a new frame (e.g., clears buffers).
         */
        virtual void beginFrame() = 0;

        /**
         * @brief Clears the screen with a specific background color.
         */
        virtual void clear(Color color) = 0;
        
        /**
         * @brief Draws a sprite based on engine components.
         * @param sprite The sprite metadata (UV rects, texture ID).
         * @param transform The world transform (position, rotation, scale).
         * @param color Tint color.
         */
        virtual void drawSprite(const struct SpriteDesc& sprite, const struct TransformComponent& transform, const Color& color) = 0;

        // --- Primitives Drawing ---
        virtual void drawCircle(engine::Vector2f position, float radius) = 0;
        virtual void drawRect(engine::Vector2f position, engine::Vector2f size) = 0;
        virtual void drawRect(engine::Vector2f position, engine::Vector2f size, Color color) = 0;
        virtual void drawText(const std::string& text, engine::Vector2f position, uint32_t fontSize, Color color) = 0;
        virtual void drawLine(engine::Vector2f start, engine::Vector2f end, Color color) = 0;
        virtual void drawPolygon(const std::vector<engine::Vector2f>& vertices, const Color& color) = 0;
        
        // --- Texture Drawing (Abstraction Leak / Optimization) ---

        /**
         * @brief Draws a raw texture directly.
         * * Uses `void*` as a handle to avoid coupling with sf::Texture in the header.
         * * Essential for full-screen effects or systems like Fog of War.
         */
        virtual void drawTexture(const void* textureHandle, engine::Vector2f position) = 0;
        virtual void drawTexture(const void* textureHandle, engine::Vector2f position, engine::Vector2f size) = 0;
        
        /**
         * @brief Direct pass-through for drawing a native sprite object.
         * @note HACK: Used for debugging or legacy integration where full component lookup is too slow/complex.
         */
        virtual void drawSpriteDirect(const void* spriteHandle, engine::Vector2f position) = 0;

        // --- View / Camera Control ---

        /// @brief Converts screen pixel coordinates to world game units (useful for mouse picking).
        virtual engine::Vector2f screenToWorld(engine::Vector2f screenPos) = 0;
        
        virtual void setViewCenter(engine::Vector2f center) = 0;
        virtual engine::Vector2f getViewCenter() const = 0;
        virtual void setViewSize(engine::Vector2f size) = 0;
        virtual engine::Vector2u getWindowSize() const = 0;

        /**
         * @brief Finalizes the frame and swaps buffers (displaying the result).
         */
        virtual void endFrame() = 0;

    protected:
        IWindow& m_window;
    };
}