#!/usr/bin/env python3
"""
Bajabongo Engine - Asset Baker
Scans assets/textures, assets/sounds, assets/music and generates resources.json manifest.

Naming Conventions:
  - Static sprites: filename.png
  - Strip animations: basename-N.png (e.g., player_run-8.png)
  - Grid animations: basename-NxM.png (e.g., zombie_walk-4x8.png)
  - Maps: map_*.png (origin set to [0, 0])
  - Sounds: filename.wav/ogg (short effects)
  - Music: filename.ogg/wav (long tracks)
"""

import os
import json
import re
import sys
from pathlib import Path
from PIL import Image

# Patterns
STRIP_PATTERN = re.compile(r'^(.+?)-(\d+)\.png$')
GRID_PATTERN = re.compile(r'^(.+?)-(\d+)x(\d+)\.png$')

# Entity keywords for feet-based pivot
ENTITY_KEYWORDS = ["player", "npc", "enemy", "zombie", "char", "unit", "mob"]

# Paths
TEXTURE_DIR = Path("../assets/textures")
SOUND_DIR = Path("../assets/sounds")
MUSIC_DIR = Path("../assets/music")
OUTPUT_PATH = Path("../assets/data/resources.json")


def is_map(basename: str) -> bool:
    """Check if the asset is a map background."""
    return basename.lower().startswith("map_")


def is_entity(basename: str) -> bool:
    """Check if the asset is an entity (for feet-based pivot)."""
    lower = basename.lower()
    return any(keyword in lower for keyword in ENTITY_KEYWORDS)


def calculate_origin(basename: str, width: int, height: int) -> list:
    """Calculate origin based on asset type."""
    if is_map(basename):
        return [0.0, 0.0]  # Top-left for maps
    elif is_entity(basename):
        return [width / 2.0, float(height)]  # Feet for entities
    else:
        return [width / 2.0, height / 2.0]  # Center for everything else


def process_static_sprite(filepath: Path, img_width: int, img_height: int) -> dict:
    """Generate texture entry for a static sprite."""
    basename = filepath.stem
    texture_id = f"{basename}_texture"
    sprite_id = f"{basename}_sprite"
    
    origin = calculate_origin(basename, img_width, img_height)
    
    return {
        "id": texture_id,
        "path": f"../../assets/textures/{filepath.name}",
        "sprites": [
            {
                "id": sprite_id,
                "region": [0, 0, img_width, img_height],
                "origin": origin
            }
        ],
        "animations": []
    }


def process_animation(filepath: Path, img_width: int, img_height: int, cols: int, rows: int) -> dict:
    """Generate texture entry for an animation (strip or grid)."""
    basename = filepath.stem.rsplit('-', 1)[0]  # Remove grid suffix
    texture_id = f"{basename}_texture"
    anim_id = f"{basename}_anim"
    
    frame_width = img_width // cols
    frame_height = img_height // rows
    frame_count = cols * rows
    
    origin = calculate_origin(basename, frame_width, frame_height)
    
    return {
        "id": texture_id,
        "path": f"../../assets/textures/{filepath.name}",
        "sprites": [],
        "animations": [
            {
                "id": anim_id,
                "frameStart": [0, 0],
                "frameSize": [frame_width, frame_height],
                "origin": origin,
                "frameCount": frame_count,
                "columns": cols,
                "duration": 0.1,  # Default per-frame duration
                "loop": True
            }
        ]
    }


