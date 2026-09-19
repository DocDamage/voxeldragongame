import unreal
from pathlib import Path
import json

print("=== STARTING IMPORT OF WP-17 REAL KNIGHT VARIANTS ===")
ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
STAGING = ROOT / "Saved/Staging/Knights"
DEST_PLAYER = "/Game/WYRMFALL/Characters/Player"

assets = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
tools = unreal.AssetToolsHelpers.get_asset_tools()

report = {
    "status": "STARTING",
    "imported_skeletal_meshes": [],
    "imported_textures": [],
    "errors": []
}

def import_asset(source_file, dest_path, asset_name, options=None):
    full_dest = f"{dest_path}/{asset_name}"
    if assets.does_asset_exist(full_dest):
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
    # 1. Base Skeleton
    base_sk = assets.load_asset(f"{DEST_PLAYER}/SK_Knight")
    skeleton = base_sk.get_editor_property("skeleton") if base_sk else None
    if not skeleton:
        raise RuntimeError("Base skeleton not found on SK_Knight")

    m_knight = assets.load_asset(f"{DEST_PLAYER}/M_Knight")

    # 2. Textures
    for tex_name, file_name in [
        ("T_KnightArcher", "TVS_VoxelKnights_Archer_Texture.png"),
        ("T_KnightCaptain", "TVS_VoxelKnights_Captain_Texture.png"),
        ("T_KnightChampion", "TVS_VoxelKnights_Champion_Texture.png"),
        ("T_KnightCommander", "TVS_VoxelKnights_Commander_Texture.png"),
    ]:
        file_path = STAGING / file_name
        if file_path.exists():
            obj = import_asset(file_path, DEST_PLAYER, tex_name)
            if obj:
                report["imported_textures"].append(obj.get_path_name())

    # 3. Skeletal Meshes sharing SK_Knight_Skeleton
    options = unreal.FbxImportUI()
    options.set_editor_property("import_mesh", True)
    options.set_editor_property("import_as_skeletal", True)
    options.set_editor_property("import_materials", False)
    options.set_editor_property("import_textures", False)
    options.set_editor_property("import_animations", False)
    options.set_editor_property("skeleton", skeleton)

    variants = [
        ("SK_KnightArcher", "TVS_VoxelKnights_Archer.fbx"),
        ("SK_KnightCaptain", "TVS_VoxelKnights_Captain.fbx"),
        ("SK_KnightChampion", "TVS_VoxelKnights_Champion.fbx"),
        ("SK_KnightCommander", "TVS_VoxelKnights_Commander.fbx"),
    ]

    for asset_name, fbx_name in variants:
        fbx_path = STAGING / fbx_name
        if not fbx_path.exists():
            report["errors"].append(f"Missing staging file: {fbx_name}")
            continue
        mesh = import_asset(fbx_path, DEST_PLAYER, asset_name, options)
        if mesh and isinstance(mesh, unreal.SkeletalMesh):
            report["imported_skeletal_meshes"].append(mesh.get_path_name())
            # Assign material M_Knight if available
            if m_knight:
                try:
                    materials = mesh.get_editor_property("materials")
                    for i in range(len(materials)):
                        mesh.set_material(i, m_knight)
                    unreal.EditorAssetSubsystem().save_loaded_asset(mesh)
                except Exception as ex:
                    print(f"Could not assign material to {asset_name}: {ex}")

    report["status"] = "SUCCESS" if len(report["errors"]) == 0 else "PARTIAL"

except Exception as e:
    report["status"] = "FAILED"
    report["errors"].append(str(e))
    print(f"Exception: {e}")

out_path = ROOT / "Saved/Diagnostics/WP17_variant_import.json"
out_path.parent.mkdir(parents=True, exist_ok=True)
with open(out_path, "w") as f:
    json.dump(report, f, indent=2)

print(f"=== WP-17 VARIANT IMPORT COMPLETE: {report['status']} ===")
