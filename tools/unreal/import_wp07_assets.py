import unreal
from pathlib import Path
import json

print("=== STARTING IMPORT OF REAL WP-07 ASSETS ===")
ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
STAGING = ROOT / "Saved/Staging/WP07"

assets = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
tools = unreal.AssetToolsHelpers.get_asset_tools()

report = {
    "status": "STARTING",
    "imported_textures": [],
    "imported_static_meshes": [],
    "imported_audio": [],
    "errors": []
}

def import_asset(source_file, dest_path, asset_name, options=None):
    full_dest = f"{dest_path}/{asset_name}"
    existing = assets.does_asset_exist(full_dest)
    if existing:
        print(f"Asset {full_dest} already exists.")
        return assets.load_asset(full_dest)

    task = unreal.AssetImportTask()
    task.filename = str(source_file.as_posix())
    task.destination_path = dest_path
    task.destination_name = asset_name
    task.automated = True
    task.save = True
    task.replace_existing = True
    task.async_ = False
    if options:
        task.options = options

    tools.import_asset_tasks([task])
    obj = assets.load_asset(full_dest)
    if obj:
        print(f"Successfully imported: {full_dest} ({obj.get_class().get_name()})")
    else:
        print(f"Failed to import: {full_dest}")
        report["errors"].append(f"Failed import: {full_dest}")
    return obj

try:
    # 1. Textures
    for tex_name, file_name, dest in [
        ("T_Campfire", "TVS_VoxelRangers_Campfire_Texture.png", "/Game/WYRMFALL/Environment/Camp"),
        ("T_FishingRod", "TVS_VoxelRangers_Spear_Texture.png", "/Game/WYRMFALL/Items/Tools"),
        ("T_Potato", "TVS_VoxelFarm_Potato_Texture.png", "/Game/WYRMFALL/Items/Ingredients"),
        ("T_OceanFish", "overworld-47-ocean_fish.png", "/Game/WYRMFALL/Items/Fish"),
        ("T_Jetty", "T_Jetty.png", "/Game/WYRMFALL/Environment/Water"),
        ("T_WaterLily", "T_WaterLily.png", "/Game/WYRMFALL/Environment/Water"),
    ]:
        obj = import_asset(STAGING / file_name, dest, tex_name)
        if obj:
            report["imported_textures"].append(obj.get_path_name())

    # 2. Static Meshes (FBX & OBJ)
    options_sm = unreal.FbxImportUI()
    options_sm.set_editor_property("import_mesh", True)
    options_sm.set_editor_property("import_as_skeletal", False)
    options_sm.set_editor_property("import_materials", False)
    options_sm.set_editor_property("import_textures", False)

    for mesh_name, file_name, dest in [
        ("SM_Campfire", "TVS_VoxelRangers_Campfire_Frame1.fbx", "/Game/WYRMFALL/Environment/Camp"),
        ("SM_FishingRod", "TVS_VoxelRangers_Spear.fbx", "/Game/WYRMFALL/Items/Tools"),
        ("SM_Potato", "TVS_VoxelFarm_Potato.fbx", "/Game/WYRMFALL/Items/Ingredients"),
        ("SM_OceanFish", "overworld-47-ocean_fish.obj", "/Game/WYRMFALL/Items/Fish"),
        ("SM_Jetty", "jetty.fbx", "/Game/WYRMFALL/Environment/Water"),
        ("SM_WaterLily", "water_lily.fbx", "/Game/WYRMFALL/Environment/Water"),
    ]:
        obj = import_asset(STAGING / file_name, dest, mesh_name, options_sm)
        if obj and isinstance(obj, unreal.StaticMesh):
            report["imported_static_meshes"].append(obj.get_path_name())

    # 3. Audio Sounds (WAV)
    for snd_name, file_name, dest in [
        ("A_Fishing_Start", "Fishing_Start.wav", "/Game/WYRMFALL/Audio/Fishing"),
        ("A_Fishing_Ready", "Fishing_Ready.wav", "/Game/WYRMFALL/Audio/Fishing"),
        ("A_Fish_Hard_Bite", "Fish_Hard_Bite.wav", "/Game/WYRMFALL/Audio/Fishing"),
        ("A_Fishing_Finish", "Fishing_Finish.wav", "/Game/WYRMFALL/Audio/Fishing"),
        ("A_Fish_Escaped_Fail", "Fish_Escaped_Fail.wav", "/Game/WYRMFALL/Audio/Fishing"),
        ("A_Fish_Fry_Sizzle", "Fish_Fry_Sizzle.wav", "/Game/WYRMFALL/Audio/Cooking"),
        ("A_Food_Simmer_Low", "Food_Simmer_Low.wav", "/Game/WYRMFALL/Audio/Cooking"),
        ("A_Food_Scoop_Plate", "Food_Scoop_Plate.wav", "/Game/WYRMFALL/Audio/Cooking"),
    ]:
        obj = import_asset(STAGING / file_name, dest, snd_name)
        if obj:
            report["imported_audio"].append(obj.get_path_name())

    report["status"] = "SUCCESS" if len(report["errors"]) == 0 else "PARTIAL"

except Exception as e:
    report["status"] = "ERROR"
    report["errors"].append(str(e))
    print(f"Exception during WP-07 asset import: {e}")

out_path = ROOT / "Saved/Diagnostics/WP07_asset_import.json"
out_path.parent.mkdir(parents=True, exist_ok=True)
with open(out_path, "w") as f:
    json.dump(report, f, indent=2)

print(f"=== ASSET IMPORT COMPLETE: {report['status']} ===")
if len(report["errors"]) > 0:
    import sys
    sys.exit(1)
