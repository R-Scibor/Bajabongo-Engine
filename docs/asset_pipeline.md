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
*   **Behavior**: Creates a `Sprite` entry.
*   **Default Origin**: Center `[width/2, height/2]`.

**Example**:
*   `box.png` (64x64) → Origin: `[32, 32]`

### 2. Entities (Feet Alignment)
Used for characters or objects that require Z-sorting/Y-sorting based on their "feet" or base.
*   **Keywords**: The filename must contain one of: `player`, `npc`, `enemy`, `zombie`, `char`, `unit`, `mob`.
*   **Behavior**: Creates a `Sprite` entry.
*   **Default Origin**: Bottom-Center `[width/2, height]`.

**Example**:
*   `zombie_idle.png` (32x64) → Origin: `[16, 64]`

### 3. Sprite Sheets (Animations)
Used for frame-based animations. The frame size **must** be specified in the filename.
*   **Format**: `name_action_WxH.png` (e.g., `hero_run_32x32.png`)
*   **Behavior**: 
    *   Parses the `_WxH` suffix to determine frame size.
    *   Calculates rows and columns based on total image size.
    *   Creates an `Animation` entry.
    *   Defaults: `loop: true`, `duration: 0.1s`.

**Examples**:
*   `player_run_32x32.png` (Image: 256x32)
    *   Frame Size: 32x32
    *   Result: 8 Frames, 1 Row.
*   `explosion_64x64.png` (Image: 256x256)
    *   Frame Size: 64x64
    *   Result: 16 Frames, 4 Rows.

## Manual Overrides

The tool is **non-destructive**. You can manually edit `resources.json` to fine-tune properties that the script creates.

**Workflow**:
1.  Run `AssetBaker.py` to generate the initial entry.
2.  Open `assets/data/resources.json`.
3.  Modify fields (e.g., set `"loop": false` or adjust `"origin"`).
4.  Future runs of the tool will **preserve** your changes, only updating file paths and image dimensions if the source file has changed.