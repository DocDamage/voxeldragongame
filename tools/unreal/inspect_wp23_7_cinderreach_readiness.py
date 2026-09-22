"""Import and measure bounded WP-23.7 Cinderreach candidates in UE 5.8."""

import hashlib
import json
import traceback
import zipfile
from pathlib import Path

import unreal


ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
SOURCE = ROOT / "assets and old docs"
DRAGON_PACK = SOURCE / "Voxel+Dragons+Pack+Upload.zip"
WATER_PACK = SOURCE / "voxel/Voxel_Water___Aquatic_Pack_-_115_Assets__Static___Animated_-8b7379eb.zip"
SAND_PACK = SOURCE / "voxel/Voxel_Sand___Beach_Pack_-_35_Assets__Static___Animated_-b0ef1ece.zip"
DUNGEON_PACK = SOURCE / "voxel/Free_Voxel_Dungeon___Execution_Starter_Kit_for_Indie_Games_-3cdb8f01.zip"
KNIGHTS_PACK = SOURCE / "voxel/characters/knights.zip"
CATHEDRAL_PACK = SOURCE / "voxel/characters/Voxel Cathedral.zip"
EXTRACTED = ROOT / "Saved/Diagnostics/WP23_7_Source"
REPORT_PATH = ROOT / "Saved/Diagnostics/WP23_7_cinderreach_unreal_intake.json"
DEST = "/Game/WYRMFALL/Development/Intake/WP23_7"


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
        "kind": "wp23_7_cinderreach_unreal_intake",
        "engine": unreal.SystemLibrary.get_engine_version(),
        "scope": "ignored intake and measurements only; no profile, map, gameplay, travel, fact, Echo, or save acceptance",
        "status": "ERROR",
    }
    try:
        required = (DRAGON_PACK, WATER_PACK, SAND_PACK, DUNGEON_PACK, KNIGHTS_PACK, CATHEDRAL_PACK)
        missing = [str(path) for path in required if not path.is_file()]
        if missing:
            raise RuntimeError(f"Missing Cinderreach candidate sources: {missing}")

        EXTRACTED.mkdir(parents=True, exist_ok=True)
        with zipfile.ZipFile(DRAGON_PACK) as archive:
            dragon_bytes = archive.read("GLTF/Lava Dragon.gltf")
        dragon_path = EXTRACTED / "Lava Dragon.gltf"
        dragon_path.write_bytes(dragon_bytes)
        dragon_json = json.loads(dragon_bytes)

        ruler_files = extract_named(KNIGHTS_PACK, EXTRACTED / "Cast", [
            "Voxel Knights/FBX/Characters/TVS_VoxelKnights_Champion.fbx",
            "Voxel Knights/FBX/Characters/TVS_VoxelKnights_Commander.fbx",
            "Voxel Knights/Textures/Characters/TVS_VoxelKnights_Champion_Texture.png",
            "Voxel Knights/Textures/Characters/TVS_VoxelKnights_Commander_Texture.png",
        ])
        overseer_files = extract_named(CATHEDRAL_PACK, EXTRACTED / "Cast", [
            "Voxel Cathedral/FBX/Characters/TVS_VoxelCathedral_Priest.fbx",
            "Voxel Cathedral/Textures/Characters/TVS_VoxelCathedral_Priest_Texture.png",
        ])

        water_root = "Voxel_Water___Aquatic_Pack_-_115_Assets__Static___Animated_-8b7379eb/fbx/free_water_fbx_extracted/Models/Static"
        water_members = []
        for stem in ("lava_fountain", "lava_lake", "lava_river", "lava_waterfall"):
            water_members += [f"{water_root}/{stem}/{stem}.fbx", f"{water_root}/{stem}/palette.png"]
        lava_files = extract_named(WATER_PACK, EXTRACTED / "Lava", water_members)

        sand_root = "Voxel_Sand___Beach_Pack_-_35_Assets__Static___Animated_-b0ef1ece/fbx/free_sand_and_beach_fbx_extracted/Models/Static"
        sand_members = []
        for stem in ("arena_sand", "ash_dune", "obsidian_shore"):
            sand_members += [f"{sand_root}/{stem}/{stem}.fbx", f"{sand_root}/{stem}/palette.png"]
        ground_files = extract_named(SAND_PACK, EXTRACTED / "Ground", sand_members)

        dungeon_root = "Free_Voxel_Dungeon___Execution_Starter_Kit_for_Indie_Games_-3cdb8f01/fbx/pack_15_dungeon_extracted/Pack - 15 Dungeon"
        dungeon_files = extract_named(DUNGEON_PACK, EXTRACTED / "Dungeon", [
            f"{dungeon_root}/Dungeon1.fbx", f"{dungeon_root}/Pillar.fbx",
            f"{dungeon_root}/Torch1.fbx", f"{dungeon_root}/Chains1.fbx",
            f"{dungeon_root}/ExecutionBlock.fbx", f"{dungeon_root}/TortureCage1.fbx",
        ])

        imports = {"pyraxis": import_asset(dragon_path, DEST + "/Pyraxis")}
        for group, files in (
            ("Cast/Rulers", ruler_files), ("Cast/Overseer", overseer_files),
            ("Environment/Lava", lava_files), ("Environment/Ground", ground_files),
            ("Environment/Dungeon", dungeon_files),
        ):
            for source in files:
                if source.suffix.casefold() in {".fbx", ".obj"}:
                    imports[group.replace("/", "_") + "_" + source.stem] = import_asset(source, DEST + "/" + group + "/" + source.stem)

        assets = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
        dragon_assets = summarize(assets, DEST + "/Pyraxis")
        skeletal = dragon_assets.get("SkeletalMesh", [])
        animations = dragon_assets.get("AnimSequence", [])
        required_motion = ("Idle", "Walk", "Flying", "Take_off", "Landing", "Attack")
        motion_hits = {
            name: [asset for asset in animations if name.casefold().replace("_", " ") in asset.casefold().replace("_", " ")]
            for name in required_motion
        }
        leader = next((name for name in skeletal if name.casefold() == "hip-local"), None)
        rig_ready = bool(leader and dragon_assets.get("Skeleton") and len(skeletal) == 35 and len(animations) >= 15 and all(motion_hits.values()))
        cast_assets = summarize(assets, DEST + "/Cast")
        environment_assets = summarize(assets, DEST + "/Environment")
        report.update({
            "status": "PASS_INTAKE" if rig_ready and len(cast_assets.get("SkeletalMesh", [])) >= 3 and len(environment_assets.get("StaticMesh", [])) >= 12 else "PARTIAL",
            "sources": {path.name: sha256(path) for path in required},
            "pyraxis": {
                "source": "assets and old docs/Voxel+Dragons+Pack+Upload.zip::GLTF/Lava Dragon.gltf",
                "gltf": {key: len(dragon_json.get(key, [])) for key in ("nodes", "meshes", "animations", "materials")},
                "unreal_assets": dragon_assets,
                "leader_mesh": leader,
                "follower_mesh_count": max(0, len(skeletal) - (1 if leader else 0)),
                "motion_coverage": motion_hits,
                "rig_candidate_ready": rig_ready,
                "profile_validation": "NOT_RUN; DRG-15 remains fail-closed",
            },
            "cast_candidates": {"assets": cast_assets, "visual_selection": "NOT_RUN"},
            "environment_candidates": {"assets": environment_assets, "rendered_scene_fit": "NOT_RUN"},
            "unreal_intake": {"imports": imports, "measured_meshes": measured_meshes(assets, DEST)},
            "remaining_gates": [
                "Rendered Pyraxis assembly and representative animation QA",
                "Rendered distinct Magnarok candidate comparison",
                "Rendered volcanic forge and readable ritual-arena fit",
                "Distinct non-franchise Flayed Choir overseer presentation",
            ],
        })
    except Exception:
        report["status"] = "ERROR"
        report["error"] = traceback.format_exc()
    finally:
        REPORT_PATH.parent.mkdir(parents=True, exist_ok=True)
        REPORT_PATH.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(f"WP-23.7 Cinderreach Unreal intake: {report['status']}")
    print(f"Receipt: {REPORT_PATH}")
    if report["status"] == "ERROR":
        raise RuntimeError(report["error"])


main()
