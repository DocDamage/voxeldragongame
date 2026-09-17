import unreal
from pathlib import Path
import json
import sys

print("=== STARTING IMPORT OF REAL WP-08 ASSETS ===")
ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
STAGING = ROOT / "Saved/Staging/WP08"

assets = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
tools = unreal.AssetToolsHelpers.get_asset_tools()

report = {
    "status": "STARTING",
    "imported_textures": [],
    "imported_static_meshes": [],
    "verified_building_meshes": [],
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
    tex_obj = import_asset(
        STAGING / "TVS_VoxelKnights_Chest_Texture.png",
        "/Game/WYRMFALL/Environment/Camp",
        "T_StorageChest"
    )
    if tex_obj:
        report["imported_textures"].append(tex_obj.get_path_name())

    # 2. Static Mesh for Storage Chest
    options_sm = unreal.FbxImportUI()
    options_sm.set_editor_property("import_mesh", True)
    options_sm.set_editor_property("import_as_skeletal", False)
    options_sm.set_editor_property("import_materials", False)
    options_sm.set_editor_property("import_textures", False)

    mesh_obj = import_asset(
        STAGING / "TVS_VoxelKnights_Chest.fbx",
        "/Game/WYRMFALL/Environment/Camp",
        "SM_StorageChest",
        options_sm
    )
    if mesh_obj and isinstance(mesh_obj, unreal.StaticMesh):
        report["imported_static_meshes"].append(mesh_obj.get_path_name())

    # 3. Verify Wood Building Meshes in Content/WYRMFALL/Environment/Building/Wood
    building_mesh_paths = [
        "/Game/WYRMFALL/Environment/Building/Wood/SM_Stylized_Wood_Foundation",
        "/Game/WYRMFALL/Environment/Building/Wood/SM_Stylized_Wood_Wall",
        "/Game/WYRMFALL/Environment/Building/Wood/SM_Stylized_Wood_Doorframe",
        "/Game/WYRMFALL/Environment/Building/Wood/SM_Stylized_Wood_Door",
        "/Game/WYRMFALL/Environment/Building/Wood/SM_Stylized_Wood_Roof",
        "/Game/WYRMFALL/Environment/Building/Wood/SM_Stylized_Wood_Ceiling",
    ]
    for b_path in building_mesh_paths:
        b_mesh = assets.load_asset(b_path)
        if b_mesh:
            report["verified_building_meshes"].append(b_path)
            print(f"Verified building mesh: {b_path} ({b_mesh.get_class().get_name()})")
        else:
            print(f"Failed to load building mesh: {b_path}")
            report["errors"].append(f"Failed to load building mesh: {b_path}")

    report["status"] = "SUCCESS" if len(report["errors"]) == 0 else "PARTIAL"

except Exception as e:
    report["status"] = "ERROR"
    report["errors"].append(str(e))
    print(f"Exception during WP-08 asset import: {e}")

out_path = ROOT / "Saved/Diagnostics/WP08_asset_import.json"
out_path.parent.mkdir(parents=True, exist_ok=True)
with open(out_path, "w") as f:
    json.dump(report, f, indent=2)

print(f"=== ASSET IMPORT COMPLETE: {report['status']} ===")
if len(report["errors"]) > 0:
    sys.exit(1)
