"""Import the supplied Palace King used by WP-23.2."""

import json
import sys
from pathlib import Path

import unreal

ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
STAGING = ROOT / "Saved/Staging/WP23/Palace"
DEST = "/Game/WYRMFALL/Development/Intake/WP23/JadePeaks/Palace"
REPORT = ROOT / "Saved/Diagnostics/WP23_2_jade_asset_intake.json"
ASSETS = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
TOOLS = unreal.AssetToolsHelpers.get_asset_tools()


def import_file(source, destination, name, skeletal=False):
    task = unreal.AssetImportTask()
    task.filename = source.as_posix()
    task.destination_path = destination
    task.destination_name = name
    task.automated = True
    task.save = True
    task.replace_existing = True
    task.async_ = False
    if source.suffix.lower() == ".fbx":
        task.factory = unreal.FbxFactory()
        options = unreal.FbxImportUI()
        options.set_editor_property("automated_import_should_detect_type", True)
        options.set_editor_property("import_mesh", True)
        options.set_editor_property("import_as_skeletal", skeletal)
        options.set_editor_property("import_animations", False)
        options.set_editor_property("import_materials", False)
        options.set_editor_property("import_textures", False)
        task.options = options
    TOOLS.import_asset_tasks([task])
    result = ASSETS.load_asset(f"{destination}/{name}")
    if not result:
        raise RuntimeError(f"Import did not produce {destination}/{name}")
    return result


def main():
    result = {"status": "ERROR", "assets": []}
    try:
        mesh = import_file(STAGING / "TVS_VoxelPalace_King_Normalized.fbx",
                           f"{DEST}/Characters", "TVS_VoxelPalace_King", True)
        texture = import_file(STAGING / "TVS_VoxelPalace_King_Texture.png",
                              f"{DEST}/Textures", "TVS_VoxelPalace_King_Texture")
        texture.set_editor_property("filter", unreal.TextureFilter.TF_NEAREST)
        material = ASSETS.load_asset(f"{DEST}/Materials/M_TVS_VoxelPalace_King")
        if not material:
            material = TOOLS.create_asset("M_TVS_VoxelPalace_King", f"{DEST}/Materials",
                                          unreal.Material, unreal.MaterialFactoryNew())
        sample = unreal.MaterialEditingLibrary.create_material_expression(
            material, unreal.MaterialExpressionTextureSampleParameter2D, -300, 0)
        sample.set_editor_property("parameter_name", "BaseTexture")
        sample.set_editor_property("texture", texture)
        unreal.MaterialEditingLibrary.connect_material_property(sample, "RGB", unreal.MaterialProperty.MP_BASE_COLOR)
        material.set_editor_property("used_with_skeletal_mesh", True)
        unreal.MaterialEditingLibrary.recompile_material(material)
        unreal.EditorAssetLibrary.save_loaded_asset(material)
        unreal.EditorAssetLibrary.save_directory(DEST, only_if_is_dirty=False, recursive=True)
        result = {"status": "PASS", "assets": [mesh.get_path_name(), texture.get_path_name(), material.get_path_name()]}
    except Exception as exc:
        result["error"] = repr(exc)
        raise
    finally:
        REPORT.parent.mkdir(parents=True, exist_ok=True)
        REPORT.write_text(json.dumps(result, indent=2), encoding="utf-8")


if __name__ == "__main__":
    try:
        main()
    except Exception:
        sys.exit(1)
