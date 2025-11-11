#pragma once
#include "engine/core/math/MathAliases.hpp"
#include "engine/rendering/IRenderer.hpp"
#include "engine/core/IWindow.hpp"
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <memory> // Dla std::unique_ptr

/**
 * @brief Konkretna implementacja interfejsów IRenderer i IWindow używająca SFML.
 *
 * Ponieważ sf::RenderWindow łączy w sobie obsługę okna i renderowania,
 * ta jedna klasa implementuje oba abstrakcyjne interfejsy.
 */
class SFMLRenderer : public IRenderer, public IWindow {
public:
    /**
     * @brief Konstruktor.
     * Przekazuje 'this' do konstruktora bazy IRenderer, ponieważ
     * ta klasa jest teraz również IWindow.
     */
    SFMLRenderer();
    ~SFMLRenderer() override;

    // --- Implementacja interfejsu IWindow ---
    void create(const std::string& title, unsigned int width, unsigned int height) override;
    void close() override;
    bool isOpen() const override;
    void pollEvents() override;
    void* getNativeHandle() const override;

    // --- Implementacja interfejsu IRenderer ---
    void beginFrame() override;
    void clear(Color color) override;
    void drawShape(engine::Vector2f position, float radius) override;
    void endFrame() override;

private:
    // Mamy teraz tylko jeden obiekt okna, który robi wszystko.
    sf::RenderWindow m_renderWindow;

    // Tymczasowy kształt dla kamienia milowego Fazy 1
    sf::CircleShape m_shape;
};