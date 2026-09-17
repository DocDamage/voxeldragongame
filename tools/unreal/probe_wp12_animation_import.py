"""Probe the supplied Tamsin idle FBX through UE 5.8's legacy FBX factory.

The normal WP-12 intake deliberately does not call source clips usable
animations after the default Interchange path classified an idle sample as a
StaticMesh. This isolated probe explicitly supplies ``FbxFactory`` and the
legacy animation-only import settings. It writes a receipt, then deletes only
the fresh, ignored probe assets. It creates no map or production asset.
"""

import json
import traceback
from pathlib import Path

import unreal


ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
SOURCE = ROOT / "Saved" / "AssetIntake" / "WP12" / "NPC" / "Voxel Rangers" / "Animations" / "Human_Idle_Anim.fbx"
SKELETON_PATH = "/Game/WYRMFALL/Development/Intake/WP12/NPC/Tamsin/SK_Tamsin_Skeleton.SK_Tamsin_Skeleton"
PROBE_DIR = "/Game/WYRMFALL/Development/Intake/WP12/NPC/_AnimationProbe"
OUTPUT = ROOT / "Saved" / "Diagnostics" / "WP12_region01_animation_import_probe.json"

ASSETS = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
TOOLS = unreal.AssetToolsHelpers.get_asset_tools()


def probe_options(skeleton):
    options = unreal.FbxImportUI()
    options.set_editor_property("automated_import_should_detect_type", False)
    options.set_editor_property("mesh_type_to_import", unreal.FBXImportType.FBXIT_ANIMATION)
    options.set_editor_property("import_mesh", False)
    options.set_editor_property("import_as_skeletal", True)
    options.set_editor_property("import_animations", True)
    options.set_editor_property("import_materials", False)
    options.set_editor_property("import_textures", False)
    options.set_editor_property("skeleton", skeleton)
    return options


def run_probe():
    if not SOURCE.is_file():
        raise RuntimeError("Missing staged supplied idle source: " + SOURCE.as_posix())
    if unreal.EditorAssetLibrary.list_assets(PROBE_DIR, recursive=True, include_folder=False):
        raise RuntimeError("Refusing to overwrite a pre-existing probe directory: " + PROBE_DIR)

    skeleton = ASSETS.load_asset(SKELETON_PATH)
    if skeleton is None or not isinstance(skeleton, unreal.Skeleton):
        raise RuntimeError("Missing Tamsin intake skeleton: " + SKELETON_PATH)

    task = unreal.AssetImportTask()
    task.filename = SOURCE.as_posix()
    task.destination_path = PROBE_DIR
    task.destination_name = "A_Tamsin_Idle_LegacyProbe"
    task.automated = True
    task.save = True
    task.replace_existing = False
    task.async_ = False
    # A specified factory bypasses the default Interchange route in AssetTools.
    task.factory = unreal.FbxFactory()
    task.options = probe_options(skeleton)
    TOOLS.import_asset_tasks([task])

    imported_paths = list(task.imported_object_paths)
    discovered_paths = unreal.EditorAssetLibrary.list_assets(PROBE_DIR, recursive=True, include_folder=False)
    objects = []
    for object_path in discovered_paths:
        asset = ASSETS.load_asset(object_path)
        if asset is None:
            continue
        row = {
            "object_path": asset.get_path_name(),
            "class": asset.get_class().get_name(),
        }
        if isinstance(asset, unreal.AnimSequence):
            animation_skeleton = asset.get_editor_property("skeleton")
            row["skeleton"] = animation_skeleton.get_path_name() if animation_skeleton else None
            row["play_length_seconds"] = asset.get_play_length()
            row["number_of_sampled_keys"] = int(asset.get_editor_property("number_of_sampled_keys"))
        objects.append(row)

    animation_rows = [row for row in objects if row["class"] == "AnimSequence"]
    valid_animation_rows = [
        row for row in animation_rows
        if row.get("skeleton") == skeleton.get_path_name()
        and row.get("play_length_seconds", 0.0) > 0.0
        and row.get("number_of_sampled_keys", 0) > 0
    ]
    return {
        "source": SOURCE.as_posix(),
        "skeleton": skeleton.get_path_name(),
        "imported_object_paths": imported_paths,
        "discovered_objects": objects,
        "valid_animation_count": len(valid_animation_rows),
        "status": "PASS_LEGACY_FBX_ANIMATION_IMPORT" if valid_animation_rows else "FAIL_NO_USABLE_ANIM_SEQUENCE",
    }


def main():
    report = {
        "engine": unreal.SystemLibrary.get_engine_version(),
        "kind": "legacy_fbx_animation_probe_not_production_placement",
        "status": "ERROR",
    }
    try:
        report.update(run_probe())
    except Exception:
        report["error"] = traceback.format_exc()
    finally:
        OUTPUT.parent.mkdir(parents=True, exist_ok=True)
        OUTPUT.write_text(json.dumps(report, indent=2), encoding="utf-8")
        # The directory was required to be empty before this run and lives under
        # the ignored development-only intake root.
        if unreal.EditorAssetLibrary.list_assets(PROBE_DIR, recursive=True, include_folder=False):
            report["cleanup"] = unreal.EditorAssetLibrary.delete_directory(PROBE_DIR)
            OUTPUT.write_text(json.dumps(report, indent=2), encoding="utf-8")

    unreal.log("[WP12_ANIMATION_PROBE] status={}".format(report["status"]))


if __name__ == "__main__":
    main()
