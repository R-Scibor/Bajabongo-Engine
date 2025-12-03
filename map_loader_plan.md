# Map Loader Implementation Plan

## Objective
Create a temporary map parser to load `map.json` (Tiled Map export), load the background image, and create static Box2D bodies for collision objects.

## Files to Create

### 1. `src/engine/assets/MapLoader.hpp`

```cpp
#pragma once

#include <string>
#include <memory>
#include "engine/core/EngineContext.hpp"

namespace engine {
    class ILogger;

    class MapLoader {
    public:
        explicit MapLoader(EngineContext& context);
        ~MapLoader() = default;

        bool load(const std::string& filepath);

    private:
        EngineContext& m_context;
        std::shared_ptr<ILogger> m_logger;
        
        void createCollisionBody(const struct MapObject& obj);
    };
}
```

### 2. `src/engine/assets/MapLoader.cpp`

- Include dependencies: `nlohmann/json.hpp`, `box2d/box2d.h`, `engine/components/PendingPhysicsBodyComponent.hpp`, `engine/components/TransformComponent.hpp`, `engine/components/RenderableComponent.hpp`.
- **load()**:
    - Parse JSON file.
    - Extract "Image Layer 1" properties:
        - `image`: "Map_milestone.png"
        - `width`/`height`
        - Create an entity for the background image:
            - `TransformComponent`: Position (0,0) or centered based on map size? Tiled coordinates are usually top-left.
            - `RenderableComponent`: Use the map texture.
    - Extract "Collision" object group:
        - Iterate through `objects` array.
        - For each object, extract `x`, `y`, `width`, `height`.
        - Create a static physics body.
        - Note: Tiled objects origin is top-left, but Box2D uses center. Adjust coordinates:
            - Center X = `x + width / 2`
            - Center Y = `y + height / 2`
        - Create entity with `PendingPhysicsBodyComponent` (static).
        - Optional: Add debug `RenderableComponent` (e.g., red box) if needed, or rely on debug draw.

## Project Configuration Updates

### `src/engine/engine.vcxproj`

Add entries for `MapLoader.hpp` and `MapLoader.cpp`.

**ItemGroup (ClInclude):**
```xml
<ClInclude Include="assets\MapLoader.hpp" />
```

**ItemGroup (ClCompile):**
```xml
<ClCompile Include="assets\MapLoader.cpp" />
```

### `src/engine/engine.vcxproj.filters`

**Header Files Filter:**
```xml
<ClInclude Include="assets\MapLoader.hpp">
  <Filter>Header Files</Filter>
</ClInclude>
```

**Source Files Filter:**
```xml
<ClCompile Include="assets\MapLoader.cpp">
  <Filter>Source Files</Filter>
</ClCompile>
```

## GameplayState Integration

In `GameplayState::onEnter`:
1. Instantiate `MapLoader`.
2. Call `load("assets/map.json")`.