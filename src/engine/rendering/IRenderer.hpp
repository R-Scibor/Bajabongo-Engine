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
        virtual void drawRect(engine::Vector2f position, engine::Vector2f size, Color color) = 0;
        virtual void drawText(const std::string& text, engine::Vector2f position, uint32_t fontSize, Color color) = 0;
        virtual void drawLine(engine::Vector2f start, engine::Vector2f end, Color color) = 0;
        virtual void drawPolygon(const std::vector<engine::Vector2f>& vertices, const Color& color) = 0;
        
        // New method for drawing raw textures (needed for Fog of War)
        // We use a void* for the texture handle because IRenderer is abstract.
        // However, standard practice in C++ interfaces often uses a wrapped Texture class or similar.
        // For this crunch, since we are using SFML, let's just forward declare sf::Texture?
        // No, that leaks implementation details.
        // But we are already leaking SFML types in other places (like sf::Event in IWindow).
        // Let's stick to the cleanest way possible given constraints.
        // We will pass an abstract "TextureHandle" or just accept we are tightly coupled to SFML for now.
        // Actually, the user instruction said "Action: Add a generic method... drawTexture(const sf::Texture& texture...)"
        // This implies adding sf::Texture dependency to IRenderer.
        
        virtual void drawTexture(const void* textureHandle, engine::Vector2f position) = 0;
        virtual void drawTexture(const void* textureHandle, engine::Vector2f position, engine::Vector2f size) = 0;
        
        // HACK: Allowing drawing of sf::Sprite directly for debug purposes in GameplayState
        virtual void drawSpriteDirect(const void* spriteHandle, engine::Vector2f position) = 0;

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