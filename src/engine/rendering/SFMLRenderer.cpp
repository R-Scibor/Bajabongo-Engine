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
        
        // Initialize reusable debug shapes with default styles (wireframe)
        m_circleShape.setFillColor(sf::Color::Transparent);
        m_circleShape.setOutlineColor(sf::Color::Red);
        m_circleShape.setOutlineThickness(1.0f);

        m_rectShape.setFillColor(sf::Color::Transparent);
        m_rectShape.setOutlineColor(sf::Color::Green);
        m_rectShape.setOutlineThickness(1.0f);

        // Robust Font Loading Strategy:
        // 1. Try local assets folder
        // 2. Try relative path (VS debugging context)
        // 3. Fallback to Windows system font
        if (!m_font.openFromFile("assets/fonts/default.ttf")) {
            if (!m_font.openFromFile("../../assets/fonts/default.ttf")) {
                // Fallback to system font if custom font fails
                (void)m_font.openFromFile("C:/Windows/Fonts/arial.ttf");
            }
        }
    }

    SFMLRenderer::~SFMLRenderer() {
        if (m_renderWindow.isOpen()) {
            m_renderWindow.close();
        }
    }

    // --- Implementacja IWindow ---

    void SFMLRenderer::create(const std::string& title, unsigned int width, unsigned int height, bool fullscreen) {
        if (fullscreen) {
            m_renderWindow.create(sf::VideoMode({ width, height }), title, sf::State::Fullscreen);
        } else {
            m_renderWindow.create(sf::VideoMode({ width, height }), title, sf::Style::Default);
        }
    }

    void SFMLRenderer::close() {
        m_renderWindow.close();
    }

    bool SFMLRenderer::isOpen() const {
        return m_renderWindow.isOpen();
    }


    void* SFMLRenderer::getNativeHandle() const {
        // Cast the OS specific handle (HWND, Window, etc.) to void*
        return reinterpret_cast<void*>(m_renderWindow.getNativeHandle());
    }

    std::optional<sf::Event> SFMLRenderer::pollEvent() {
        return m_renderWindow.pollEvent();
    }

    Vector2u SFMLRenderer::getSize() const {
        sf::Vector2u size = m_renderWindow.getSize();
        return { size.x, size.y };
    }

    void SFMLRenderer::setFramerateLimit(unsigned int limit) {
        m_renderWindow.setFramerateLimit(limit);
    }

    // --- Implementacja IRenderer ---

    void SFMLRenderer::beginFrame() {
        // Cast to void to suppress [[nodiscard]] warning if ignoring result
        (void)m_renderWindow.setActive(true);
    }

    void SFMLRenderer::clear(Color color) {
        m_renderWindow.clear(sf::Color(color.r, color.g, color.b, color.a));
    }

    void SFMLRenderer::drawCircle(engine::Vector2f position, float radius) {
        sf::Vector2f sfmlPosition(position.x, position.y);
        m_circleShape.setPosition(sfmlPosition);
        m_circleShape.setRadius(radius);
        m_circleShape.setOrigin({radius, radius}); // Center the circle origin
        m_renderWindow.draw(m_circleShape);
    }

    void SFMLRenderer::drawRect(engine::Vector2f position, engine::Vector2f size) {
        sf::Vector2f sfmlPosition(position.x, position.y);
        sf::Vector2f sfmlSize(size.x, size.y);

        // Reset to debug wireframe style (in case it was changed by fill method)
        m_rectShape.setFillColor(sf::Color::Transparent);
        m_rectShape.setOutlineColor(sf::Color::Green);
        m_rectShape.setOutlineThickness(1.0f);

        m_rectShape.setPosition(sfmlPosition);
        m_rectShape.setSize(sfmlSize);
        m_rectShape.setOrigin({sfmlSize.x * 0.5f, sfmlSize.y * 0.5f}); // Center origin
        m_renderWindow.draw(m_rectShape);
    }

    void SFMLRenderer::drawRect(engine::Vector2f position, engine::Vector2f size, Color color) {
        sf::Vector2f sfmlPosition(position.x, position.y);
        sf::Vector2f sfmlSize(size.x, size.y);

        // Fill style for UI
        m_rectShape.setFillColor(sf::Color(color.r, color.g, color.b, color.a));
        m_rectShape.setOutlineThickness(0.0f);

        m_rectShape.setPosition(sfmlPosition);
        m_rectShape.setSize(sfmlSize);
        m_rectShape.setOrigin({0.0f, 0.0f}); // Top-left origin for UI elements
        m_renderWindow.draw(m_rectShape);
    }

    void SFMLRenderer::drawText(const std::string& text, engine::Vector2f position, uint32_t fontSize, Color color) {
        sf::Text textObj(m_font);
        textObj.setString(text);
        textObj.setCharacterSize(fontSize);
        textObj.setFillColor(sf::Color(color.r, color.g, color.b, color.a));
        textObj.setPosition({position.x, position.y});
        m_renderWindow.draw(textObj);
    }

    void SFMLRenderer::drawLine(engine::Vector2f start, engine::Vector2f end, Color color) {
        sf::Vertex line[] = {
            sf::Vertex(sf::Vector2f(start.x, start.y), sf::Color(color.r, color.g, color.b, color.a)),
            sf::Vertex(sf::Vector2f(end.x, end.y), sf::Color(color.r, color.g, color.b, color.a))
        };
        m_renderWindow.draw(line, 2, sf::PrimitiveType::Lines);
    }

    void SFMLRenderer::drawPolygon(const std::vector<engine::Vector2f>& vertices, const Color& color) {
        if (vertices.empty()) return;

        sf::VertexArray polygon(sf::PrimitiveType::TriangleFan, vertices.size());
        for (size_t i = 0; i < vertices.size(); ++i) {
            polygon[i].position = sf::Vector2f(vertices[i].x, vertices[i].y);
            polygon[i].color = sf::Color(color.r, color.g, color.b, color.a);
        }
        m_renderWindow.draw(polygon);
    }

    void SFMLRenderer::drawTexture(const void* textureHandle, engine::Vector2f position) {
        if (!textureHandle) return;
        
        // Unsafe cast: relying on caller to pass valid sf::Texture*
        const sf::Texture* texture = static_cast<const sf::Texture*>(textureHandle);
        sf::Sprite sprite(*texture);
        sprite.setPosition({position.x, position.y});
        
        m_renderWindow.draw(sprite);
    }

    void SFMLRenderer::drawTexture(const void* textureHandle, engine::Vector2f position, engine::Vector2f size) {
        if (!textureHandle) return;
        
        const sf::Texture* texture = static_cast<const sf::Texture*>(textureHandle);
        sf::Sprite sprite(*texture);
        sprite.setPosition({position.x, position.y});
        
        // Calculate scaling factors to fit target size
        sf::Vector2u texSize = texture->getSize();
        if (texSize.x > 0 && texSize.y > 0) {
            sprite.setScale({
                size.x / static_cast<float>(texSize.x),
                size.y / static_cast<float>(texSize.y)
            });
        }
        
        m_renderWindow.draw(sprite);
    }

    void SFMLRenderer::drawSpriteDirect(const void* spriteHandle, engine::Vector2f position) {
        if (!spriteHandle) return;
        
        const sf::Sprite* sprite = static_cast<const sf::Sprite*>(spriteHandle);
        
        // Create a copy to modify position without affecting original
        sf::Sprite copy = *sprite;
        copy.setPosition({position.x, position.y});
        m_renderWindow.draw(copy);
    }

    void SFMLRenderer::drawSprite(const SpriteDesc& spriteDesc, const TransformComponent& transform, const Color& color) {
        if (!m_resourceManager) {
            if (m_logger) m_logger->error("SFMLRenderer: ResourceManager is not set. Cannot draw sprite.");
            return;
        }

        // Resolve Texture ID to concrete SFML Texture
        auto texture = m_resourceManager->getTexture(spriteDesc.textureId);
        if (!texture) {
            // Error already logged by ResourceManager
            return;
        }

        sf::Sprite sprite(*texture);
        sprite.setTextureRect(spriteDesc.uvRect);
        sprite.setOrigin(spriteDesc.origin);
        
        // Apply ECS Transform
        sprite.setPosition(sf::Vector2f(transform.position.x, transform.position.y));
        sprite.setRotation(sf::radians(transform.rotation)); // SFML 3.x uses Radians by default
        sprite.setScale({transform.scale.x, transform.scale.y});
        
        sprite.setColor(sf::Color(color.r, color.g, color.b, color.a));

        m_renderWindow.draw(sprite);
    }

    void SFMLRenderer::setViewCenter(engine::Vector2f center) {
        sf::View view = m_renderWindow.getView();
        view.setCenter({center.x, center.y});
        m_renderWindow.setView(view);
    }

    engine::Vector2f SFMLRenderer::getViewCenter() const {
        const sf::View& view = m_renderWindow.getView();
        return { view.getCenter().x, view.getCenter().y };
    }

    void SFMLRenderer::setViewSize(engine::Vector2f size) {
        sf::View view = m_renderWindow.getView();
        view.setSize({ size.x, size.y });
        m_renderWindow.setView(view);
    }

    engine::Vector2u SFMLRenderer::getWindowSize() const {
        return getSize();
    }

    engine::Vector2f SFMLRenderer::screenToWorld(engine::Vector2f screenPos) {
        sf::Vector2i pixelPos(static_cast<int>(screenPos.x), static_cast<int>(screenPos.y));
        // Use SFML's built-in coordinate mapping which accounts for current View (camera)
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