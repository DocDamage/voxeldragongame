"""Programmatic intake and verification of genuine Jadefang (Chinese Dragon) assets in Unreal 5.8."""
import json
from pathlib import Path
import sys
import unreal

print("=== STARTING WP-20 JADEFANG ASSET INTAKE ===")
ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
GLTF_SOURCE = ROOT / "assets and old docs/Chinese+Dragon.gltf"
DEST_PATH = "/Game/WYRMFALL/Development/Intake/WP20/Jadefang"

assets = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
tools = unreal.AssetToolsHelpers.get_asset_tools()

report = {
    "kind": "wp20_jadefang_asset_intake",
    "engine": unreal.SystemLibrary.get_engine_version(),
    "status": "STARTING",
    "source_file": str(GLTF_SOURCE),
    "leader_mesh": None,
    "skeleton": None,
    "follower_meshes": [],
    "animations": [],
    "materials": [],
    "errors": []
}

try:
    # 1. Import or check existing assets
    existing_assets = assets.list_assets(DEST_PATH, recursive=True)
    if len(existing_assets) < 30:
        print(f"Importing {GLTF_SOURCE} into {DEST_PATH}...")
        task = unreal.AssetImportTask()
        task.filename = str(GLTF_SOURCE.as_posix())
        task.destination_path = DEST_PATH
        task.automated = True
        task.save = True
        task.replace_existing = True
        task.async_ = False
        tools.import_asset_tasks([task])
    else:
        print(f"Discovered {len(existing_assets)} existing assets in {DEST_PATH}")

    # 2. Inspect all imported assets
    all_imported = assets.list_assets(DEST_PATH, recursive=True)
    for a in all_imported:
        obj = assets.load_asset(a)
        if not obj:
            continue
        cls_name = obj.get_class().get_name()
        if cls_name == "Skeleton":
            report["skeleton"] = a
        elif cls_name == "SkeletalMesh":
            mesh_name = obj.get_name()
            if mesh_name == "Hip-Local":
                report["leader_mesh"] = a
            else:
                report["follower_meshes"].append(mesh_name)
        elif cls_name == "AnimSequence":
            report["animations"].append(obj.get_name())
        elif cls_name in ("Material", "MaterialInstanceConstant"):
            report["materials"].append(obj.get_name())

    print(f"Leader Mesh: {report['leader_mesh']}")
    print(f"Skeleton: {report['skeleton']}")
    print(f"Follower Meshes count: {len(report['follower_meshes'])}")
    print(f"Animations count: {len(report['animations'])}")
    print(f"Materials count: {len(report['materials'])}")

    if report["leader_mesh"] and report["skeleton"] and len(report["follower_meshes"]) >= 20 and len(report["animations"]) >= 5:
        report["status"] = "SUCCESS"
    else:
        report["status"] = "PARTIAL"
        report["errors"].append("Missing required leader mesh, skeleton, or minimum follower meshes/animations")

except Exception as e:
    report["status"] = "ERROR"
    report["errors"].append(str(e))
    print(f"Exception during Jadefang asset intake: {e}")

out_path = ROOT / "Saved/Diagnostics/WP20_jadefang_asset_intake.json"
out_path.parent.mkdir(parents=True, exist_ok=True)
with open(out_path, "w", encoding="utf-8") as f:
    json.dump(report, f, indent=2)

print(f"Asset intake report written to: {out_path}")
print(f"Result: {report['status']}")
sys.exit(0 if report["status"] == "SUCCESS" else 1)
