"""Import and measure bounded WP-23.4 Frosthold candidates in UE 5.8."""

import hashlib
import json
import traceback
import zipfile
from pathlib import Path

import unreal


ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
SOURCE = ROOT / "assets and old docs"
DRAGON_PACK = SOURCE / "Voxel+Dragons+Pack+Upload.zip"
HEX_PACK = SOURCE / "voxel/hexagon set/Hexatilesv1.zip"
WATER_PACK = SOURCE / "voxel/Voxel_Water___Aquatic_Pack_-_115_Assets__Static___Animated_-8b7379eb.zip"
GRASS_PACK = SOURCE / "voxel/Voxel_Grass___Mushrooms_Pack_-_84_Assets__Static___Animated_-ebf10438.zip"
KNIGHTS_PACK = SOURCE / "voxel/characters/knights.zip"
PALACE_PACK = SOURCE / "voxel/characters/palace.zip"
EXTRACTED = ROOT / "Saved/Diagnostics/WP23_4_Source"
REPORT_PATH = ROOT / "Saved/Diagnostics/WP23_4_frosthold_unreal_intake.json"
DEST = "/Game/WYRMFALL/Development/Intake/WP23_4"


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
        "kind": "wp23_4_frosthold_unreal_intake",
        "engine": unreal.SystemLibrary.get_engine_version(),
        "scope": "ignored intake and measurements only; no profile, map, gameplay, travel, fact, Echo, or save acceptance",
        "status": "ERROR",
    }
    try:
        required = (DRAGON_PACK, HEX_PACK, WATER_PACK, GRASS_PACK, KNIGHTS_PACK, PALACE_PACK)
        missing = [str(path) for path in required if not path.is_file()]
        if missing:
            raise RuntimeError(f"Missing Frosthold candidate sources: {missing}")

        EXTRACTED.mkdir(parents=True, exist_ok=True)
        with zipfile.ZipFile(DRAGON_PACK) as archive:
            dragon_bytes = archive.read("GLTF/White Dragon.gltf")
        dragon_path = EXTRACTED / "White Dragon.gltf"
        dragon_path.write_bytes(dragon_bytes)
        dragon_json = json.loads(dragon_bytes)

        ruler_files = extract_named(KNIGHTS_PACK, EXTRACTED / "Rulers", [
            "Voxel Knights/FBX/Characters/TVS_VoxelKnights_Captain.fbx",
            "Voxel Knights/FBX/Characters/TVS_VoxelKnights_Champion.fbx",
            "Voxel Knights/Textures/Characters/TVS_VoxelKnights_Captain_Texture.png",
            "Voxel Knights/Textures/Characters/TVS_VoxelKnights_Champion_Texture.png",
        ]) + extract_named(PALACE_PACK, EXTRACTED / "Rulers", [
            "Voxel Palace/FBX/Characters/TVS_VoxelPalace_King.fbx",
            "Voxel Palace/Textures/Characters/TVS_VoxelPalace_King_Texture.png",
        ])
        fort_files = extract_named(KNIGHTS_PACK, EXTRACTED / "Fort", [
            "Voxel Knights/FBX/Environment/TVS_VoxelKnights_Keep.fbx",
            "Voxel Knights/FBX/Environment/TVS_VoxelKnights_Tower.fbx",
            "Voxel Knights/FBX/Environment/TVS_VoxelKnights_Gate.fbx",
            "Voxel Knights/Textures/Environment/TVS_VoxelKnights_Keep_Texture.png",
            "Voxel Knights/Textures/Environment/TVS_VoxelKnights_Tower_Texture.png",
            "Voxel Knights/Textures/Environment/TVS_VoxelKnights_Gate_Texture.png",
        ])
        hex_files = extract_named(HEX_PACK, EXTRACTED / "SnowTiles", [
            "Obj/overworld-10-snow_icelake.obj", "Obj/overworld-10-snow_icelake.mtl", "Obj/overworld-10-snow_icelake.png",
            "Obj/overworld-8-snow_igloo.obj", "Obj/overworld-8-snow_igloo.mtl", "Obj/overworld-8-snow_igloo.png",
            "Obj/overworld-7-props_igloo.obj", "Obj/overworld-7-props_igloo.mtl", "Obj/overworld-7-props_igloo.png",
        ])
        water_members = []
        water_root = "Voxel_Water___Aquatic_Pack_-_115_Assets__Static___Animated_-8b7379eb/fbx/free_water_fbx_extracted/Models/Static"
        for stem in ("frozen_lake", "frozen_river", "frozen_waterfall"):
            water_members += [f"{water_root}/{stem}/{stem}.fbx", f"{water_root}/{stem}/palette.png"]
        water_files = extract_named(WATER_PACK, EXTRACTED / "FrozenWater", water_members)
        grass_members = []
        grass_root = "Voxel_Grass___Mushrooms_Pack_-_84_Assets__Static___Animated_-ebf10438/fbx/free_grass_and_mushrooms_extracted/Models/Static"
        for stem in ("arctic_grass", "arctic_lichen", "frosted_grass"):
            grass_members += [f"{grass_root}/{stem}/{stem}.fbx", f"{grass_root}/{stem}/palette.png"]
        grass_files = extract_named(GRASS_PACK, EXTRACTED / "ArcticGround", grass_members)

        imports = {"frostmane": import_asset(dragon_path, DEST + "/Frostmane")}
        for source in ruler_files:
            if source.suffix.casefold() == ".fbx":
                imports["ruler_" + source.stem] = import_asset(source, DEST + "/Rulers/" + source.stem)
        for source in fort_files:
            if source.suffix.casefold() == ".fbx":
                imports["fort_" + source.stem] = import_asset(source, DEST + "/Environment/Fort/" + source.stem)
        for group, files in (("SnowTiles", hex_files), ("FrozenWater", water_files), ("ArcticGround", grass_files)):
            for source in files:
                if source.suffix.casefold() in {".fbx", ".obj"}:
                    imports[group + "_" + source.stem] = import_asset(source, DEST + "/Environment/" + group + "/" + source.stem)

        assets = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
        dragon_assets = summarize(assets, DEST + "/Frostmane")
        skeletal = dragon_assets.get("SkeletalMesh", [])
        animations = dragon_assets.get("AnimSequence", [])
        required_motion = ("Idle", "Walk", "Flying", "Take_off", "Landing", "Attack")
        motion_hits = {
            name: [asset for asset in animations if name.casefold().replace("_", " ") in asset.casefold().replace("_", " ")]
            for name in required_motion
        }
        leader = next((name for name in skeletal if name.casefold() == "hip-local"), None)
        rig_ready = bool(leader and dragon_assets.get("Skeleton") and len(skeletal) == 37 and len(animations) >= 15 and all(motion_hits.values()))
        ruler_assets = summarize(assets, DEST + "/Rulers")
        environment_assets = summarize(assets, DEST + "/Environment")
        report.update({
            "status": "PASS_INTAKE" if rig_ready and len(ruler_assets.get("SkeletalMesh", [])) >= 3 and len(environment_assets.get("StaticMesh", [])) >= 10 else "PARTIAL",
            "sources": {path.name: sha256(path) for path in required},
            "frostmane": {
                "source": "assets and old docs/Voxel+Dragons+Pack+Upload.zip::GLTF/White Dragon.gltf",
                "gltf": {key: len(dragon_json.get(key, [])) for key in ("nodes", "meshes", "animations", "materials")},
                "unreal_assets": dragon_assets,
                "leader_mesh": leader,
                "follower_mesh_count": max(0, len(skeletal) - (1 if leader else 0)),
                "motion_coverage": motion_hits,
                "rig_candidate_ready": rig_ready,
                "profile_validation": "NOT_RUN; DRG-15 remains fail-closed",
            },
            "ruler_candidates": {"assets": ruler_assets, "visual_selection": "NOT_RUN"},
            "environment_candidates": {"assets": environment_assets, "rendered_scene_fit": "NOT_RUN"},
            "unreal_intake": {"imports": imports, "measured_meshes": measured_meshes(assets, DEST)},
            "remaining_gates": [
                "Rendered Frostmane assembly and representative animation QA",
                "Rendered distinct Alaric candidate comparison",
                "Rendered snow, ice, fort, and preserved/stilled-detail environment fit",
                "Distinct prisoners' nightmare presentation",
            ],
        })
    except Exception:
        report["status"] = "ERROR"
        report["error"] = traceback.format_exc()
    finally:
        REPORT_PATH.parent.mkdir(parents=True, exist_ok=True)
        REPORT_PATH.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(f"WP-23.4 Frosthold Unreal intake: {report['status']}")
    print(f"Receipt: {REPORT_PATH}")
    if report["status"] == "ERROR":
        raise RuntimeError(report["error"])


main()
