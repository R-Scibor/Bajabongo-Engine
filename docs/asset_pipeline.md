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

The pipeline uses **hyphen-delimited grid dimensions** in filenames to automatically generate asset metadata.

### 1. Map Backgrounds
Large background images for levels.
- **Format**: `map_*.png` (must start with `map_`)
- **Origin**: Top-Left `[0, 0]`
- **Example**: `map_hideout.png` (1024×1024) → Origin: `[0, 0]`

### 2. Static Sprites
Props, UI elements, and other single-image assets.
- **Format**: `filename.png`
- **Origin**: Center `[w/2, h/2]`
- **Example**: `box.png` (64×64) → Origin: `[32, 32]`

### 3. Entities (Feet Alignment)
Characters and objects that need Y-sorting. Filename must contain one of: `player`, `npc`, `enemy`, `zombie`, `char`, `unit`, `mob`.
- **Format**: `basename.png`
- **Origin**: Bottom-Center `[w/2, h]` (feet position)
- **Example**: `zombie_idle.png` (32×64) → Origin: `[16, 64]`

### 4. Strip Animations
Single-row sprite sheets.
- **Format**: `basename-N.png` (N = frame count)
- **Example**: `player_run-8.png` → 8 frames, horizontal strip
- **Grid**: 8 columns × 1 row

### 5. Grid Animations
Multi-row sprite sheets.
- **Format**: `basename-NxM.png` (N = columns, M = rows)
- **Example**: `explosion-4x4.png` → 16 frames in 4×4 grid
- **Example**: `enemy_walk-6x2.png` → 12 frames in 6×2 grid

**Notes**:
- Frame dimensions are **calculated automatically** from image size ÷ grid dimensions
- Animation defaults: `loop: true`, `duration: 0.1s` per frame
- To override duration/loop, edit `resources.json` manually (changes are preserved on re-bake)
- Origin rules apply to animations too (maps use `[0,0]`, entities use feet pivot)


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
    *   **Usage**: The background image of the level. The engine scales this by the map's `mapScale` value (default 1.5).
    *   **Requirements**: Must define dimensions in the JSON: `"imageheight": 1024, "imagewidth": 1024`.

2.  **Map Properties (Root)**:
    *   **mapScale** (Float): Optional. Defines the global scaling factor for the map visual and physics (default: 1.5).
    *   Example: `{ "mapScale": 1.5, ... }` in the JSON root.

3.  **Collision Layer (Walls)**:
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
