"""Inspect the supplied Tamsin FBX's authored material-import result.

The development preview intentionally began with a simple texture material and
rendered mostly black. This temporary import probe records the source material
graph and mesh assignment without retaining imported assets. It is diagnostic
evidence only, never production content.
"""

import json
import traceback
from pathlib import Path

import unreal


ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
SOURCE = ROOT / "Saved" / "AssetIntake" / "WP12" / "NPC" / "Voxel Rangers" / "FBX" / "Character" / "TVS_VoxelRangers_Captain.fbx"
PROBE_DIR = "/Game/WYRMFALL/Development/Intake/WP12/NPC/_SourceMaterialProbe"
OUTPUT = ROOT / "Saved" / "Diagnostics" / "WP12_region01_source_material_probe.json"

ASSETS = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
TOOLS = unreal.AssetToolsHelpers.get_asset_tools()


def options():
    result = unreal.FbxImportUI()
    result.set_editor_property("automated_import_should_detect_type", False)
    result.set_editor_property("mesh_type_to_import", unreal.FBXImportType.FBXIT_SKELETAL_MESH)
    result.set_editor_property("import_mesh", True)
    result.set_editor_property("import_as_skeletal", True)
    result.set_editor_property("import_animations", False)
    result.set_editor_property("import_materials", True)
    result.set_editor_property("import_textures", True)
    return result


def material_row(material):
    row = {"object_path": material.get_path_name(), "class": material.get_class().get_name()}
    if isinstance(material, unreal.Material):
        row["expressions"] = []
        for expression in material.get_editor_property("expressions"):
            expression_row = {"class": expression.get_class().get_name()}
            if isinstance(expression, unreal.MaterialExpressionTextureSample):
                texture = expression.get_editor_property("texture")
                expression_row["texture"] = texture.get_path_name() if texture else None
            row["expressions"].append(expression_row)
    elif isinstance(material, unreal.MaterialInstance):
        parent = material.get_editor_property("parent")
        row["parent"] = parent.get_path_name() if parent else None
        row["parameters"] = {}
        for property_name in ("texture_parameter_values", "scalar_parameter_values", "vector_parameter_values"):
            try:
                row["parameters"][property_name] = [str(value) for value in material.get_editor_property(property_name)]
            except Exception as error:
                row["parameters"][property_name] = "UNAVAILABLE: " + str(error)
    return row


def main():
    report = {
        "engine": unreal.SystemLibrary.get_engine_version(),
        "kind": "temporary_interchange_fbx_source_material_import_probe_not_production_placement",
        "status": "ERROR",
    }
    try:
        if not SOURCE.is_file():
            raise RuntimeError("Missing staged source FBX: " + SOURCE.as_posix())
        if unreal.EditorAssetLibrary.list_assets(PROBE_DIR, recursive=True, include_folder=False):
            raise RuntimeError("Refusing to overwrite a pre-existing probe directory: " + PROBE_DIR)

        task = unreal.AssetImportTask()
        task.filename = SOURCE.as_posix()
        task.destination_path = PROBE_DIR
        task.destination_name = "SK_Tamsin_SourceMaterialProbe"
        task.automated = True
        task.save = True
        task.replace_existing = False
        task.async_ = False
        # Let UE 5.8's normal Interchange route handle this known multi-root
        # source mesh. The legacy factory cannot ingest it as skeletal data,
        # while the scoped intake already proved Interchange can.
        task.options = options()
        TOOLS.import_asset_tasks([task])

        discovered = unreal.EditorAssetLibrary.list_assets(PROBE_DIR, recursive=True, include_folder=False)
        report["imported_object_paths"] = list(task.imported_object_paths)
        report["objects"] = []
        skeletal_meshes = []
        materials = []
        for object_path in discovered:
            asset = ASSETS.load_asset(object_path)
            if asset is None:
                continue
            if isinstance(asset, unreal.SkeletalMesh):
                skeletal_meshes.append(asset)
            elif isinstance(asset, unreal.MaterialInterface):
                materials.append(asset)
            report["objects"].append({"object_path": asset.get_path_name(), "class": asset.get_class().get_name()})

        report["materials"] = [material_row(material) for material in materials]
        report["skeletal_meshes"] = []
        for mesh in skeletal_meshes:
            assignments = []
            for slot in mesh.get_editor_property("materials"):
                interface = slot.get_editor_property("material_interface")
                assignments.append(interface.get_path_name() if interface else None)
            report["skeletal_meshes"].append({"object_path": mesh.get_path_name(), "materials": assignments})

        report["status"] = "PASS_AUTHORED_MATERIAL_DISCOVERY" if skeletal_meshes and materials else "FAIL_NO_AUTHORED_MATERIAL_DISCOVERED"
    except Exception:
        report["error"] = traceback.format_exc()
    finally:
        OUTPUT.parent.mkdir(parents=True, exist_ok=True)
        OUTPUT.write_text(json.dumps(report, indent=2), encoding="utf-8")
        if unreal.EditorAssetLibrary.list_assets(PROBE_DIR, recursive=True, include_folder=False):
            report["cleanup"] = unreal.EditorAssetLibrary.delete_directory(PROBE_DIR)
            OUTPUT.write_text(json.dumps(report, indent=2), encoding="utf-8")

    unreal.log("[WP12_SOURCE_MATERIAL_PROBE] status={}".format(report["status"]))


if __name__ == "__main__":
    main()
