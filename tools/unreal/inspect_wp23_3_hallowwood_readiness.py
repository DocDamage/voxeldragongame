"""Import and measure the bounded WP-23.3 Hallowwood candidates in UE 5.8."""

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
FOREST_PACK = SOURCE / "voxel/Low_poly_voxel_forest-c8c1e081.zip"
PARK_PACK = SOURCE / "voxel/playground park.zip"
VILLAGERS = SOURCE / "voxel/characters/villagers.zip"
WIZARDS = SOURCE / "voxel/characters/wizards.zip"

EXTRACTED = ROOT / "Saved/Diagnostics/WP23_3_Source"
REPORT_PATH = ROOT / "Saved/Diagnostics/WP23_3_hallowwood_unreal_intake.json"
DEST = "/Game/WYRMFALL/Development/Intake/WP23_3"


def sha256(path):
    value = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            value.update(chunk)
    return value.hexdigest()


def extract_members(archive_path, destination, suffixes):
    extracted = []
    with zipfile.ZipFile(archive_path) as archive:
        for name in archive.namelist():
            normalized = name.replace("\\", "/")
            if any(normalized.endswith(suffix) for suffix in suffixes):
                archive.extract(name, destination)
                extracted.append(destination / name)
    missing = [suffix for suffix in suffixes if not any(
        str(path).replace("\\", "/").endswith(suffix) for path in extracted)]
    if missing:
        raise RuntimeError(f"Missing expected members in {archive_path.name}: {missing}")
    return extracted


def extract_park(destination):
    nested_name = "3D Voxel Park Pack - Obj File.zip"
    with zipfile.ZipFile(PARK_PACK) as outer:
        payload = outer.read(nested_name)
    wanted_stems = (
        "Playground_Rocking_Penguin_01",
        "Playground_Slide",
        "Playground_Swing_01",
    )
    extracted = []
    with zipfile.ZipFile(io.BytesIO(payload)) as nested:
        for name in nested.namelist():
            if any(Path(name).stem == stem for stem in wanted_stems):
                nested.extract(name, destination)
                extracted.append(destination / name)
    for stem in wanted_stems:
        if not any(path.stem == stem and path.suffix.lower() == ".obj" for path in extracted):
            raise RuntimeError(f"Missing park OBJ candidate: {stem}")
    return extracted


def first_ending(paths, suffix):
    return next(Path(path) for path in paths if str(path).replace("\\", "/").endswith(suffix))


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


def import_animation(source, destination, name, skeleton):
    options = unreal.FbxImportUI()
    options.set_editor_property("automated_import_should_detect_type", False)
    options.set_editor_property("mesh_type_to_import", unreal.FBXImportType.FBXIT_ANIMATION)
    options.set_editor_property("import_mesh", False)
    options.set_editor_property("import_as_skeletal", True)
    options.set_editor_property("import_animations", True)
    options.set_editor_property("import_materials", False)
    options.set_editor_property("import_textures", False)
    options.set_editor_property("skeleton", skeleton)
    task = unreal.AssetImportTask()
    task.filename = source.as_posix()
    task.destination_path = destination
    task.destination_name = name
    task.automated = True
    task.save = True
    task.replace_existing = True
    task.async_ = False
    task.factory = unreal.FbxFactory()
    task.options = options
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
            body_setup = asset.get_editor_property("body_setup")
            rows.append({
                "object_path": asset.get_path_name(),
                "class": "StaticMesh",
                "bounds_min": list(box.min.to_tuple()),
                "bounds_max": list(box.max.to_tuple()),
                "body_setup_present": body_setup is not None,
                "materials": material_paths(asset.get_editor_property("static_materials")),
            })
    return rows


