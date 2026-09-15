"""Native intake and measured inspection of bounded real WP-00 samples.

Writes only excluded Development/Intake assets and Saved evidence. No game owner.
Run with C: UE 5.8.2 UnrealEditor-Cmd and -ExecutePythonScript (forward slashes).
"""
from pathlib import Path
import json
import traceback
import unreal

ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
DEST = "/Game/WYRMFALL/Development/Intake/WP00/Review"
assets = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
tools = unreal.AssetToolsHelpers.get_asset_tools()
report = {"engine": unreal.SystemLibrary.get_engine_version(),
          "kind": "native_asset_inspection_not_gameplay_acceptance", "imports": [], "meshes": [], "animations": []}
inputs = [
    ("Wolf", ROOT / "Saved/AssetIntake/WP00/Review/wolf.fbx"),
    ("Sword", ROOT / "Saved/AssetIntake/WP00/Review/TVS_VoxelKnights_Sword.fbx"),
    ("WolfTexture", ROOT / "Saved/AssetIntake/WP00/Review/wolf.png"),
    ("SwordTexture", ROOT / "Saved/AssetIntake/WP00/Review/TVS_VoxelKnights_Sword_Texture.png"),
    ("KnightTexture", ROOT / "Saved/AssetIntake/WP00/Voxel Knights/Textures/Characters/TVS_VoxelKnights_Knight_Texture.png"),
]
try:
    for label, source in inputs:
        dest = DEST + "/" + label
        existing = list(assets.list_assets(dest, recursive=True, include_folder=False))
        if not existing:
            task = unreal.AssetImportTask()
            task.filename = source.as_posix()
            task.destination_path = dest
            task.automated = True
            task.save = True
            task.replace_existing = False
            task.async_ = False
            tools.import_asset_tasks([task])
            existing = list(assets.list_assets(dest, recursive=True, include_folder=False))
        report["imports"].append({"label": label, "source": source.as_posix(), "packages": existing})
        if not existing:
            raise RuntimeError("Empty import: " + label)
    for path in assets.list_assets("/Game/WYRMFALL/Development/Intake/WP00", recursive=True, include_folder=False):
        obj = assets.load_asset(path)
        if isinstance(obj, (unreal.SkeletalMesh, unreal.StaticMesh)):
            row = {"path": path, "class": obj.get_class().get_name()}
            if isinstance(obj, unreal.SkeletalMesh):
                bounds = obj.get_imported_bounds()
                row["origin"] = bounds.origin.to_tuple()
                row["extent"] = bounds.box_extent.to_tuple()
                comp = unreal.SkeletalMeshComponent()
                comp.set_skinned_asset_and_update(obj)
                row["bones"] = comp.get_num_bones()
                row["skeleton"] = obj.get_editor_property("skeleton").get_path_name()
                mats = obj.get_editor_property("materials")
            else:
                box = obj.get_bounding_box()
                row["min"] = box.min.to_tuple()
                row["max"] = box.max.to_tuple()
                mats = obj.get_editor_property("static_materials")
            row["materials"] = [m.material_interface.get_path_name() if m.material_interface else None for m in mats]
            report["meshes"].append(row)
        elif isinstance(obj, unreal.AnimSequence):
            report["animations"].append({"path": path, "length": obj.get_play_length(), "skeleton": obj.get_editor_property("skeleton").get_path_name()})
    report["status"] = "PASS_SCOPED_IMPORT_AND_NUMERIC_INSPECTION"
except Exception:
    report["status"] = "ERROR"
    report["error"] = traceback.format_exc()
finally:
    (ROOT / "Saved/Diagnostics/WP00_asset_review.json").write_text(json.dumps(report, indent=2), encoding="utf-8")
if report["status"] == "ERROR":
    raise RuntimeError(report["error"])
