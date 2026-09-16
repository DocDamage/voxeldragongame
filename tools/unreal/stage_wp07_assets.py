import zipfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
STAGING = ROOT / "Saved/Staging/WP07"
STAGING.mkdir(parents=True, exist_ok=True)

print(f"Staging WP-07 assets to: {STAGING}")

# 1. Rangers zip (Campfire, Spear/Rod)
rangers_zip = ROOT / "assets and old docs/voxel/characters/rangers.zip"
with zipfile.ZipFile(rangers_zip) as z:
    for src, dst in [
        ("Voxel Rangers/FBX/Environment/TVS_VoxelRangers_Campfire_Frame1.fbx", "TVS_VoxelRangers_Campfire_Frame1.fbx"),
        ("Voxel Rangers/Textures/Environment/TVS_VoxelRangers_Campfire_Texture.png", "TVS_VoxelRangers_Campfire_Texture.png"),
        ("Voxel Rangers/FBX/Props/TVS_VoxelRangers_Spear.fbx", "TVS_VoxelRangers_Spear.fbx"),
        ("Voxel Rangers/Textures/Props/TVS_VoxelRangers_Spear_Texture.png", "TVS_VoxelRangers_Spear_Texture.png"),
    ]:
        data = z.read(src)
        (STAGING / dst).write_bytes(data)
        print(f"Extracted: {dst} ({len(data)} bytes)")

# 2. Farmers zip (Potato)
farmers_zip = ROOT / "assets and old docs/voxel/characters/farmers.zip"
with zipfile.ZipFile(farmers_zip) as z:
    for src, dst in [
        ("Voxel Farm/FBX/Environment/TVS_VoxelFarm_Potato.fbx", "TVS_VoxelFarm_Potato.fbx"),
        ("Voxel Farm/Textures/Environment/TVS_VoxelFarm_Potato_Texture.png", "TVS_VoxelFarm_Potato_Texture.png"),
    ]:
        data = z.read(src)
        (STAGING / dst).write_bytes(data)
        print(f"Extracted: {dst} ({len(data)} bytes)")

# 3. Hexatiles zip (Ocean fish)
hexa_zip = ROOT / "assets and old docs/voxel/hexagon set/Hexatilesv1.zip"
with zipfile.ZipFile(hexa_zip) as z:
    for src, dst in [
        ("Obj/overworld-47-ocean_fish.obj", "overworld-47-ocean_fish.obj"),
        ("Obj/overworld-47-ocean_fish.mtl", "overworld-47-ocean_fish.mtl"),
        ("Obj/overworld-47-ocean_fish.png", "overworld-47-ocean_fish.png"),
    ]:
        data = z.read(src)
        (STAGING / dst).write_bytes(data)
        print(f"Extracted: {dst} ({len(data)} bytes)")

# 4. Aquatic pack (Jetty, Water Lily)
water_zip = ROOT / "assets and old docs/voxel/Voxel_Water___Aquatic_Pack_-_115_Assets__Static___Animated_-8b7379eb.zip"
with zipfile.ZipFile(water_zip) as z:
    for src, dst in [
        ("Voxel_Water___Aquatic_Pack_-_115_Assets__Static___Animated_-8b7379eb/fbx/free_water_fbx_extracted/Models/Static/jetty/jetty.fbx", "jetty.fbx"),
        ("Voxel_Water___Aquatic_Pack_-_115_Assets__Static___Animated_-8b7379eb/fbx/free_water_fbx_extracted/Models/Static/jetty/palette.png", "T_Jetty.png"),
        ("Voxel_Water___Aquatic_Pack_-_115_Assets__Static___Animated_-8b7379eb/fbx/free_water_fbx_extracted/Models/Static/water_lily/water_lily.fbx", "water_lily.fbx"),
        ("Voxel_Water___Aquatic_Pack_-_115_Assets__Static___Animated_-8b7379eb/fbx/free_water_fbx_extracted/Models/Static/water_lily/palette.png", "T_WaterLily.png"),
    ]:
        data = z.read(src)
        (STAGING / dst).write_bytes(data)
        print(f"Extracted: {dst} ({len(data)} bytes)")

# 5. Fishing Minigame SFX
fish_sfx_zip = ROOT / "assets and old docs/voxel/Fishing_Minigame_SFX_120_Sounds.zip"
with zipfile.ZipFile(fish_sfx_zip) as z:
    for src, dst in [
        ("WAV/06_UI_Reward_Fail/Fishing_Start.wav", "Fishing_Start.wav"),
        ("WAV/06_UI_Reward_Fail/Fishing_Ready.wav", "Fishing_Ready.wav"),
        ("WAV/04_Water_Fish_Action/Fish_Hard_Bite.wav", "Fish_Hard_Bite.wav"),
        ("WAV/06_UI_Reward_Fail/Fishing_Finish.wav", "Fishing_Finish.wav"),
        ("WAV/06_UI_Reward_Fail/Fish_Escaped_Fail.wav", "Fish_Escaped_Fail.wav"),
    ]:
        data = z.read(src)
        (STAGING / dst).write_bytes(data)
        print(f"Extracted: {dst} ({len(data)} bytes)")

# 6. Cooking Minigame SFX
cook_sfx_zip = ROOT / "assets and old docs/voxel/Cooking_Minigame_SFX_120_Sounds.zip"
with zipfile.ZipFile(cook_sfx_zip) as z:
    for src, dst in [
        ("Cooking_Minigame_SFX_120_Sounds/06_Cooking_Heat/06_07_Fish_Fry_Sizzle.wav", "Fish_Fry_Sizzle.wav"),
        ("Cooking_Minigame_SFX_120_Sounds/06_Cooking_Heat/06_15_Food_Simmer_Low.wav", "Food_Simmer_Low.wav"),
        ("Cooking_Minigame_SFX_120_Sounds/08_Plating_Serving/08_03_Food_Scoop_Plate.wav", "Food_Scoop_Plate.wav"),
    ]:
        data = z.read(src)
        (STAGING / dst).write_bytes(data)
        print(f"Extracted: {dst} ({len(data)} bytes)")

print("=== STAGING COMPLETE ===")
