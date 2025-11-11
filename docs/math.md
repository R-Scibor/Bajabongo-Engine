# Core Math Library

This document outlines the fundamental, library-independent math types used throughout the Bajabongo-Engine. The primary goal of this library is to decouple the engine's core logic from any specific external math libraries (like SFML or Box2D), ensuring architectural purity and long-term flexibility.

## Design Philosophy: The Adapter Pattern

We strictly adhere to the **Adapter Pattern** for all interactions with third-party libraries.

1.  **Internal Language:** All engine and game logic exclusively uses our internal types, such as `engine::Vector2f` and `engine::Rectf`.
2.  **System Boundary:** Conversion to and from library-specific types (e.g., `sf::Vector2f`) happens **only** at the lowest possible level—inside the concrete implementation of an interface.

This ensures that 99% of the codebase remains agnostic of the underlying rendering or physics engine.

### Example: `SFMLRenderer`

A rendering system's public interface will accept engine types:
`virtual void drawSprite(engine::Vector2f position, ...);`

The conversion happens privately inside the `.cpp` file:
```cpp
// Inside SFMLRenderer.cpp
void SFMLRenderer::drawSprite(engine::Vector2f enginePos, ...)
{
    // 1. Adapt to the library's type
    sf::Vector2f sfmlPos(enginePos.x, enginePos.y);

    // 2. Use the library
    m_sprite.setPosition(sfmlPos);
    m_window.draw(m_sprite);
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
- `using Rectf = Rect<float>;`
- `using Recti = Rect<int>;`