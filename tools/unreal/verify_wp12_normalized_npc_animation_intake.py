"""Verify Blender-normalized supplied WP-12 NPC FBXs in Unreal 5.8.

The source normalizer exports every mesh and clip from the same 25-bone
armature. This intake imports those local, ignored derivatives through the
legacy FBX path, which is required for the supplied animation-only files, and
records mesh/animation skeleton agreement. It creates no production map or
gameplay actors.
"""

import json
import traceback
from pathlib import Path

import unreal


ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
SOURCE_RECEIPT = ROOT / "Saved" / "Diagnostics" / "WP12_region01_normalized_npc_sources.json"
OUTPUT = ROOT / "Saved" / "Diagnostics" / "WP12_region01_normalized_npc_intake.json"
# A fresh intake root prevents a prior diagnostic import from masking a
# corrected normalized FBX source during this bounded compatibility check.
DEST_ROOT = "/Game/WYRMFALL/Development/Intake/WP12/NormalizedNPCTrimmed"

ASSETS = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
TOOLS = unreal.AssetToolsHelpers.get_asset_tools()


def legacy_mesh_options():
    options = unreal.FbxImportUI()
    # Let the legacy factory detect the normalized single-root mesh as skeletal
    # before AssetTools resolves its supported class.
    options.set_editor_property("automated_import_should_detect_type", True)
    options.set_editor_property("import_mesh", True)
    options.set_editor_property("import_as_skeletal", True)
    options.set_editor_property("import_animations", False)
    options.set_editor_property("import_materials", False)
    options.set_editor_property("import_textures", False)
    return options


def legacy_animation_options(skeleton):
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


def import_or_load(source, destination, name, options):
    object_path = destination + "/" + name
    if ASSETS.does_asset_exist(object_path):
        asset = ASSETS.load_asset(object_path)
        if asset is None:
            raise RuntimeError("Existing asset does not load: " + object_path)
        return asset, False
    task = unreal.AssetImportTask()
    task.filename = source
    task.destination_path = destination
    task.destination_name = name
    task.automated = True
    task.save = True
    task.replace_existing = False
    task.async_ = False
    task.factory = unreal.FbxFactory()
    task.options = options
    TOOLS.import_asset_tasks([task])
    asset = ASSETS.load_asset(object_path)
    if asset is None:
        raise RuntimeError("Import did not produce expected asset: " + object_path)
    return asset, True


def animation_row(animation):
    skeleton = animation.get_editor_property("skeleton")
    return {
        "object_path": animation.get_path_name(),
        "class": animation.get_class().get_name(),
        "skeleton": skeleton.get_path_name() if skeleton else None,
        "play_length_seconds": animation.get_play_length(),
        "number_of_sampled_keys": int(animation.get_editor_property("number_of_sampled_keys")),
    }


def main():
    report = {
        "engine": unreal.SystemLibrary.get_engine_version(),
        "kind": "normalized_supplied_npc_mesh_animation_intake_not_production_placement",
        "status": "ERROR",
        "roles": [],
    }
    try:
        source = json.loads(SOURCE_RECEIPT.read_text(encoding="utf-8"))
        for source_role in source["roles"]:
            role = source_role["role"]
            normalized = source_role["normalized_files"]
            destination = DEST_ROOT + "/" + role
            mesh, mesh_imported = import_or_load(
                normalized["mesh"]["path"], destination, "SK_" + role, legacy_mesh_options()
            )
            if not isinstance(mesh, unreal.SkeletalMesh):
                raise RuntimeError("Normalized mesh was not imported as SkeletalMesh for " + role)
            skeleton = mesh.get_editor_property("skeleton")
            if skeleton is None:
                raise RuntimeError("Normalized mesh has no skeleton for " + role)
            component = unreal.SkeletalMeshComponent()
            component.set_skinned_asset_and_update(mesh)
            mesh_row = {
                "object_path": mesh.get_path_name(),
                "class": mesh.get_class().get_name(),
                "skeleton": skeleton.get_path_name(),
                "bone_count": component.get_num_bones(),
                "imported_this_run": mesh_imported,
            }
            expected_bones = source_role["root_normalization"]["normalized_bone_count"]
            if mesh_row["bone_count"] != expected_bones:
                raise RuntimeError("Normalized mesh bone-count mismatch for {}: {} != {}".format(
                    role, mesh_row["bone_count"], expected_bones
                ))
            animations = {}
            for kind in ("idle", "walk"):
                animation, animation_imported = import_or_load(
                    normalized[kind]["path"],
                    destination,
                    "A_{}_{}".format(role, kind.title()),
                    legacy_animation_options(skeleton),
                )
                if not isinstance(animation, unreal.AnimSequence):
                    raise RuntimeError("Normalized {} was not AnimSequence for {}".format(kind, role))
                row = animation_row(animation)
                row["imported_this_run"] = animation_imported
                if row["skeleton"] != skeleton.get_path_name():
                    raise RuntimeError("Normalized {} skeleton mismatch for {}".format(kind, role))
                if row["play_length_seconds"] <= 0.0 or row["number_of_sampled_keys"] <= 0:
                    raise RuntimeError("Normalized {} has no sampled data for {}".format(kind, role))
                animations[kind] = row
            report["roles"].append({"role": role, "mesh": mesh_row, "animations": animations})
        report["status"] = "PASS_NORMALIZED_NPC_MESH_AND_ANIMATION_SKELETON_COMPATIBILITY"
    except Exception:
        report["error"] = traceback.format_exc()
        raise
    finally:
        OUTPUT.parent.mkdir(parents=True, exist_ok=True)
        OUTPUT.write_text(json.dumps(report, indent=2), encoding="utf-8")


if __name__ == "__main__":
    main()
