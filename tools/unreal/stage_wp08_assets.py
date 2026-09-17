"""
Stages authentic WP-08 camp, building, and storage assets from local archives.
- Voxel Knights Chest FBX and PNG from knights.zip -> Saved/Staging/WP08
- Stylized Wood building meshes from EasyBuildingSystemV10 -> Content/WYRMFALL/Environment/Building/Wood
"""

import os
import shutil
import zipfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent.parent
STAGING_DIR = ROOT / "Saved/Staging/WP08"
STAGING_DIR.mkdir(parents=True, exist_ok=True)

print(f"Staging WP-08 assets into: {STAGING_DIR}")

# 1. Extract Voxel Chest from knights.zip
KNIGHTS_ZIP = ROOT / "assets and old docs/voxel/characters/knights.zip"
if not KNIGHTS_ZIP.is_file():
    raise FileNotFoundError(f"knights.zip not found at: {KNIGHTS_ZIP}")

with zipfile.ZipFile(KNIGHTS_ZIP, 'r') as z:
    for filename in z.namelist():
        if "TVS_VoxelKnights_Chest.fbx" in filename:
            target = STAGING_DIR / "TVS_VoxelKnights_Chest.fbx"
            with open(target, 'wb') as f:
                f.write(z.read(filename))
            print(f"Extracted: {target.name} ({target.stat().st_size} bytes)")
        elif "TVS_VoxelKnights_Chest_Texture.png" in filename:
            target = STAGING_DIR / "TVS_VoxelKnights_Chest_Texture.png"
            with open(target, 'wb') as f:
                f.write(z.read(filename))
            print(f"Extracted: {target.name} ({target.stat().st_size} bytes)")

# 2. Copy Stylized Wood building meshes and textures from EasyBuildingSystemV10
EBS_SRC = ROOT / "assets and old docs/EasyBuildingSystemV10/Content/EasyBuildingSystem"
BUILDING_DEST = ROOT / "Content/WYRMFALL/Environment/Building/Wood"
BUILDING_DEST.mkdir(parents=True, exist_ok=True)

WOOD_MESHES = [
    "SM_Stylized_Wood_Foundation.uasset",
    "SM_Stylized_Wood_Wall.uasset",
    "SM_Stylized_Wood_Doorframe.uasset",
    "SM_Stylized_Wood_Door.uasset",
    "SM_Stylized_Wood_Roof.uasset",
    "SM_Stylized_Wood_Ceiling.uasset",
]

mesh_src_dir = EBS_SRC / "Meshes/Structures/Stylized/Wood"
for mesh_name in WOOD_MESHES:
    src = mesh_src_dir / mesh_name
    dest = BUILDING_DEST / mesh_name
    if src.is_file():
        shutil.copy2(src, dest)
        print(f"Copied building mesh: {mesh_name} ({dest.stat().st_size} bytes)")
    else:
        print(f"WARNING: Missing mesh {src}")

# Copy materials and textures for stylized wood
tex_src_dir = EBS_SRC / "Textures/Structures/Stylized/Wood"
if tex_src_dir.is_dir():
    for f in tex_src_dir.glob("*.uasset"):
        dest = BUILDING_DEST / f.name
        shutil.copy2(f, dest)
        print(f"Copied texture: {f.name}")

mat_src_dir = EBS_SRC / "Materials/Instances/Stylized"
if mat_src_dir.is_dir():
    for f in mat_src_dir.glob("*Wood*.uasset"):
        dest = BUILDING_DEST / f.name
        shutil.copy2(f, dest)
        print(f"Copied material: {f.name}")

print("=== WP-08 ASSET STAGING COMPLETE ===")
