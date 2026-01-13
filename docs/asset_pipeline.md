# Asset Pipeline & Baking Tool

The Bajabongo Engine includes a specialized utility located in `tools/AssetBaker.py` to automate the management of game assets. This tool scans the `assets/textures/` directory and generates the `resources.json` manifest, serving as the bridge between raw assets and the engine's resource manager.

## Overview

The Asset Baker streamlines the workflow by:
1.  **Scanning Recursively**: It checks `assets/textures` and all subdirectories.
2.  **Auto-Generating Metadata**: It calculates dimensions and frames for sprites and animations.
3.  **Hot-Reloading Support**: It preserves manually tuned values (like animation speed or pivot points) while updating paths and dimensions.

## Usage

### Prerequisites
The tool requires Python and the `Pillow` library for image processing.

```bash
pip install Pillow
```

### Running the Baker
Execute the script from the `tools/` directory:

```bash
cd tools
python AssetBaker.py
```

The script will:
*   Scan `../assets/textures`.
*   Update or create `../assets/data/resources.json`.
*   Report processed textures and animations.

## Naming Conventions

The pipeline uses **filename conventions** to automatically determine asset types and properties. Adhering to these rules minimizes manual configuration.

### 1. Static Sprites
Used for standard props, UI elements, and backgrounds.
*   **Format**: `filename.png`
*   **Behavior**:
    *   Generates Texture ID: `filename_texture`.
    *   Generates Sprite ID: `filename_sprite`.
*   **Default Origin**: Center `[width/2, height/2]`.

**Example**:
*   `box.png` (64x64) → Texture: `box_texture`, Sprite: `box_sprite`, Origin: `[32, 32]`

### 2. Entities (Feet Alignment)
Used for characters or objects that require Z-sorting/Y-sorting based on their "feet" or base.
*   **Keywords**: The filename must contain one of: `player`, `npc`, `enemy`, `zombie`, `char`, `unit`, `mob`.
*   **Behavior**:
    *   Generates Texture ID: `filename_texture`.
    *   Generates Sprite ID: `filename_sprite`.
*   **Default Origin**: Bottom-Center `[width/2, height]`.

**Example**:
*   `zombie_idle.png` (32x64) → Origin: `[16, 64]`

### 3. Sprite Sheets (Animations)
Used for frame-based animations. The frame size **must** be specified in the filename.
*   **Format**: `basename_WxH.png` (e.g., `hero_run_32x32.png`)
*   **Behavior**:
    *   Parses the `_WxH` suffix to determine frame size.
    *   Generates Texture ID: `basename_texture` (e.g., `hero_run_texture`).
    *   Generates Animation ID: `basename_anim` (e.g., `hero_run_anim`).
    *   Calculates rows and columns based on total image size.
    *   Defaults: `loop: true`, `duration: 0.1s`.

**Examples**:
*   `player_run_32x32.png` (Image: 256x32)
    *   IDs: `player_run_texture`, `player_run_anim`
    *   Frame Size: 32x32
    *   Result: 8 Frames, 1 Row.
*   `explosion_64x64.png` (Image: 256x256)
    *   Frame Size: 64x64
    *   Result: 16 Frames, 4 Rows.

## Auto-Generation Note

The `resources.json` file is **fully auto-generated** by the `AssetBaker.py` tool.

*   **Do not manually edit** `resources.json`, as your changes will be overwritten the next time the baker is run.
*   All properties (IDs, regions, origins) are derived directly from filenames and image dimensions.

## Map Creation in Tiled

The Bajabongo Engine uses [Tiled Map Editor](https://www.mapeditor.org/) for level design. The engine parses the JSON export format from Tiled.

### Layer Structure

The `MapLoader` expects specific layers to function correctly. Ensure your Tiled map follows this structure:

1.  **Image Layer (Background)**:
    *   **Type**: `Image Layer`
    *   **Name**: Any
    *   **Usage**: The background image of the level. The engine scales this by 1.5x by default.

2.  **Collision Layer (Walls)**:
    *   **Type**: `Object Layer`
    *   **Name**: `Collision` (Case-sensitive)
    *   **Usage**: Draw rectangles or polygons here to define solid walls.
    *   **Physics**: These objects become static `Wall` bodies.

3.  **HalfCollision Layer (Low Obstacles)**:
    *   **Type**: `Object Layer`
    *   **Name**: `HalfCollision` (Case-sensitive)
    *   **Usage**: Draw shapes for low obstacles like crates or tables.
    *   **Physics**: These become `LowObstacle` bodies. Players/Enemies collide, but Projectiles pass over.

4.  **Entities Layer (Spawns)**:
    *   **Type**: `Object Layer`
    *   **Name**: `Entities` (Case-sensitive)
    *   **Usage**: Place Points or Rectangles to spawn game entities.
    *   **Properties**:
        *   `class` (or `type` in older Tiled versions): (Required) The Archetype ID to spawn (e.g., `Player`, `Enemy_Zombie`, `Portal_Door`).
        *   If `class` is missing, it tries to use the object's `name` as the type.
        *   **Position**: The engine spawns the entity at the object's position.
            *   For Point objects: `(x, y)`
            *   For Rect objects: Center `(x + w/2, y + h/2)`

### Workflow
1.  Create a map in Tiled.
2.  Set up the layers as described above.
3.  Export the map as **JSON** to `assets/data/map.json`.
4.  Run the game; the `MapLoader` will parse the file and build the physics world and entities.
