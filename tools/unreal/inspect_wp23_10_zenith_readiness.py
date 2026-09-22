"""Import and measure bounded WP-23.10 Zenith candidates in UE 5.8."""

import hashlib
import json
import traceback
import zipfile
from pathlib import Path

import unreal


ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
SOURCE = ROOT / "assets and old docs"
DRAGON_PACK = SOURCE / "Voxel+Dragons+Pack+Upload.zip"
COLONY_PACK = SOURCE / "voxel/monogon voxels.zip"
ROBOT_PACK = SOURCE / "voxel/modular robots/Modular_Robots.zip"
EXTRACTED = ROOT / "Saved/Diagnostics/WP23_10_Source"
REPORT_PATH = ROOT / "Saved/Diagnostics/WP23_10_zenith_unreal_intake.json"
DEST = "/Game/WYRMFALL/Development/Intake/WP23_10"


def sha256(path):
    value = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            value.update(chunk)
    return value.hexdigest()


def extract_named(archive_path, destination, members):
    extracted = []
    with zipfile.ZipFile(archive_path) as archive:
        by_normalized = {name.replace("\\", "/"): name for name in archive.namelist()}
        for normalized in members:
            original = by_normalized.get(normalized)
            if not original:
                raise RuntimeError(f"Missing member in {archive_path.name}: {normalized}")
            target = destination / Path(normalized).name
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_bytes(archive.read(original))
            extracted.append(target)
    return extracted


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
    summary = {}
    for path in asset_subsystem.list_assets(destination, recursive=True, include_folder=False):
        asset = asset_subsystem.load_asset(path)
        if asset:
            summary.setdefault(asset.get_class().get_name(), []).append(asset.get_name())
    return {key: sorted(set(value)) for key, value in sorted(summary.items())}


def material_paths(slots):
    return [slot.material_interface.get_path_name() if slot.material_interface else None for slot in slots]


def measured_meshes(asset_subsystem, destination):
    rows = []
    for path in asset_subsystem.list_assets(destination, recursive=True, include_folder=False):
        asset = asset_subsystem.load_asset(path)
        if isinstance(asset, unreal.SkeletalMesh):
            bounds = asset.get_imported_bounds()
            component = unreal.SkeletalMeshComponent()
            component.set_skinned_asset_and_update(asset)
            skeleton = asset.get_editor_property("skeleton")
            rows.append({
                "object_path": asset.get_path_name(), "class": "SkeletalMesh",
                "bounds_origin": list(bounds.origin.to_tuple()),
                "bounds_extent": list(bounds.box_extent.to_tuple()),
                "bone_count": component.get_num_bones(),
                "skeleton": skeleton.get_path_name() if skeleton else None,
                "materials": material_paths(asset.get_editor_property("materials")),
            })
        elif isinstance(asset, unreal.StaticMesh):
            box = asset.get_bounding_box()
            rows.append({
                "object_path": asset.get_path_name(), "class": "StaticMesh",
                "bounds_min": list(box.min.to_tuple()), "bounds_max": list(box.max.to_tuple()),
                "body_setup_present": asset.get_editor_property("body_setup") is not None,
                "materials": material_paths(asset.get_editor_property("static_materials")),
            })
    return rows


