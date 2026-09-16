import unreal
from pathlib import Path
import json
import traceback

print("=== STARTING IMPORT OF REAL KNIGHT ASSETS ===")
ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
STAGING = ROOT / "Saved/Staging/Knights"
DEST_PLAYER = "/Game/WYRMFALL/Characters/Player"
DEST_ITEMS = "/Game/WYRMFALL/Items/Weapons"

assets = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
tools = unreal.AssetToolsHelpers.get_asset_tools()

report = {
    "status": "STARTING",
    "imported_skeletal_meshes": [],
    "imported_textures": [],
    "imported_static_meshes": [],
    "imported_animations": [],
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
        ("T_Knight", "TVS_VoxelKnights_Knight_Texture.png", DEST_PLAYER),
        ("T_KnightHelmUp", "TVS_VoxelKnights_HelmUpKnight_Texture.png", DEST_PLAYER),
        ("T_Sword", "TVS_VoxelKnights_Sword_Texture.png", DEST_ITEMS),
        ("T_Shield", "TVS_VoxelKnights_Shield_Texture.png", DEST_ITEMS),
    ]:
        obj = import_asset(STAGING / file_name, dest, tex_name)
        if obj:
            report["imported_textures"].append(obj.get_path_name())

    # 2. Skeletal Mesh Base: SK_Knight
    options_sk = unreal.FbxImportUI()
    options_sk.set_editor_property("import_mesh", True)
    options_sk.set_editor_property("import_as_skeletal", True)
    options_sk.set_editor_property("import_materials", False)
    options_sk.set_editor_property("import_textures", False)
    options_sk.set_editor_property("import_animations", False)
    
    sk_knight = import_asset(STAGING / "TVS_VoxelKnights_Knight.fbx", DEST_PLAYER, "SK_Knight", options_sk)
    if sk_knight and isinstance(sk_knight, unreal.SkeletalMesh):
        report["imported_skeletal_meshes"].append(sk_knight.get_path_name())
        skeleton = sk_knight.get_editor_property("skeleton")
        print(f"Base Knight skeleton: {skeleton.get_path_name() if skeleton else 'None'}")
    else:
        skeleton = None
        report["errors"].append("SK_Knight not a SkeletalMesh")

    # 3. Skeletal Mesh Variation: SK_KnightHelmDown sharing same skeleton
    options_helm = unreal.FbxImportUI()
    options_helm.set_editor_property("import_mesh", True)
    options_helm.set_editor_property("import_as_skeletal", True)
    options_helm.set_editor_property("import_materials", False)
    options_helm.set_editor_property("import_textures", False)
    options_helm.set_editor_property("import_animations", False)
    if skeleton:
        options_helm.set_editor_property("skeleton", skeleton)
        
    sk_helm = import_asset(STAGING / "TVS_VoxelKnights_KnightHelmDown.fbx", DEST_PLAYER, "SK_KnightHelmDown", options_helm)
    if sk_helm and isinstance(sk_helm, unreal.SkeletalMesh):
        report["imported_skeletal_meshes"].append(sk_helm.get_path_name())

    # 4. Static Meshes: SM_Sword, SM_Shield
    options_sm = unreal.FbxImportUI()
    options_sm.set_editor_property("import_mesh", True)
    options_sm.set_editor_property("import_as_skeletal", False)
    options_sm.set_editor_property("import_materials", False)
    options_sm.set_editor_property("import_textures", False)
    
    sm_sword = import_asset(STAGING / "TVS_VoxelKnights_Sword.fbx", DEST_ITEMS, "SM_Sword", options_sm)
    if sm_sword and isinstance(sm_sword, unreal.StaticMesh):
        report["imported_static_meshes"].append(sm_sword.get_path_name())

    sm_shield = import_asset(STAGING / "TVS_VoxelKnights_Shield.fbx", DEST_ITEMS, "SM_Shield", options_sm)
    if sm_shield and isinstance(sm_shield, unreal.StaticMesh):
        report["imported_static_meshes"].append(sm_shield.get_path_name())

    # 5. Animations
    if skeleton:
        for anim_name, file_name in [
            ("A_Human_Idle", "Human_Idle_Anim.fbx"),
            ("A_Human_Walk", "Human_Walk_Anim.fbx"),
            ("A_Human_Slash", "Human_Slash_Anim.fbx")
        ]:
            options_anim = unreal.FbxImportUI()
            options_anim.set_editor_property("import_mesh", False)
            options_anim.set_editor_property("import_as_skeletal", False)
            options_anim.set_editor_property("import_materials", False)
            options_anim.set_editor_property("import_textures", False)
            options_anim.set_editor_property("import_animations", True)
            options_anim.set_editor_property("skeleton", skeleton)
            
            anim_obj = import_asset(STAGING / file_name, DEST_PLAYER, anim_name, options_anim)
            if anim_obj and isinstance(anim_obj, unreal.AnimSequence):
                report["imported_animations"].append(anim_obj.get_path_name())

    report["status"] = "PASS" if not report["errors"] else "PARTIAL"
except Exception as ex:
    report["status"] = "ERROR"
    report["exception"] = traceback.format_exc()
    print("Exception during import:", traceback.format_exc())

out_file = ROOT / "Saved/Diagnostics/knight_assets_import_report.json"
out_file.parent.mkdir(parents=True, exist_ok=True)
out_file.write_text(json.dumps(report, indent=2), encoding="utf-8")
print(f"Report written to {out_file}")
