# Core Math Library

This document outlines the fundamental, library-independent math types used throughout the Bajabongo-Engine. The primary goal of this library is to decouple the engine's core logic from any specific external math libraries (like SFML or Box2D), ensuring architectural purity and long-term flexibility.

## Design Philosophy: The Adapter Pattern

We strictly adhere to the **Adapter Pattern** for all interactions with third-party libraries.

1.  **Internal Language:** Engine and game logic use our internal types, such as `engine::Vector2f` and `engine::Rectf`, for positions, sizes, and geometry. Some rendering data still uses SFML types directly (`RenderableComponent::color` is `sf::Color`; `SpriteDesc` uses `sf::IntRect` and `sf::Vector2f`).
2.  **System Boundary:** Conversion to and from library-specific types (e.g., `sf::Vector2f`) happens **only** at the lowest possible level—inside the concrete implementation of an interface.

This keeps most of the codebase agnostic of the underlying rendering or physics engine.

### Example: `SFMLRenderer`

The renderer's public interface (`IRenderer`) accepts engine types:
`virtual void drawCircle(engine::Vector2f position, float radius) = 0;`

The conversion happens privately inside the `.cpp` file (simplified, origin setup omitted):
```cpp
// Inside SFMLRenderer.cpp
void SFMLRenderer::drawCircle(engine::Vector2f position, float radius)
{
    // 1. Adapt to the library's type
    sf::Vector2f sfmlPosition(position.x, position.y);

    // 2. Use the library
    m_circleShape.setPosition(sfmlPosition);
    m_circleShape.setRadius(radius);
    m_renderWindow.draw(m_circleShape);
}
```

## Available Types

All types are located in `engine/core/math/`.

### `Vector2<T>`
A simple template struct representing a 2D vector.
- **Members:** `T x`, `T y`
- **Header:** `engine/core/math/Vector2.hpp`

### `Rect<T>`
A template struct for an axis-aligned rectangle.
- **Members:** `T left`, `T top`, `T width`, `T height`
- **Header:** `engine/core/math/Rect.hpp`

### Type Aliases
For convenience, the following common aliases are provided in `engine/core/math/MathAliases.hpp`:
- `using Vector2f = Vector2<float>;`
- `using Vector2i = Vector2<int>;`
- `using Vector2u = Vector2<unsigned int>;`
- `using Rectf = Rect<float>;`
- `using Recti = Rect<int>;`

### Other helpers
- `Angle.hpp`: `Pi` and `wrapAngle`, which wraps an angle in radians to `[-Pi, Pi]`.
- `GeometryUtils.hpp`: `GeometryUtils::isPointInPolygon`.
- `Random.hpp`: `engine::Random`, the seedable RNG behind `--seed`.
- `MathJson.hpp`: nlohmann-json conversions for the math types.
