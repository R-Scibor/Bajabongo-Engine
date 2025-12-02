import os
import json
import re
from PIL import Image  # pip install Pillow

# --- CONFIGURATION ---
TEXTURES_DIR = "../assets/textures"
OUTPUT_JSON = "../assets/data/resources.json"

# Keywords that trigger "Feet" origin (Bottom-Center)
FEET_ORIGIN_KEYWORDS = ["player", "npc", "enemy", "zombie", "char", "unit", "mob"]

def get_origin(width, height, filename):
    """
    Decides the pivot point based on filename keywords.
    Default: Center (w/2, h/2)
    Entities: Bottom-Center (w/2, h)
    """
    name_lower = filename.lower()
    if any(k in name_lower for k in FEET_ORIGIN_KEYWORDS):
        return [width / 2.0, float(height)] # Feet
    return [width / 2.0, height / 2.0]      # Center

def parse_grid_dimensions(filename):
    """
    Looks for pattern '_WxH' in filename (e.g., 'hero_run_32x32.png').
    Returns (frame_w, frame_h) or None.
    """
    # Regex looks for _digitsXdigits just before the extension
    match = re.search(r'_(\d+)x(\d+)', filename)
    if match:
        return int(match.group(1)), int(match.group(2))
    return None

def bake_resources():
    print(f"[-] Baking resources (Advanced Mode)...")
    
    data = {"textures": [], "animations": []} # We keep animations in root for your format? 
    # NOTE: Your JSON structure shows animations INSIDE textures. I will stick to that.
    
    existing_map = {}
    if os.path.exists(OUTPUT_JSON):
        try:
            with open(OUTPUT_JSON, 'r') as f:
                existing_data = json.load(f)
                for tex in existing_data.get("textures", []):
                    existing_map[tex["id"]] = tex
        except Exception:
            pass

    for root, dirs, files in os.walk(TEXTURES_DIR):
        for file in files:
            if not file.lower().endswith(('.png', '.jpg', '.jpeg')):
                continue
                
            full_path = os.path.join(root, file)
            rel_path = os.path.relpath(full_path, os.path.dirname(OUTPUT_JSON)).replace("\\", "/")
            
            file_stem = os.path.splitext(file)[0]
            texture_id = f"{file_stem}_texture"
            
            try:
                with Image.open(full_path) as img:
                    img_w, img_h = img.size
            except:
                continue

            # 1. Get or Create Texture Entry
            entry = existing_map.get(texture_id, {
                "id": texture_id,
                "path": rel_path,
                "sprites": [],
                "animations": []
            })
            
            # Always update path in case of movement
            entry["path"] = rel_path

            # 2. Check for Grid/Animation Pattern (e.g. _32x32)
            grid_dims = parse_grid_dimensions(file_stem)
            
            if grid_dims:
                frame_w, frame_h = grid_dims
                
                # Validate math
                if img_w % frame_w != 0 or img_h % frame_h != 0:
                    print(f"[!] WARNING: {file} size ({img_w}x{img_h}) not divisible by frame size {grid_dims}!")
                
                cols = img_w // frame_w
                rows = img_h // frame_h
                total_frames = cols * rows
                anim_id = f"{file_stem}_anim"

                # Check if this animation already exists
                has_anim = any(a["id"] == anim_id for a in entry.get("animations", []))
                
                if not has_anim:
                    print(f"[+] Detected SpriteSheet: {file} -> {cols}x{rows} grid")
                    entry["animations"].append({
                        "id": anim_id,
                        "frameStart": [0, 0], # Starts at 0,0
                        "frameSize": [frame_w, frame_h],
                        "frameCount": total_frames,
                        "columns": cols,
                        "duration": 0.1, # Default duration
                        "loop": True
                    })
            
            else:
                # 3. Handle Static Sprite (Non-Grid)
                # If it's NOT a grid, we assume it's a single static image
                sprite_id = f"{file_stem}_sprite"
                
                # Find existing sprite to preserve custom data
                target_sprite = next((s for s in entry["sprites"] if s["id"] == sprite_id), None)
                
                if target_sprite:
                    # Update Region only
                    target_sprite["region"] = [0, 0, img_w, img_h]
                else:
                    # Create New
                    origin = get_origin(img_w, img_h, file_stem)
                    entry["sprites"].append({
                        "id": sprite_id,
                        "region": [0, 0, img_w, img_h],
                        "origin": origin
                    })

            data["textures"].append(entry)

    # Write
    with open(OUTPUT_JSON, 'w') as f:
        json.dump(data, f, indent=4)
    print(f"[-] Done. Processed {len(data['textures'])} textures.")

if __name__ == "__main__":
    bake_resources()
