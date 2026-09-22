"""Import the user-selected WP-23.7 forge and overseer presentation assets."""

import hashlib
import json
import struct
import traceback
from pathlib import Path

import unreal


ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
OUT = ROOT / "Saved/Diagnostics/WP23_7_SelectedAssetIntake"
REPORT_PATH = OUT / "unreal_intake.json"
DEST = "/Game/WYRMFALL/Development/Intake/WP23_7/Selected"
SOURCES = {
    "forge_source": Path(r"G:\downloads\the_blacksmiths.glb"),
    "forge_voxel_fine": OUT / "forge_voxel_fine.glb",
    "harvester_source": Path(r"G:\downloads\thatched_horror_harvester.glb"),
    "harvester_voxel_fine": OUT / "harvester_voxel_fine.glb",
}
DESTINATIONS = {
    "forge_voxel_fine": DEST + "/Forge/ModularVoxelFine",
    "harvester_source": DEST + "/Overseer/Source",
    "harvester_voxel_fine": DEST + "/Overseer/VoxelFine",
}
EXPECTED_SOURCE_HASHES = {
    "forge_source": "BF8B5EA61196C151D99254E2F6D7F1AEB89A97BF7B7DF9BF00D8D187B1BEEA14",
    "harvester_source": "BE662676FE4DF689E5EA77A2B49FA7305504691DD5A226FF93E32276CD776F57",
}


def sha256(path):
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest().upper()


def glb_metadata(path):
    with path.open("rb") as handle:
        magic, version, length = struct.unpack("<4sII", handle.read(12))
        if magic != b"glTF" or version != 2:
            raise RuntimeError(f"Not GLB 2.0: {path}")
        chunk_length, chunk_type = struct.unpack("<II", handle.read(8))
        if chunk_type != 0x4E4F534A:
            raise RuntimeError(f"Missing JSON chunk: {path}")
        document = json.loads(handle.read(chunk_length).decode("utf-8").rstrip("\x00 \t\r\n"))
    return {
        "byte_length": length,
        "generator": document.get("asset", {}).get("generator"),
        "counts": {key: len(document.get(key, [])) for key in ("nodes", "meshes", "materials", "textures", "images", "skins", "animations")},
        "copyright": document.get("asset", {}).get("copyright"),
        "license_record": document.get("asset", {}).get("extras", {}).get("license") if isinstance(document.get("asset", {}).get("extras"), dict) else None,
    }


def import_asset(source, destination):
    task = unreal.AssetImportTask()
    task.filename = source.as_posix()
    task.destination_path = destination
    task.automated = True
    task.save = True
    task.replace_existing = True
    task.async_ = False
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    return list(task.imported_object_paths)


def summarize(asset_subsystem, destination):
    result = {}
    for object_path in asset_subsystem.list_assets(destination, recursive=True, include_folder=False):
        asset = asset_subsystem.load_asset(object_path)
        if asset:
            result.setdefault(asset.get_class().get_name(), []).append(asset.get_path_name())
    return {key: sorted(set(values)) for key, values in sorted(result.items())}


def mesh_measurements(asset_subsystem, destination):
    rows = []
    for object_path in asset_subsystem.list_assets(destination, recursive=True, include_folder=False):
        asset = asset_subsystem.load_asset(object_path)
        if isinstance(asset, unreal.StaticMesh):
            box = asset.get_bounding_box()
            rows.append({
                "object_path": asset.get_path_name(),
                "bounds_min": list(box.min.to_tuple()),
                "bounds_max": list(box.max.to_tuple()),
                "material_slots": len(asset.get_editor_property("static_materials")),
                "body_setup_present": asset.get_editor_property("body_setup") is not None,
            })
    return rows


def main():
    report = {
        "kind": "wp23_7_selected_asset_unreal_intake",
        "engine": unreal.SystemLibrary.get_engine_version(),
        "scope": "isolated presentation intake only; no animation, rigging, gameplay, map, facts, Echo, travel, save, or regional completion proof",
        "status": "ERROR",
        "sources": {},
        "imports": {},
    }
    try:
        for label, path in SOURCES.items():
            if not path.is_file():
                raise FileNotFoundError(path)
            digest = sha256(path)
            if label in EXPECTED_SOURCE_HASHES and digest != EXPECTED_SOURCE_HASHES[label]:
                raise RuntimeError(f"{label} hash mismatch: {digest}")
            report["sources"][label] = {"path": str(path), "sha256": digest, "glb": glb_metadata(path)}

        imported = {}
        for label, destination in DESTINATIONS.items():
            imported[label] = import_asset(SOURCES[label], destination)

        assets = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
        summaries = {label: summarize(assets, destination) for label, destination in DESTINATIONS.items()}
        measurements = {label: mesh_measurements(assets, destination) for label, destination in DESTINATIONS.items()}
        forge_count = len(summaries["forge_voxel_fine"].get("StaticMesh", []))
        source_overseer_count = len(summaries["harvester_source"].get("StaticMesh", []))
        voxel_overseer_count = len(summaries["harvester_voxel_fine"].get("StaticMesh", []))
        no_animation = not any(summaries[label].get("AnimSequence") or summaries[label].get("Skeleton") for label in ("harvester_source", "harvester_voxel_fine"))
        report.update({
            "status": "PASS_INTAKE" if forge_count == 141 and source_overseer_count == 4 and voxel_overseer_count == 4 and no_animation else "PARTIAL",
            "imports": imported,
            "asset_summaries": summaries,
            "mesh_measurements": measurements,
            "acceptance_checks": {
                "forge_modular_static_mesh_count": forge_count,
                "forge_expected_modular_count": 141,
                "harvester_source_static_mesh_count": source_overseer_count,
                "harvester_voxel_static_mesh_count": voxel_overseer_count,
                "harvester_skeleton_or_animation_imported": not no_animation,
                "forge_objects_combined": forge_count != 141,
            },
            "provenance_boundary": "User-supplied local files; fab-model-conversion generator; no embedded copyright or license. Original listing and applicable project license remain required before redistribution or production acceptance.",
        })
    except Exception:
        report["status"] = "ERROR"
        report["error"] = traceback.format_exc()
    finally:
        OUT.mkdir(parents=True, exist_ok=True)
        REPORT_PATH.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(f"WP-23.7 selected asset Unreal intake: {report['status']}")
    print(f"Receipt: {REPORT_PATH}")
    if report["status"] == "ERROR":
        raise RuntimeError(report["error"])


main()
