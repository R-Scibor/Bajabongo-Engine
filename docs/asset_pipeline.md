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