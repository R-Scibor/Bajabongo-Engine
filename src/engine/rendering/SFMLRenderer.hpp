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

        // --- IRenderer Implementation ---
        void beginFrame() override;
        void clear(Color color) override;
        void drawCircle(engine::Vector2f position, float radius) override;
        void drawRect(engine::Vector2f position, engine::Vector2f size) override;
        void drawSprite(const SpriteDesc& sprite, const TransformComponent& transform, const Color& color) override;
        void endFrame() override;

        // --- SFML Specific ---
        void setResourceManager(std::shared_ptr<class IResourceManager> resourceManager);
        sf::RenderWindow& getNativeRenderWindow();

    private:
        sf::RenderWindow m_renderWindow;
        std::shared_ptr<class IResourceManager> m_resourceManager;
        sf::CircleShape m_circleShape;
        sf::RectangleShape m_rectShape;
    };
}