def main():
    report = {
        "kind": "wp23_10_zenith_unreal_intake",
        "engine": unreal.SystemLibrary.get_engine_version(),
        "scope": "ignored intake and measurements only; no profile, city, colony, gameplay, travel, fact, Echo, or save acceptance",
        "status": "ERROR",
    }
    try:
        required = (DRAGON_PACK, COLONY_PACK, ROBOT_PACK)
        missing = [str(path) for path in required if not path.is_file()]
        if missing:
            raise RuntimeError(f"Missing Zenith candidate sources: {missing}")

        EXTRACTED.mkdir(parents=True, exist_ok=True)
        with zipfile.ZipFile(DRAGON_PACK) as archive:
            dragon_bytes = archive.read("GLTF/Mecha Dragon.gltf")
        dragon_path = EXTRACTED / "Mecha Dragon.gltf"
        dragon_path.write_bytes(dragon_bytes)
        dragon_json = json.loads(dragon_bytes)

        colony_root = "Voxel Space Colony/FreeSample"
        colony_files = extract_named(COLONY_PACK, EXTRACTED / "Colony", [
            f"{colony_root}/BuildingBlock_2.fbx", f"{colony_root}/GroundTile_008.fbx",
            f"{colony_root}/Lander.fbx", f"{colony_root}/SatelliteDish_1.fbx",
            f"{colony_root}/SolarPanel_4.fbx", f"{colony_root}/Prop_14.fbx",
        ])

        robot_members = []
        for index in (0, 10, 20, 30, 40, 50, 60, 70, 78):
            robot_members.extend([f"Robot.vox-{index}.obj", f"Robot.vox-{index}.mtl", f"Robot.vox-{index}.png"])
        robot_files = extract_named(ROBOT_PACK, EXTRACTED / "Robots", robot_members)

        imports = {"mecha_dragon": import_asset(dragon_path, DEST + "/MechaDragon")}
        for source in colony_files:
            if source.suffix.casefold() == ".fbx":
                imports["colony_" + source.stem] = import_asset(source, DEST + "/Colony/" + source.stem)
        for source in robot_files:
            if source.suffix.casefold() == ".obj":
                imports["robot_" + source.stem] = import_asset(source, DEST + "/Robots/" + source.stem.replace(".", "_"))

        assets = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
        dragon_assets = summarize(assets, DEST + "/MechaDragon")
        skeletal = dragon_assets.get("SkeletalMesh", [])
        animations = dragon_assets.get("AnimSequence", [])
        required_motion = ("Idle", "Walk", "Flying", "Take_off", "Landing", "Attack")
        motion_hits = {
            name: [asset for asset in animations if name.casefold().replace("_", " ") in asset.casefold().replace("_", " ")]
            for name in required_motion
        }
        leader = next((name for name in skeletal if name.casefold() == "hip-local"), None)
        rig_ready = bool(leader and dragon_assets.get("Skeleton") and len(skeletal) == 35 and len(animations) >= 15 and all(motion_hits.values()))
        colony_assets = summarize(assets, DEST + "/Colony")
        robot_assets = summarize(assets, DEST + "/Robots")
        report.update({
            "status": "PASS_INTAKE" if rig_ready and len(colony_assets.get("SkeletalMesh", [])) >= 6 and len(robot_assets.get("StaticMesh", [])) >= 9 else "PARTIAL",
            "sources": {path.name: sha256(path) for path in required},
            "mecha_dragon": {
                "source": "assets and old docs/Voxel+Dragons+Pack+Upload.zip::GLTF/Mecha Dragon.gltf",
                "gltf": {key: len(dragon_json.get(key, [])) for key in ("nodes", "meshes", "animations", "materials")},
                "unreal_assets": dragon_assets,
                "leader_mesh": leader,
                "follower_mesh_count": max(0, len(skeletal) - (1 if leader else 0)),
                "motion_coverage": motion_hits,
                "rig_candidate_ready": rig_ready,
                "profile_validation": "NOT_RUN; Zenith Mecha Dragon profile remains fail-closed",
            },
            "colony_candidates": {
                "assets": colony_assets,
                "source_member_count": 21,
                "source_character": "exterior tiles and props; no supplied enclosed interior route identified",
                "playable_colony_validation": "NOT_RUN",
            },
            "robot_candidates": {
                "assets": robot_assets,
                "source_member_count": 79,
                "source_character": "independent OBJ pieces; authored character assembly and animation not established",
                "population_validation": "NOT_RUN",
            },
            "unreal_intake": {"imports": imports, "measured_meshes": measured_meshes(assets, DEST)},
            "remaining_gates": [
                "Rendered coherent Mecha Dragon assembly and representative animation QA",
                "Playable colony arrival hub, enclosed interior route, objective, and return proof",
                "Authored robot population and distinct Overking Vantrix-9 presentation",
                "Distinct Broodmother Cipher infestation-dungeon presentation",
            ],
        })
    except Exception:
        report["status"] = "ERROR"
        report["error"] = traceback.format_exc()
    finally:
        REPORT_PATH.parent.mkdir(parents=True, exist_ok=True)
        REPORT_PATH.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(f"WP-23.10 Zenith Unreal intake: {report['status']}")
    print(f"Receipt: {REPORT_PATH}")
    if report["status"] == "ERROR":
        raise RuntimeError(report["error"])


main()
