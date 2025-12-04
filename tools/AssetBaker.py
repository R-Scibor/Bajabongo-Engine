import os
import json
import re
from PIL import Image

# --- CONFIG ---
TEXTURES_DIR = "../assets/textures"
OUTPUT_JSON = "../assets/data/resources.json"
JSON_PATH_PREFIX = "../../assets/textures/"

FEET_ORIGIN_KEYWORDS = ["player", "npc", "enemy", "zombie", "char", "unit", "mob"]

name_size_re = re.compile(r"^(.*)_(\d+)x(\d+)$")

def split_name_and_size(stem: str):
    """
    'player_walk_128x128' -> ('player_walk', 128, 128)
    'box' -> ('box', None, None)
    """
    m = name_size_re.match(stem)
    if not m:
        return stem, None, None
    base = m.group(1)
    return base, int(m.group(2)), int(m.group(3))

def get_origin(w, h, base_name):
    low = base_name.lower()
    if any(k in low for k in FEET_ORIGIN_KEYWORDS):
        return [w / 2.0, float(h)]   # feet
    return [w / 2.0, h / 2.0]        # center

def bake_resources():
    print(f"[AssetBaker] scanning {TEXTURES_DIR} ...")

    # Load existing resources to preserve manual edits (origins, regions, etc.)
    existing_textures = {}
    if os.path.exists(OUTPUT_JSON):
        try:
            with open(OUTPUT_JSON, "r") as f:
                data = json.load(f)
                for t in data.get("textures", []):
                    existing_textures[t["id"]] = t
        except Exception as e:
            print(f"[AssetBaker] Warning: could not load existing {OUTPUT_JSON}: {e}")

    textures = []
    tex_count = 0

    for root, dirs, files in os.walk(TEXTURES_DIR):
        for file in files:
            if not file.lower().endswith((".png", ".jpg", ".jpeg")):
                continue

            full_path = os.path.join(root, file)
            rel_inside = os.path.relpath(full_path, TEXTURES_DIR).replace("\\", "/")
            json_path = JSON_PATH_PREFIX + rel_inside

            stem = os.path.splitext(file)[0]          # e.g. player_walk_128x128
            base_name, sx, sy = split_name_and_size(stem)  # e.g. player_walk, 128,128

            tex_id = f"{base_name}_texture"
            sprite_id = f"{base_name}_sprite"
            anim_id = f"{base_name}_anim"

            try:
                with Image.open(full_path) as img:
                    w, h = img.size
            except Exception as e:
                print(f"[AssetBaker] ERROR reading {full_path}: {e}")
                continue

            entry = {
                "id": tex_id,
                "path": json_path,
                "sprites": [],
                "animations": []
            }

            # Generate default data from file
            new_sprites = []
            new_anims = []

            # Decide if this is an animation sheet or a static sprite
            if sx and sy and w % sx == 0 and h % sy == 0 and (w > sx or h > sy):
                # Spritesheet -> animation
                cols = max(1, w // sx)
                rows = max(1, h // sy)
                frame_count = cols * rows

                new_anims.append({
                    "id": anim_id,
                    "frameStart": [0, 0],
                    "frameSize": [sx, sy],
                    "frameCount": frame_count,
                    "columns": cols,
                    "duration": 0.1,
                    "loop": True
                })
            else:
                # Single sprite (full image)
                origin = get_origin(w, h, base_name)
                new_sprites.append({
                    "id": sprite_id,
                    "region": [0, 0, w, h],
                    "origin": origin
                })

            # Merge with existing data if available
            if tex_id in existing_textures:
                old_tex = existing_textures[tex_id]
                
                # Preserve sprites: keep all old ones, add new ones if ID missing
                old_sprites_list = old_tex.get("sprites", [])
                old_sprite_ids = set(s["id"] for s in old_sprites_list)
                
                # Start with old sprites (preserving their settings)
                entry["sprites"] = list(old_sprites_list)
                
                # Add generated ones only if they don't exist
                for s in new_sprites:
                    if s["id"] not in old_sprite_ids:
                        entry["sprites"].append(s)

                # Preserve animations: keep all old ones, add new ones if ID missing
                old_anims_list = old_tex.get("animations", [])
                old_anim_ids = set(a["id"] for a in old_anims_list)
                
                entry["animations"] = list(old_anims_list)
                
                for a in new_anims:
                    if a["id"] not in old_anim_ids:
                        entry["animations"].append(a)
            else:
                # No existing data, use generated
                entry["sprites"] = new_sprites
                entry["animations"] = new_anims

            textures.append(entry)
            tex_count += 1

    out = {
        "textures": textures,
        "animations": []  # keep for future use
    }

    with open(OUTPUT_JSON, "w") as f:
        json.dump(out, f, indent=4)

    print(f"[AssetBaker] done. {tex_count} textures written to {OUTPUT_JSON}")

if __name__ == "__main__":
    bake_resources()
