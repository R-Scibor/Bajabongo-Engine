#include "SFMLRenderer.hpp"
#include "engine/core/IWindow.hpp"
#include "engine/core/IResourceManager.hpp"
#include "engine/rendering/Sprite.hpp"
#include "engine/components/TransformComponent.hpp"
#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/WindowHandle.hpp>
#include <SFML/Window/Event.hpp>     
#include <SFML/Window/VideoMode.hpp> 
#include <SFML/Graphics/Sprite.hpp>
#include <optional>                 

namespace engine
{
    SFMLRenderer::SFMLRenderer()
        : IRenderer(*this) {
        m_circleShape.setFillColor(sf::Color::Transparent);
        m_circleShape.setOutlineColor(sf::Color::Red);
        m_circleShape.setOutlineThickness(1.0f);

        m_rectShape.setFillColor(sf::Color::Transparent);
        m_rectShape.setOutlineColor(sf::Color::Green);
        m_rectShape.setOutlineThickness(1.0f);
    }

    SFMLRenderer::~SFMLRenderer() {
        if (m_renderWindow.isOpen()) {
            m_renderWindow.close();
        }
    }

    // --- Implementacja IWindow ---

    void SFMLRenderer::create(const std::string& title, unsigned int width, unsigned int height) {
        m_renderWindow.create(sf::VideoMode({ width, height }), title, sf::Style::Default);
    }

    void SFMLRenderer::close() {
        m_renderWindow.close();
    }

    bool SFMLRenderer::isOpen() const {
        return m_renderWindow.isOpen();
    }


    void* SFMLRenderer::getNativeHandle() const {
        return reinterpret_cast<void*>(m_renderWindow.getNativeHandle());
    }

    std::optional<sf::Event> SFMLRenderer::pollEvent() {
        return m_renderWindow.pollEvent();
    }

    // --- Implementacja IRenderer ---

    void SFMLRenderer::beginFrame() {
        // Cast to void to suppress [[nodiscard]] warning
        (void)m_renderWindow.setActive(true);
    }

    void SFMLRenderer::clear(Color color) {
        m_renderWindow.clear(sf::Color(color.r, color.g, color.b, color.a));
    }

    void SFMLRenderer::drawCircle(engine::Vector2f position, float radius) {
        sf::Vector2f sfmlPosition(position.x, position.y);
        m_circleShape.setPosition(sfmlPosition);
        m_circleShape.setRadius(radius);
        m_circleShape.setOrigin({radius, radius}); // Center the circle
        m_renderWindow.draw(m_circleShape);
    }

    void SFMLRenderer::drawRect(engine::Vector2f position, engine::Vector2f size) {
        sf::Vector2f sfmlPosition(position.x, position.y);
        sf::Vector2f sfmlSize(size.x, size.y);

        m_rectShape.setPosition(sfmlPosition);
        m_rectShape.setSize(sfmlSize);
        m_rectShape.setOrigin({sfmlSize.x * 0.5f, sfmlSize.y * 0.5f}); // Center the rect
        m_renderWindow.draw(m_rectShape);
    }

    void SFMLRenderer::drawSprite(const SpriteDesc& spriteDesc, const TransformComponent& transform) {
        if (!m_resourceManager) return;

        auto texture = m_resourceManager->getTexture(spriteDesc.textureId);
        if (!texture) return; // Or draw a placeholder

        sf::Sprite sprite(*texture);
        sprite.setTextureRect(spriteDesc.uvRect);
        sprite.setOrigin(spriteDesc.origin);
        // Fix: Explicitly construct sf::Vector2f to avoid ambiguity or mismatch
        sprite.setPosition(sf::Vector2f(transform.position.x, transform.position.y));
        
        sprite.setRotation(sf::radians(transform.rotation));
        sprite.setScale({transform.scale.x, transform.scale.y});

        m_renderWindow.draw(sprite);
    }

    void SFMLRenderer::endFrame() {
        m_renderWindow.display();
    }

    // --- SFML Specific ---

    void SFMLRenderer::setResourceManager(std::shared_ptr<IResourceManager> resourceManager) {
        m_resourceManager = std::move(resourceManager);
    }

    sf::RenderWindow& SFMLRenderer::getNativeRenderWindow() {
        return m_renderWindow;
    }
}