def main():
    report = {
        "kind": "wp23_3_hallowwood_unreal_intake",
        "engine": unreal.SystemLibrary.get_engine_version(),
        "scope": "ignored intake and measurements only; no profile, gameplay, map, travel, fact, Echo, or save acceptance",
        "status": "ERROR",
    }
    try:
        required = (DRAGON_PACK, FOREST_PACK, PARK_PACK, VILLAGERS, WIZARDS)
        missing = [str(path) for path in required if not path.is_file()]
        if missing:
            raise RuntimeError(f"Missing Hallowwood candidate sources: {missing}")

        EXTRACTED.mkdir(parents=True, exist_ok=True)
        with zipfile.ZipFile(DRAGON_PACK) as archive:
            dragon_bytes = archive.read("GLTF/Wooden Dragon.gltf")
        dragon_path = EXTRACTED / "Wooden Dragon.gltf"
        dragon_path.write_bytes(dragon_bytes)
        dragon_json = json.loads(dragon_bytes)

        villager_files = extract_members(VILLAGERS, EXTRACTED / "Villagers", [
            "/FBX/Characters/TVS_VoxelVillage_OldMan.fbx",
            "/Textures/Characters/TVS_VoxelVillage_OldMan_Texture.png",
            "/Animations/Human_Idle_Anim.fbx",
            "/Animations/Human_Walk_Anim.fbx",
        ])
        wizard_files = extract_members(WIZARDS, EXTRACTED / "Wizards", [
            "/FBX/Characters/TVS_VoxelWizards_MasterWizard.fbx",
            "/Textures/Characters/TVS_VoxelWizards_MasterWizard_Texture.png",
            "/Animations/Humans/Human_Idle_Anim.fbx",
            "/Animations/Humans/Human_Walk_Anim.fbx",
        ])
        forest_files = extract_members(FOREST_PACK, EXTRACTED / "Forest", [
            "/Forest/ForestScene.fbx",
            "/Forest/Forest_tex.png",
        ])
        park_files = extract_park(EXTRACTED / "Park")

        asset_subsystem = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
        imports = {
            "grovemaw": import_asset(dragon_path, DEST + "/Grovemaw"),
            "osk_old_man": import_asset(
                first_ending(villager_files, "/TVS_VoxelVillage_OldMan.fbx"),
                DEST + "/Osk/OldMan"),
            "osk_master_wizard": import_asset(
                first_ending(wizard_files, "/TVS_VoxelWizards_MasterWizard.fbx"),
                DEST + "/Osk/MasterWizard"),
            "forest_scene": import_asset(
                first_ending(forest_files, "/ForestScene.fbx"),
                DEST + "/Environment/Forest"),
        }
        old_man_mesh = asset_subsystem.load_asset(
            DEST + "/Osk/OldMan/TVS_VoxelVillage_OldMan.TVS_VoxelVillage_OldMan")
        wizard_mesh = asset_subsystem.load_asset(
            DEST + "/Osk/MasterWizard/TVS_VoxelWizards_MasterWizard.TVS_VoxelWizards_MasterWizard")
        if not isinstance(old_man_mesh, unreal.SkeletalMesh) or not isinstance(wizard_mesh, unreal.SkeletalMesh):
            raise RuntimeError("Osk candidates did not import as skeletal meshes")
        imports["osk_old_man_idle"] = import_animation(
            first_ending(villager_files, "/Animations/Human_Idle_Anim.fbx"),
            DEST + "/Osk/OldMan/Animations", "A_OldMan_Idle",
            old_man_mesh.get_editor_property("skeleton"))
        imports["osk_old_man_walk"] = import_animation(
            first_ending(villager_files, "/Animations/Human_Walk_Anim.fbx"),
            DEST + "/Osk/OldMan/Animations", "A_OldMan_Walk",
            old_man_mesh.get_editor_property("skeleton"))
        imports["osk_master_wizard_idle"] = import_animation(
            first_ending(wizard_files, "/Animations/Humans/Human_Idle_Anim.fbx"),
            DEST + "/Osk/MasterWizard/Animations", "A_MasterWizard_Idle",
            wizard_mesh.get_editor_property("skeleton"))
        imports["osk_master_wizard_walk"] = import_animation(
            first_ending(wizard_files, "/Animations/Humans/Human_Walk_Anim.fbx"),
            DEST + "/Osk/MasterWizard/Animations", "A_MasterWizard_Walk",
            wizard_mesh.get_editor_property("skeleton"))
        for stem in ("Playground_Rocking_Penguin_01", "Playground_Slide", "Playground_Swing_01"):
            source = next(path for path in park_files if path.stem == stem and path.suffix.lower() == ".obj")
            imports["park_" + stem] = import_asset(source, DEST + "/Environment/Park/" + stem)

        dragon_assets = summarize_assets(asset_subsystem, DEST + "/Grovemaw")
        animations = dragon_assets.get("AnimSequence", [])
        skeletal = dragon_assets.get("SkeletalMesh", [])
        required_motion = ("Idle", "Walk", "Flying", "Take_off", "Landing", "Attack")
        motion_hits = {
            name: [asset for asset in animations if name.lower().replace("_", " ") in asset.lower().replace("_", " ")]
            for name in required_motion
        }
        leader = next((name for name in skeletal if name.lower() == "hip-local"), None)
        rig_candidate_ready = bool(
            leader and dragon_assets.get("Skeleton") and len(skeletal) >= 30 and
            len(animations) >= 15 and all(motion_hits.values()))

        osk_assets = summarize_assets(asset_subsystem, DEST + "/Osk")
        environment_assets = summarize_assets(asset_subsystem, DEST + "/Environment")
        osk_candidate_ready = (
            len(osk_assets.get("SkeletalMesh", [])) >= 2 and
            len(osk_assets.get("AnimSequence", [])) >= 4)
        environment_candidate_ready = bool(
            environment_assets.get("StaticMesh") or environment_assets.get("SkeletalMesh"))

        report.update({
            "status": "PASS_INTAKE" if all((
                rig_candidate_ready, osk_candidate_ready, environment_candidate_ready,
            )) else "PARTIAL",
            "sources": {
                "dragon_pack_sha256": sha256(DRAGON_PACK),
                "forest_pack_sha256": sha256(FOREST_PACK),
                "park_pack_sha256": sha256(PARK_PACK),
                "villager_pack_sha256": sha256(VILLAGERS),
                "wizard_pack_sha256": sha256(WIZARDS),
            },
            "grovemaw": {
                "source": "assets and old docs/Voxel+Dragons+Pack+Upload.zip::GLTF/Wooden Dragon.gltf",
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
            "osk_candidates": {
                "assets": osk_assets,
                "candidate_ready": osk_candidate_ready,
                "visual_selection": "NOT_RUN; neither candidate is accepted by this intake",
            },
            "environment_candidates": {
                "assets": environment_assets,
                "candidate_ready": environment_candidate_ready,
                "rendered_scene_fit": "NOT_RUN",
            },
            "unreal_intake": {
                "imports": imports,
                "measured_meshes": measured_meshes(asset_subsystem, DEST),
            },
            "remaining_gates": [
                "Rendered Grovemaw modular assembly and representative animation QA",
                "Rendered Old Man versus Master Wizard Osk comparison and explicit selection",
                "Rendered forest and abandoned-carnival trace scene fit",
                "Distinct supplied fits for Hollow Harvestman, carnival presence, and unfinished puppet",
            ],
        })
    except Exception:
        report["status"] = "ERROR"
        report["error"] = traceback.format_exc()
    finally:
        REPORT_PATH.parent.mkdir(parents=True, exist_ok=True)
        REPORT_PATH.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")

    print(f"WP-23.3 Hallowwood Unreal intake: {report['status']}")
    print(f"Receipt: {REPORT_PATH}")
    if report["status"] == "ERROR":
        raise RuntimeError(report["error"])


main()
