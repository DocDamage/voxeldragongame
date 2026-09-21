"""Import and measure the bounded WP-23.8 Ashen Wastes candidates in UE 5.8."""

import hashlib
import json
import traceback
import zipfile
from pathlib import Path

import unreal


ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
SOURCE = ROOT / "assets and old docs"
DRAGON_PACK = SOURCE / "Voxel+Dragons+Pack+Upload.zip"
MONOGON_PACK = SOURCE / "voxel/monogon voxels.zip"
SAND_PACK = SOURCE / "voxel/Voxel_Sand___Beach_Pack_-_35_Assets__Static___Animated_-b0ef1ece.zip"
WATER_PACK = SOURCE / "voxel/Voxel_Water___Aquatic_Pack_-_115_Assets__Static___Animated_-8b7379eb.zip"

EXTRACTED = ROOT / "Saved/Diagnostics/WP23_8_Source"
REPORT_PATH = ROOT / "Saved/Diagnostics/WP23_8_ashen_wastes_unreal_intake.json"
DEST = "/Game/WYRMFALL/Development/Intake/WP23_8"

BUNKER_STEMS = (
    "Bunker - Free Sample-0-Wall_Container_Blue-0",
    "Bunker - Free Sample-30-Chair_Red-2",
    "Bunker - Free Sample-70-Crate-0",
    "Bunker - Free Sample-81-Shelf-3",
    "Bunker - Free Sample-84-Documents_colored-2",
    "Bunker - Free Sample-85-WoundCable-1",
    "Bunker - Free Sample-88-Lantern-1",
    "Bunker - Free Sample-95-Floor_Dark_Gray-4",
    "Bunker - Free Sample-97-Girder",
    "Bunker - Free Sample-98-StorageBin",
)


def sha256(path):
    value = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            value.update(chunk)
    return value.hexdigest()


def write_member(archive, member, destination):
    destination.parent.mkdir(parents=True, exist_ok=True)
    destination.write_bytes(archive.read(member))
    return destination


def extract_suffixes(archive_path, destination, suffixes):
    extracted = []
    with zipfile.ZipFile(archive_path) as archive:
        for original in archive.namelist():
            normalized = original.replace("\\", "/")
            if any(normalized.endswith(suffix) for suffix in suffixes):
                extracted.append(write_member(archive, original, destination / Path(normalized).name))
    missing = [suffix for suffix in suffixes if not any(path.name == Path(suffix).name for path in extracted)]
    if missing:
        raise RuntimeError(f"Missing expected members in {archive_path.name}: {missing}")
    return extracted


def extract_bunker(destination):
    extracted = []
    prefix = "Voxel Nuclear Bunker/Free Sample/"
    with zipfile.ZipFile(MONOGON_PACK) as archive:
        by_normalized = {name.replace("\\", "/"): name for name in archive.namelist()}
        for stem in BUNKER_STEMS:
            for suffix in (".obj", ".png"):
                normalized = prefix + stem + suffix
                original = by_normalized.get(normalized)
                if not original:
                    raise RuntimeError(f"Missing bunker member: {normalized}")
                extracted.append(write_member(archive, original, destination / (stem + suffix)))
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


def summarize_assets(asset_subsystem, destination):
    summary = {}
    for path in asset_subsystem.list_assets(destination, recursive=True, include_folder=False):
        asset = asset_subsystem.load_asset(path)
        if asset:
            summary.setdefault(asset.get_class().get_name(), []).append(asset.get_name())
    return {key: sorted(set(value)) for key, value in sorted(summary.items())}


def material_paths(slots):
    return [
        slot.material_interface.get_path_name() if slot.material_interface else None
        for slot in slots
    ]


