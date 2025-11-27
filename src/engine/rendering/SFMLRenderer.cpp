#include "SFMLRenderer.hpp"
#include "engine/core/IWindow.hpp"
#include "engine/core/IResourceManager.hpp"
#include "engine/rendering/Sprite.hpp"
#include "engine/components/TransformComponent.hpp"
#include "engine/core/ILogger.hpp"
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

    void SFMLRenderer::drawLine(engine::Vector2f start, engine::Vector2f end, Color color) {
        sf::Vertex line[] = {
            sf::Vertex(sf::Vector2f(start.x, start.y), sf::Color(color.r, color.g, color.b, color.a)),
            sf::Vertex(sf::Vector2f(end.x, end.y), sf::Color(color.r, color.g, color.b, color.a))
        };
        m_renderWindow.draw(line, 2, sf::PrimitiveType::Lines);
    }

    void SFMLRenderer::drawSprite(const SpriteDesc& spriteDesc, const TransformComponent& transform, const Color& color) {
        if (!m_resourceManager) {
            if (m_logger) m_logger->error("SFMLRenderer: ResourceManager is not set. Cannot draw sprite.");
            return;
        }

        auto texture = m_resourceManager->getTexture(spriteDesc.textureId);
        if (!texture) {
            // Texture loading failure already logged by ResourceManager,
            // but we might want to log that drawing failed too, or just return.
            // ResourceManager logs error on getTexture failure, so we can just return.
            return;
        }

        sf::Sprite sprite(*texture);
        sprite.setTextureRect(spriteDesc.uvRect);
        sprite.setOrigin(spriteDesc.origin);
        // Fix: Explicitly construct sf::Vector2f to avoid ambiguity or mismatch
        sprite.setPosition(sf::Vector2f(transform.position.x, transform.position.y));
        
        sprite.setRotation(sf::radians(transform.rotation));
        sprite.setScale({transform.scale.x, transform.scale.y});
        sprite.setColor(sf::Color(color.r, color.g, color.b, color.a));

        m_renderWindow.draw(sprite);
    }

    engine::Vector2f SFMLRenderer::screenToWorld(engine::Vector2f screenPos) {
        sf::Vector2i pixelPos(static_cast<int>(screenPos.x), static_cast<int>(screenPos.y));
        sf::Vector2f worldPos = m_renderWindow.mapPixelToCoords(pixelPos);
        return { worldPos.x, worldPos.y };
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