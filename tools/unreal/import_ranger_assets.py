import unreal
from pathlib import Path
import json

print("=== STARTING IMPORT OF REAL RANGER ASSETS ===")
ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
STAGING = ROOT / "Saved/Staging/Rangers"
DEST_ITEMS = "/Game/WYRMFALL/Items/Weapons"

assets = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
tools = unreal.AssetToolsHelpers.get_asset_tools()

report = {
    "status": "STARTING",
    "imported_textures": [],
    "imported_static_meshes": [],
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
        ("T_Bow", "TVS_VoxelRangers_Bow_Texture.png", DEST_ITEMS),
        ("T_Arrow", "TVS_VoxelRangers_Arrow_Texture.png", DEST_ITEMS),
    ]:
        obj = import_asset(STAGING / file_name, dest, tex_name)
        if obj:
            report["imported_textures"].append(obj.get_path_name())

    # 2. Static Meshes: SM_Bow, SM_Arrow
    options_sm = unreal.FbxImportUI()
    options_sm.set_editor_property("import_mesh", True)
    options_sm.set_editor_property("import_as_skeletal", False)
    options_sm.set_editor_property("import_materials", False)
    options_sm.set_editor_property("import_textures", False)
    
    sm_bow = import_asset(STAGING / "TVS_VoxelRangers_Bow.fbx", DEST_ITEMS, "SM_Bow", options_sm)
    if sm_bow and isinstance(sm_bow, unreal.StaticMesh):
        report["imported_static_meshes"].append(sm_bow.get_path_name())

    sm_arrow = import_asset(STAGING / "TVS_VoxelRangers_Arrow.fbx", DEST_ITEMS, "SM_Arrow", options_sm)
    if sm_arrow and isinstance(sm_arrow, unreal.StaticMesh):
        report["imported_static_meshes"].append(sm_arrow.get_path_name())

    report["status"] = "SUCCESS" if len(report["errors"]) == 0 else "PARTIAL"

except Exception as e:
    report["status"] = "ERROR"
    report["errors"].append(str(e))
    print(f"Exception during asset import: {e}")

out_path = ROOT / "Saved/Diagnostics/WP06_asset_import.json"
out_path.parent.mkdir(parents=True, exist_ok=True)
with open(out_path, "w") as f:
    json.dump(report, f, indent=2)

    if len(report["errors"]) > 0:
        import sys
        sys.exit(1)
    import sys
    sys.exit(0)
