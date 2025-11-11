#pragma once
#include "engine/core/math/MathAliases.hpp"
#include <cstdint> // For uint8_t

// Forward declaration to avoid circular includes
class IWindow;

/**
 * @brief A simple color structure.
 */
struct Color {
    uint8_t r, g, b, a;
};

/**
 * @brief Defines a pure abstract interface for a renderer.
 * This decouples the engine from any specific rendering library (like SFML Graphics).
 * The Application core will only interact with this interface.
 */
class IRenderer {
public:
    /**
     * @brief Constructs the renderer, injecting the window it will draw to.
     * @param window A reference to the IWindow implementation.
     */
    explicit IRenderer(IWindow& window) : m_window(window) {}
    virtual ~IRenderer() = default;

    /**
     * @brief Prepares the renderer for a new frame.
     */
    virtual void beginFrame() = 0;

    /**
     * @brief Clears the render target with a specified color.
     * @param color The color to clear the screen with.
     */
    virtual void clear(Color color) = 0;

    /**
     * @brief Temporary method for the Phase 1 milestone.
     * This will be replaced by a proper RenderSystem.
     */
    virtual void drawShape(engine::Vector2f position, float radius) = 0;

    /**
     * @brief Finalizes the frame and displays it on the screen.
     */
    virtual void endFrame() = 0;

protected:
    // A reference to the window interface, injected via the constructor.
    IWindow& m_window;
};