def scan_textures() -> list:
    """Scan texture directory and generate texture entries."""
    textures = []
    
    if not TEXTURE_DIR.exists():
        print(f"ERROR: Texture directory not found: {TEXTURE_DIR}")
        return []
    
    for filepath in sorted(TEXTURE_DIR.glob("*.png")):
        filename = filepath.name
        
        # Open image to get dimensions
        try:
            with Image.open(filepath) as img:
                img_width, img_height = img.size
        except Exception as e:
            print(f"WARNING: Failed to read {filename}: {e}")
            continue
        
        # Check for grid pattern
        grid_match = GRID_PATTERN.match(filename)
        if grid_match:
            basename = grid_match.group(1)
            cols = int(grid_match.group(2))
            rows = int(grid_match.group(3))
            
            if cols < 1 or rows < 1:
                print(f"WARNING: Invalid grid dimensions in {filename} ({cols}x{rows})")
                continue
            
            print(f"[ANIM] {filename} → {cols}x{rows} grid ({cols * rows} frames)")
            textures.append(process_animation(filepath, img_width, img_height, cols, rows))
            continue
        
        # Check for strip pattern
        strip_match = STRIP_PATTERN.match(filename)
        if strip_match:
            basename = strip_match.group(1)
            cols = int(strip_match.group(2))
            rows = 1
            
            if cols < 2:
                print(f"WARNING: Strip animations need at least 2 frames: {filename}")
                continue
            
            print(f"[ANIM] {filename} → {cols}-frame strip")
            textures.append(process_animation(filepath, img_width, img_height, cols, rows))
            continue
        
        # Static sprite
        sprite_type = "MAP" if is_map(filepath.stem) else "SPRITE"
        print(f"[{sprite_type}] {filename} → static sprite")
        textures.append(process_static_sprite(filepath, img_width, img_height))

        # Generate single-frame animation clip for static sprites that might be used as states
        if not is_map(filepath.stem):
             basename = filepath.stem
             anim_id = basename 
             
             last_texture = textures[-1]
             
             origin = calculate_origin(basename, img_width, img_height)

             last_texture["animations"].append({
                "id": anim_id,
                "frameStart": [0, 0],
                "frameSize": [img_width, img_height],
                "origin": origin,
                "frameCount": 1,
                "columns": 1,
                "duration": 0.1,
                "loop": True
             })
             print(f"[GEN] Generated 1-frame clip: {anim_id}")
    
    return textures


def scan_audio(directory: Path, asset_type: str) -> list:
    """Scan audio directory and generate entries."""
    entries = []
    
    if not directory.exists():
        # It's fine if the directory doesn't exist yet, just return empty list
        return []
        
    for filepath in sorted(directory.glob("*.*")):
        if filepath.suffix.lower() not in ['.wav', '.ogg', '.mp3', '.flac']:
            continue
            
        print(f"[{asset_type.upper()}] {filepath.name}")
        entries.append({
            "id": filepath.stem,
            "path": f"../../assets/{asset_type}/{filepath.name}"
        })
    return entries


def merge_with_existing(new_textures: list, new_sounds: list, new_music: list) -> dict:
    """Merge new data with existing manifest (preserve manual edits)."""
    if not OUTPUT_PATH.exists():
        return {"textures": new_textures, "sounds": new_sounds, "music": new_music, "animations": []}
    
    try:
        with open(OUTPUT_PATH, 'r') as f:
            existing = json.load(f)
    except Exception as e:
        print(f"WARNING: Failed to load existing manifest: {e}")
        return {"textures": new_textures, "sounds": new_sounds, "music": new_music, "animations": []}
    
    # Merge Textures (preserve animations)
    existing_tex_map = {tex["id"]: tex for tex in existing.get("textures", [])}
    merged_textures = []
    
    for new_tex in new_textures:
        tex_id = new_tex["id"]
        if tex_id in existing_tex_map:
            old_tex = existing_tex_map[tex_id]
            # Preserve manual animation overrides
            if new_tex["animations"] and old_tex.get("animations"):
                for new_anim in new_tex["animations"]:
                    for old_anim in old_tex["animations"]:
                        if new_anim["id"] == old_anim["id"]:
                            new_anim["duration"] = old_anim.get("duration", 0.1)
                            new_anim["loop"] = old_anim.get("loop", True)
        merged_textures.append(new_tex)
    
    # Merge Sounds and Music (simple overwrite for now as they have no complex properties yet)
    # If we add properties like "volume" or "defaultLoop", we would need logic here.
    
    return {
        "textures": merged_textures,
        "sounds": new_sounds,
        "music": new_music,
        "animations": existing.get("animations", [])
    }


def main():
    print("=== Bajabongo Asset Baker ===")
    print(f"Scanning: {TEXTURE_DIR.absolute()}")
    
    textures = scan_textures()
    sounds = scan_audio(SOUND_DIR, "sounds")
    music = scan_audio(MUSIC_DIR, "music")
    
    if not textures and not sounds and not music:
        print("No assets found.")
        # Don't return, allow writing empty manifest if needed, or just keep existing
    
    manifest = merge_with_existing(textures, sounds, music)
    
    # Write output
    OUTPUT_PATH.parent.mkdir(parents=True, exist_ok=True)
    with open(OUTPUT_PATH, 'w') as f:
        json.dump(manifest, f, indent=4)
    
    print(f"\n✅ Generated {OUTPUT_PATH}")
    print(f"   Textures: {len(textures)}")
    print(f"   Sounds:   {len(sounds)}")
    print(f"   Music:    {len(music)}")
    print(f"   Animations: {sum(len(t['animations']) for t in textures)}")


if __name__ == "__main__":
    main()
