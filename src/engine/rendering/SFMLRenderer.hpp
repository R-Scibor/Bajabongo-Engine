#pragma once
#include "engine/core/math/MathAliases.hpp"
#include "engine/rendering/IRenderer.hpp"
#include "engine/core/IWindow.hpp"
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <memory>
#include <optional>

namespace engine
{
    class ILogger;
}

namespace engine
{
    /**
     * @brief Concrete implementation of IRenderer and IWindow using SFML.
     */
    class SFMLRenderer : public IRenderer, public IWindow {
    public:
        SFMLRenderer();
        ~SFMLRenderer() override;

        // --- IWindow Implementation ---
        void create(const std::string& title, unsigned int width, unsigned int height) override;
        void close() override;
        bool isOpen() const override;
        void* getNativeHandle() const override;
        std::optional<sf::Event> pollEvent() override;
        Vector2u getSize() const override;
        void setFramerateLimit(unsigned int limit) override;

        // --- IRenderer Implementation ---
        void beginFrame() override;
        void clear(Color color) override;
        void drawCircle(engine::Vector2f position, float radius) override;
        void drawRect(engine::Vector2f position, engine::Vector2f size) override;
        void drawLine(engine::Vector2f start, engine::Vector2f end, Color color) override;
        void drawPolygon(const std::vector<engine::Vector2f>& vertices, const Color& color) override;
        void drawTexture(const void* textureHandle, engine::Vector2f position) override;
        void drawSpriteDirect(const void* spriteHandle, engine::Vector2f position) override;
        void drawSprite(const SpriteDesc& sprite, const TransformComponent& transform, const Color& color) override;
        engine::Vector2f screenToWorld(engine::Vector2f screenPos) override;
        void setViewCenter(engine::Vector2f center) override;
        engine::Vector2f getViewCenter() const override;
        void setViewSize(engine::Vector2f size) override;
        engine::Vector2u getWindowSize() const override;
        void endFrame() override;

        // --- SFML Specific ---
        void setResourceManager(std::shared_ptr<class IResourceManager> resourceManager);
        void setLogger(std::shared_ptr<ILogger> logger) { m_logger = logger; }
        sf::RenderWindow& getNativeRenderWindow();

    private:
        sf::RenderWindow m_renderWindow;
        std::shared_ptr<class IResourceManager> m_resourceManager;
        sf::CircleShape m_circleShape;
        sf::RectangleShape m_rectShape;
        std::shared_ptr<ILogger> m_logger;
    };
}