"""Import and measure bounded WP-23.9 Bonelands candidates in UE 5.8."""

import hashlib
import io
import json
import traceback
import zipfile
from pathlib import Path

import unreal


ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
SOURCE = ROOT / "assets and old docs"
DRAGON_PACK = SOURCE / "Voxel+Dragons+Pack+Upload.zip"
CEMETERY_PACK = SOURCE / "voxel/cemetary and church voxel set.zip"
CATHEDRAL_PACK = SOURCE / "voxel/characters/Voxel Cathedral.zip"
KNIGHTS_PACK = SOURCE / "voxel/characters/knights.zip"
EXTRACTED = ROOT / "Saved/Diagnostics/WP23_9_Source"
REPORT_PATH = ROOT / "Saved/Diagnostics/WP23_9_bonelands_unreal_intake.json"
DEST = "/Game/WYRMFALL/Development/Intake/WP23_9"

CEMETERY_STEMS = (
    "church-0-cata_walldoor2",
    "church-12-cata_wallskull",
    "church-14-cata_floor",
    "church-4-Skele_mob",
    "church-46-skullpileCC",
    "church-55-coffin",
    "church-66-crypttop",
    "church-67-crypt",
    "church-77-gargoylepillar",
    "church-79-gargoyle",
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


def extract_named(archive_path, destination, members):
    extracted = []
    with zipfile.ZipFile(archive_path) as archive:
        by_normalized = {name.replace("\\", "/"): name for name in archive.namelist()}
        for normalized in members:
            original = by_normalized.get(normalized)
            if not original:
                raise RuntimeError(f"Missing member in {archive_path.name}: {normalized}")
            extracted.append(write_member(archive, original, destination / Path(normalized).name))
    return extracted


def extract_cemetery(destination):
    extracted = []
    with zipfile.ZipFile(CEMETERY_PACK) as outer:
        nested_name = next(name for name in outer.namelist() if name.casefold().endswith(".zip"))
        with zipfile.ZipFile(io.BytesIO(outer.read(nested_name))) as archive:
            by_normalized = {name.replace("\\", "/"): name for name in archive.namelist()}
            for stem in CEMETERY_STEMS:
                for suffix in (".obj", ".mtl", ".png"):
                    normalized = f"obj/{stem}{suffix}"
                    original = by_normalized.get(normalized)
                    if not original:
                        raise RuntimeError(f"Missing cemetery member: {normalized}")
                    extracted.append(write_member(archive, original, destination / f"{stem}{suffix}"))
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
                "object_path": asset.get_path_name(),
                "class": "SkeletalMesh",
                "bounds_origin": list(bounds.origin.to_tuple()),
                "bounds_extent": list(bounds.box_extent.to_tuple()),
                "bone_count": component.get_num_bones(),
                "skeleton": skeleton.get_path_name() if skeleton else None,
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
        "kind": "wp23_9_bonelands_unreal_intake",
        "engine": unreal.SystemLibrary.get_engine_version(),
        "scope": "ignored intake and measurements only; no profile, gameplay, map, travel, fact, Echo, or save acceptance",
        "status": "ERROR",
    }
    try:
        required = (DRAGON_PACK, CEMETERY_PACK, CATHEDRAL_PACK, KNIGHTS_PACK)
        missing = [str(path) for path in required if not path.is_file()]
        if missing:
            raise RuntimeError(f"Missing Bonelands candidate sources: {missing}")

        EXTRACTED.mkdir(parents=True, exist_ok=True)
        with zipfile.ZipFile(DRAGON_PACK) as archive:
            dragon_bytes = archive.read("GLTF/Skull Dragon.gltf")
        dragon_path = EXTRACTED / "Skull Dragon.gltf"
        dragon_path.write_bytes(dragon_bytes)
        dragon_json = json.loads(dragon_bytes)

        knight_files = extract_named(KNIGHTS_PACK, EXTRACTED / "Kael", [
            "Voxel Knights/FBX/Characters/TVS_VoxelKnights_Commander.fbx",
            "Voxel Knights/FBX/Characters/TVS_VoxelKnights_Champion.fbx",
            "Voxel Knights/OBJ/Characters/TVS_VoxelKnights_Commander.obj",
            "Voxel Knights/OBJ/Characters/TVS_VoxelKnights_Champion.obj",
            "Voxel Knights/Textures/Characters/TVS_VoxelKnights_Commander_Texture.png",
            "Voxel Knights/Textures/Characters/TVS_VoxelKnights_Champion_Texture.png",
        ])
        cathedral_files = extract_named(CATHEDRAL_PACK, EXTRACTED / "Cathedral", [
            "Voxel Cathedral/FBX/Characters/TVS_VoxelCathedral_Crusader.fbx",
            "Voxel Cathedral/OBJ/Characters/TVS_VoxelCathedral_Crusader.obj",
            "Voxel Cathedral/FBX/Environment/TVS_VoxelCathedral_Cathedral.fbx",
            "Voxel Cathedral/FBX/Environment/TVS_VoxelCathedral_CrossGrave.fbx",
            "Voxel Cathedral/FBX/Environment/TVS_VoxelCathedral_Statue.fbx",
            "Voxel Cathedral/Textures/Characters/TVS_VoxelCathedral_Crusader_Texture.png",
            "Voxel Cathedral/Textures/Environment/TVS_VoxelCathedral_Cathedral_Texture.png",
            "Voxel Cathedral/Textures/Environment/TVS_VoxelCathedral_CrossGrave_Texture.png",
            "Voxel Cathedral/Textures/Environment/TVS_VoxelCathedral_Statues_Texture.png",
        ])
        cemetery_files = extract_cemetery(EXTRACTED / "Cemetery")

        imports = {"ossuroth": import_asset(dragon_path, DEST + "/Ossuroth")}
        for source in knight_files:
            if source.suffix.casefold() == ".fbx":
                imports["kael_" + source.stem] = import_asset(source, DEST + "/Kael/" + source.stem)
            elif source.suffix.casefold() == ".obj":
                imports["kael_static_" + source.stem] = import_asset(source, DEST + "/Kael/Static/" + source.stem)
        for source in cathedral_files:
            if source.suffix.casefold() == ".fbx":
                group = "Kael" if "Crusader" in source.name else "Environment/Cathedral"
                imports["cathedral_" + source.stem] = import_asset(source, DEST + "/" + group + "/" + source.stem)
            elif source.suffix.casefold() == ".obj" and "Crusader" in source.name:
                imports["kael_static_" + source.stem] = import_asset(source, DEST + "/Kael/Static/" + source.stem)
        for source in cemetery_files:
            if source.suffix.casefold() == ".obj":
                imports["cemetery_" + source.stem] = import_asset(source, DEST + "/Environment/Cemetery/" + source.stem)

        asset_subsystem = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
        dragon_assets = summarize_assets(asset_subsystem, DEST + "/Ossuroth")
        skeletal = dragon_assets.get("SkeletalMesh", [])
        animations = dragon_assets.get("AnimSequence", [])
        required_motion = ("Idle", "Walk", "Flying", "Take_off", "Landing", "Attack")
        motion_hits = {
            name: [asset for asset in animations if name.casefold().replace("_", " ") in asset.casefold().replace("_", " ")]
            for name in required_motion
        }
        leader = next((name for name in skeletal if name.casefold() == "hip-local"), None)
        rig_ready = bool(leader and dragon_assets.get("Skeleton") and len(skeletal) >= 35 and len(animations) >= 15 and all(motion_hits.values()))
        kael_assets = summarize_assets(asset_subsystem, DEST + "/Kael")
        environment_assets = summarize_assets(asset_subsystem, DEST + "/Environment")
        kael_ready = len(kael_assets.get("StaticMesh", [])) + len(kael_assets.get("SkeletalMesh", [])) >= 3
        environment_ready = len(environment_assets.get("StaticMesh", [])) >= len(CEMETERY_STEMS) + 3

        report.update({
            "status": "PASS_INTAKE" if all((rig_ready, kael_ready, environment_ready)) else "PARTIAL",
            "sources": {
                "dragon_pack_sha256": sha256(DRAGON_PACK),
                "cemetery_pack_sha256": sha256(CEMETERY_PACK),
                "cathedral_pack_sha256": sha256(CATHEDRAL_PACK),
                "knights_pack_sha256": sha256(KNIGHTS_PACK),
            },
            "ossuroth": {
                "source": "assets and old docs/Voxel+Dragons+Pack+Upload.zip::GLTF/Skull Dragon.gltf",
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
                "rig_candidate_ready": rig_ready,
                "profile_validation": "NOT_RUN; DRG-15 remains fail-closed",
            },
            "kael_candidates": {"assets": kael_assets, "candidate_ready": kael_ready, "visual_selection": "NOT_RUN"},
            "environment_candidates": {
                "assets": environment_assets,
                "candidate_ready": environment_ready,
                "rendered_scene_fit": "NOT_RUN",
            },
            "unreal_intake": {"imports": imports, "measured_meshes": measured_meshes(asset_subsystem, DEST)},
            "remaining_gates": [
                "Rendered Ossuroth assembly and representative animation QA",
                "Rendered Commander, Champion, and Crusader Kael comparison",
                "Rendered crypt, cathedral, grave, skeleton, and gargoyle environment fit",
                "Distinct non-franchise wrapped guardian and Skinning Man presentations",
            ],
        })
    except Exception:
        report["status"] = "ERROR"
        report["error"] = traceback.format_exc()
    finally:
        REPORT_PATH.parent.mkdir(parents=True, exist_ok=True)
        REPORT_PATH.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")

    print(f"WP-23.9 Bonelands Unreal intake: {report['status']}")
    print(f"Receipt: {REPORT_PATH}")
    if report["status"] == "ERROR":
        raise RuntimeError(report["error"])


main()