def measured_meshes(asset_subsystem, destination):
    rows = []
    for path in asset_subsystem.list_assets(destination, recursive=True, include_folder=False):
        asset = asset_subsystem.load_asset(path)
        if isinstance(asset, unreal.SkeletalMesh):
            bounds = asset.get_imported_bounds()
            component = unreal.SkeletalMeshComponent()
            component.set_skinned_asset_and_update(asset)
            skeleton = asset.get_editor_property("skeleton")
            physics = asset.get_editor_property("physics_asset")
            rows.append({
                "object_path": asset.get_path_name(),
                "class": "SkeletalMesh",
                "bounds_origin": list(bounds.origin.to_tuple()),
                "bounds_extent": list(bounds.box_extent.to_tuple()),
                "bone_count": component.get_num_bones(),
                "skeleton": skeleton.get_path_name() if skeleton else None,
                "physics_asset": physics.get_path_name() if physics else None,
                "materials": material_paths(asset.get_editor_property("materials")),
            })
        elif isinstance(asset, unreal.StaticMesh):
            box = asset.get_bounding_box()
            rows.append({
                "object_path": asset.get_path_name(),
                "class": "StaticMesh",
                "bounds_min": list(box.min.to_tuple()),
                "bounds_max": list(box.max.to_tuple()),
                "body_setup_present": asset.get_editor_property("body_setup") is not None,
                "materials": material_paths(asset.get_editor_property("static_materials")),
            })
    return rows


