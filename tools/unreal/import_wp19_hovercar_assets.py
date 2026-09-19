import unreal
from pathlib import Path
import json
import sys

print("=== STARTING IMPORT OF REAL WP-19 HOVERCAR ASSETS ===")
ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
STAGING = ROOT / "Saved/Staging/WP19"
DEST_PATH = "/Game/WYRMFALL/Vehicles"

assets = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
tools = unreal.AssetToolsHelpers.get_asset_tools()

report = {
    "status": "STARTING",
    "imported_textures": [],
    "imported_static_meshes": [],
    "created_materials": [],
    "errors": []
}

def import_asset(source_file, dest_path, asset_name, options=None):
    full_dest = f"{dest_path}/{asset_name}"
    if assets.does_asset_exist(full_dest):
        print(f"Asset {full_dest} already exists. Loading...")
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
    # 1. Texture
    tex_path = STAGING / "T_ZenithHovercar.png"
    tex_obj = import_asset(tex_path, DEST_PATH, "T_ZenithHovercar")
    if tex_obj:
        report["imported_textures"].append(tex_obj.get_path_name())
        tex_obj.set_editor_property("filter", unreal.TextureFilter.TF_NEAREST)
        tex_obj.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_DEFAULT)
        tex_obj.set_editor_property("srgb", True)
        unreal.EditorAssetLibrary.save_loaded_asset(tex_obj)

    # 2. Material
    mat_path = f"{DEST_PATH}/M_ZenithHovercar"
    if not assets.does_asset_exist(mat_path):
        mat_factory = unreal.MaterialFactoryNew()
        mat_obj = tools.create_asset("M_ZenithHovercar", DEST_PATH, unreal.Material, mat_factory)
        if mat_obj:
            tex_sample = unreal.MaterialEditingLibrary.create_material_expression(mat_obj, unreal.MaterialExpressionTextureSampleParameter2D, -300, 0)
            tex_sample.set_editor_property("parameter_name", "BaseTexture")
            if tex_obj:
                tex_sample.set_editor_property("texture", tex_obj)
            unreal.MaterialEditingLibrary.connect_material_property(tex_sample, "RGB", unreal.MaterialProperty.MP_BASE_COLOR)
            unreal.MaterialEditingLibrary.recompile_material(mat_obj)
            unreal.EditorAssetLibrary.save_loaded_asset(mat_obj)
            report["created_materials"].append(mat_path)
    else:
        mat_obj = assets.load_asset(mat_path)

    # 3. Static Mesh
    obj_path = STAGING / "SM_ZenithHovercar.obj"
    mesh_obj = import_asset(obj_path, DEST_PATH, "SM_ZenithHovercar")
    if mesh_obj and isinstance(mesh_obj, unreal.StaticMesh):
        report["imported_static_meshes"].append(mesh_obj.get_path_name())
        # Assign material if created
        if mat_obj:
            mesh_obj.set_material(0, mat_obj)
            unreal.EditorAssetLibrary.save_loaded_asset(mesh_obj)
            print(f"Assigned {mat_path} to {mesh_obj.get_path_name()}")

    report["status"] = "SUCCESS" if len(report["errors"]) == 0 else "PARTIAL"

except Exception as e:
    report["status"] = "ERROR"
    report["errors"].append(str(e))
    print(f"Exception during WP-19 hovercar asset import: {e}")

out_path = ROOT / "Saved/Diagnostics/WP19_hovercar_asset_intake.json"
out_path.parent.mkdir(parents=True, exist_ok=True)
with open(out_path, "w", encoding="utf-8") as f:
    json.dump(report, f, indent=2)

print(f"Asset intake report written to: {out_path}")
print(f"Result: {report['status']}")
sys.exit(0 if report["status"] == "SUCCESS" else 1)
