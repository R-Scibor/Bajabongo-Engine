#include "SFMLRenderer.h"
#include "engine/core/IWindow.h"
#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/WindowHandle.hpp>
#include <SFML/Window/Event.hpp>     
#include <SFML/Window/VideoMode.hpp> 
#include <optional>                 

SFMLRenderer::SFMLRenderer()
    : IRenderer(*this) {
    m_shape.setRadius(50.f);
    m_shape.setFillColor(sf::Color::Red);
    m_shape.setPosition(sf::Vector2f(100.f, 100.f));
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

void SFMLRenderer::pollEvents() {
    while (std::optional<sf::Event> event = m_renderWindow.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            close();
        }
    }
}

void* SFMLRenderer::getNativeHandle() const {
    return reinterpret_cast<void*>(m_renderWindow.getNativeHandle());
}

// --- Implementacja IRenderer ---

void SFMLRenderer::beginFrame() {
    m_renderWindow.setActive(true);
}

void SFMLRenderer::clear(Color color) {
    m_renderWindow.clear(sf::Color(color.r, color.g, color.b, color.a));
}

void SFMLRenderer::drawShape() {
    m_renderWindow.draw(m_shape);
}

void SFMLRenderer::endFrame() {
    m_renderWindow.display();
}