def main():
    report = {
        "kind": "wp23_8_ashen_wastes_unreal_intake",
        "engine": unreal.SystemLibrary.get_engine_version(),
        "scope": "ignored intake and measurements only; no profile, gameplay, map, travel, fact, Echo, or save acceptance",
        "status": "ERROR",
    }
    try:
        required = (DRAGON_PACK, MONOGON_PACK, SAND_PACK, WATER_PACK)
        missing = [str(path) for path in required if not path.is_file()]
        if missing:
            raise RuntimeError(f"Missing Ashen Wastes candidate sources: {missing}")

        EXTRACTED.mkdir(parents=True, exist_ok=True)
        with zipfile.ZipFile(DRAGON_PACK) as archive:
            dragon_bytes = archive.read("GLTF/Zombie Dragon.gltf")
        dragon_path = EXTRACTED / "Zombie Dragon.gltf"
        dragon_path.write_bytes(dragon_bytes)
        dragon_json = json.loads(dragon_bytes)

        apocalypse_files = extract_suffixes(MONOGON_PACK, EXTRACTED / "Apocalypse", [
            "/Character_Hero.fbx",
            "/Character_Zombie.fbx",
            "/VoxelApocalypse_Character.png",
        ])
        bunker_files = extract_bunker(EXTRACTED / "Bunker")
        desert_files = extract_suffixes(SAND_PACK, EXTRACTED / "DesertRuin", [
            "/desert_ruin/desert_ruin.fbx",
            "/desert_ruin/palette.png",
        ])
        toxic_files = extract_suffixes(WATER_PACK, EXTRACTED / "ToxicPuddle", [
            "/toxic_puddle/toxic_puddle.fbx",
            "/toxic_puddle/palette.png",
        ])

        imports = {
            "rotwing": import_asset(dragon_path, DEST + "/Rotwing"),
            "rotking_hero": import_asset(
                next(path for path in apocalypse_files if path.name == "Character_Hero.fbx"),
                DEST + "/Rotking/Hero"),
            "rotking_zombie": import_asset(
                next(path for path in apocalypse_files if path.name == "Character_Zombie.fbx"),
                DEST + "/Rotking/Zombie"),
            "desert_ruin": import_asset(
                next(path for path in desert_files if path.suffix.casefold() == ".fbx"),
                DEST + "/Environment/DesertRuin"),
            "toxic_puddle": import_asset(
                next(path for path in toxic_files if path.suffix.casefold() == ".fbx"),
                DEST + "/Environment/ToxicPuddle"),
        }
        for source in bunker_files:
            if source.suffix.casefold() == ".obj":
                imports["bunker_" + source.stem] = import_asset(source, DEST + "/Environment/Bunker/" + source.stem)

        asset_subsystem = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
        dragon_assets = summarize_assets(asset_subsystem, DEST + "/Rotwing")
        skeletal = dragon_assets.get("SkeletalMesh", [])
        animations = dragon_assets.get("AnimSequence", [])
        required_motion = ("Idle", "Walk", "Flying", "Take_off", "Landing", "Attack")
        motion_hits = {
            name: [asset for asset in animations if name.casefold().replace("_", " ") in asset.casefold().replace("_", " ")]
            for name in required_motion
        }
        leader = next((name for name in skeletal if name.casefold() == "hip-local"), None)
        rig_candidate_ready = bool(
            leader and dragon_assets.get("Skeleton") and len(skeletal) >= 30
            and len(animations) >= 15 and all(motion_hits.values())
        )
        rotking_assets = summarize_assets(asset_subsystem, DEST + "/Rotking")
        environment_assets = summarize_assets(asset_subsystem, DEST + "/Environment")
        rotking_ready = len(rotking_assets.get("StaticMesh", [])) + len(rotking_assets.get("SkeletalMesh", [])) >= 2
        environment_ready = len(environment_assets.get("StaticMesh", [])) >= len(BUNKER_STEMS) + 2

        report.update({
            "status": "PASS_INTAKE" if all((rig_candidate_ready, rotking_ready, environment_ready)) else "PARTIAL",
            "sources": {
                "dragon_pack_sha256": sha256(DRAGON_PACK),
                "monogon_pack_sha256": sha256(MONOGON_PACK),
                "sand_pack_sha256": sha256(SAND_PACK),
                "water_pack_sha256": sha256(WATER_PACK),
            },
            "rotwing": {
                "source": "assets and old docs/Voxel+Dragons+Pack+Upload.zip::GLTF/Zombie Dragon.gltf",
                "gltf": {
                    "nodes": len(dragon_json.get("nodes", [])),
                    "meshes": len(dragon_json.get("meshes", [])),
                    "animations": len(dragon_json.get("animations", [])),
                    "materials": len(dragon_json.get("materials", [])),
                },
                "unreal_assets": dragon_assets,
                "leader_mesh": leader,
                "follower_mesh_count": max(0, len(skeletal) - (1 if leader else 0)),
                "motion_coverage": motion_hits,
                "rig_candidate_ready": rig_candidate_ready,
                "profile_validation": "NOT_RUN; DRG-15 remains fail-closed",
            },
            "rotking_candidates": {
                "assets": rotking_assets,
                "candidate_ready": rotking_ready,
                "visual_selection": "NOT_RUN; neither candidate is accepted by this intake",
            },
            "environment_candidates": {
                "assets": environment_assets,
                "candidate_ready": environment_ready,
                "rendered_scene_fit": "NOT_RUN",
                "laboratory": "BLOCKED_NO_EXACT_SUPPLIED_SOURCE",
            },
            "unreal_intake": {
                "imports": imports,
                "measured_meshes": measured_meshes(asset_subsystem, DEST),
            },
            "remaining_gates": [
                "Rendered Rotwing modular assembly and representative animation QA",
                "Rendered Hero versus Zombie Rotking comparison and explicit selection or rejection",
                "Rendered bunker, desert ruin, and toxic puddle scene fit",
                "Exact laboratory and distinct Doctor Hollowmend/false-rescuer presentation sources",
            ],
        })
    except Exception:
        report["status"] = "ERROR"
        report["error"] = traceback.format_exc()
    finally:
        REPORT_PATH.parent.mkdir(parents=True, exist_ok=True)
        REPORT_PATH.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")

    print(f"WP-23.8 Ashen Wastes Unreal intake: {report['status']}")
    print(f"Receipt: {REPORT_PATH}")
    if report["status"] == "ERROR":
        raise RuntimeError(report["error"])


main()
