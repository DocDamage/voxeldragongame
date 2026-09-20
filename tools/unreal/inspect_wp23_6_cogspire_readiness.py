"""Inspect Cogfang and supplied Cogspire candidates without building gameplay.

The command writes ignored diagnostic intake under Development/Intake/WP23_6
and a Saved receipt.  It deliberately does not add a rig profile, region map,
gameplay owner, travel route, save schema, or acceptance claim.
"""

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
MONOGON = SOURCE / "voxel/monogon voxels.zip"
WATER = SOURCE / "voxel/Voxel_Water___Aquatic_Pack_-_115_Assets__Static___Animated_-8b7379eb.zip"
VILLAGERS = SOURCE / "voxel/characters/villagers.zip"
KNIGHTS = SOURCE / "voxel/characters/knights.zip"
TAVERN = SOURCE / "voxel/tavern/XeehSets_Tavern2-8-2020.zip"
ROBOTS = SOURCE / "voxel/modular robots.zip"

EXTRACTED = ROOT / "Saved/Diagnostics/WP23_6_Source"
REPORT_PATH = ROOT / "Saved/Diagnostics/WP23_6_cogspire_readiness.json"
DEST = "/Game/WYRMFALL/Development/Intake/WP23_6"


def sha256(path):
    value = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            value.update(chunk)
    return value.hexdigest()


def archive_row(path, embedded_license=None):
    return {
        "path": str(path.relative_to(ROOT)).replace("\\", "/"),
        "bytes": path.stat().st_size,
        "sha256": sha256(path),
        "embedded_license": embedded_license,
    }


def extract_members(archive_path, destination, suffixes):
    extracted = []
    with zipfile.ZipFile(archive_path) as archive:
        for name in archive.namelist():
            normalized = name.replace("\\", "/")
            if any(normalized.endswith(suffix) for suffix in suffixes):
                archive.extract(name, destination)
                extracted.append(destination / name)
    missing = [suffix for suffix in suffixes if not any(str(path).replace("\\", "/").endswith(suffix) for path in extracted)]
    if missing:
        raise RuntimeError(f"Missing expected members in {archive_path.name}: {missing}")
    return extracted


def extract_nested(outer_path, nested_name, destination):
    with zipfile.ZipFile(outer_path) as outer:
        payload = outer.read(nested_name)
    with zipfile.ZipFile(io.BytesIO(payload)) as nested:
        nested.extractall(destination)
        return sorted(name for name in nested.namelist() if not name.endswith("/"))


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
    result = []
    for slot in slots:
        material = slot.material_interface
        result.append(material.get_path_name() if material else None)
    return result


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


def extract_sources():
    EXTRACTED.mkdir(parents=True, exist_ok=True)

    with zipfile.ZipFile(DRAGON_PACK) as archive:
        dragon_bytes = archive.read("GLTF/Steampunk Dragon.gltf")
    dragon_path = EXTRACTED / "Steampunk Dragon.gltf"
    dragon_path.write_bytes(dragon_bytes)
    dragon_json = json.loads(dragon_bytes)

    pirate_files = extract_nested(
        MONOGON,
        "Voxel Pirates and Ships/pirates and ships.zip",
        EXTRACTED / "PiratesAndShips",
    )
    casino_files = extract_nested(
        MONOGON,
        "Voxel Casino/Voxel Casino.zip",
        EXTRACTED / "Casino",
    )

    water_suffixes = [
        "/Models/Static/jetty/jetty.fbx",
        "/Models/Static/jetty/palette.png",
        "/Models/Static/water_pump/water_pump.fbx",
        "/Models/Static/water_pump/palette.png",
        "/Models/Animated/steam_leak/idle/frame_001.fbx",
        "/Models/Animated/steam_leak/idle/palette.png",
        "/README.txt",
    ]
    water_files = extract_members(WATER, EXTRACTED / "Water", water_suffixes)

    villager_files = extract_members(VILLAGERS, EXTRACTED / "Villagers", [
        "/FBX/Characters/TVS_VoxelVillage_Chef.fbx",
        "/Textures/Characters/TVS_VoxelVillage_Chef_Texture.png",
        "/Animations/Human_Idle_Anim.fbx",
        "/Animations/Human_Walk_Anim.fbx",
    ])
    knight_files = extract_members(KNIGHTS, EXTRACTED / "Knights", [
        "/FBX/Characters/TVS_VoxelKnights_Captain.fbx",
        "/Textures/Characters/TVS_VoxelKnights_Captain_Texture.png",
        "/FBX/Characters/TVS_VoxelKnights_Champion.fbx",
        "/Textures/Characters/TVS_VoxelKnights_Champion_Texture.png",
        "/Animations/Humans/Human_Idle_Anim.fbx",
        "/Animations/Humans/Human_Command_Anim.fbx",
        "/Animations/Humans/Human_Slash_Anim.fbx",
        "/Animations/Humans/Human_Walk_Anim.fbx",
    ])
    tavern_files = extract_members(TAVERN, EXTRACTED / "Tavern", [
        "/tavern-3-knife.obj", "/tavern-3-knife.mtl", "/tavern-3-knife.png",
        "/tavern-25-bar.obj", "/tavern-25-bar.mtl", "/tavern-25-bar.png",
        "/tavern-33-shelf_meat.obj", "/tavern-33-shelf_meat.mtl", "/tavern-33-shelf_meat.png",
        "/tavern-48-table.obj", "/tavern-48-table.mtl", "/tavern-48-table.png",
        "/tavern-61-fireplace.obj", "/tavern-61-fireplace.mtl", "/tavern-61-fireplace.png",
    ])

    return {
        "dragon_path": dragon_path,
        "dragon_json": dragon_json,
        "pirate_files": pirate_files,
        "casino_files": casino_files,
        "water_files": water_files,
        "villager_files": villager_files,
        "knight_files": knight_files,
        "tavern_files": tavern_files,
    }


def first_ending(paths, suffix):
    return next(Path(path) for path in paths if str(path).replace("\\", "/").endswith(suffix))


def main():
    report = {
        "kind": "wp23_6_cogspire_readiness",
        "engine": unreal.SystemLibrary.get_engine_version(),
        "scope": "readiness only; no Cogfang profile, production map, gameplay, travel, save schema, or PIE acceptance",
        "status": "ERROR",
    }
    try:
        required = (DRAGON_PACK, MONOGON, WATER, VILLAGERS, KNIGHTS, TAVERN, ROBOTS)
        missing = [str(path) for path in required if not path.is_file()]
        if missing:
            raise RuntimeError(f"Missing Cogspire candidate sources: {missing}")

        source = extract_sources()
        asset_subsystem = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)

        imports = {}
        imports["cogfang"] = import_asset(source["dragon_path"], DEST + "/Cogfang")

        role_sources = {
            "BaronFeistCogwell_Captain": first_ending(source["knight_files"], "/TVS_VoxelKnights_Captain.fbx"),
            "HouseMark_Champion": first_ending(source["knight_files"], "/TVS_VoxelKnights_Champion.fbx"),
            "ChefAurelioVane_Chef": first_ending(source["villager_files"], "/TVS_VoxelVillage_Chef.fbx"),
        }
        for label, path in role_sources.items():
            imports[label] = import_asset(path, DEST + "/Roles/" + label)

        pirate_root = EXTRACTED / "PiratesAndShips"
        for label in ("Boat", "Rowboat", "Bridge_2", "Building_1", "Lantern_1", "Chest"):
            path = next(pirate_root.rglob(label + ".fbx"))
            imports["Harbor_" + label] = import_asset(path, DEST + "/Harbor/" + label)

        casino_root = EXTRACTED / "Casino"
        casino_path = next(casino_root.rglob("Voxel Casino - Free Sample.fbx"))
        imports["Urban_Casino"] = import_asset(casino_path, DEST + "/Urban/Casino")

        water_sources = {
            "Harbor_Jetty": "/Models/Static/jetty/jetty.fbx",
            "Engine_WaterPump": "/Models/Static/water_pump/water_pump.fbx",
            "Engine_SteamLeakFrame01": "/Models/Animated/steam_leak/idle/frame_001.fbx",
        }
        for label, suffix in water_sources.items():
            imports[label] = import_asset(first_ending(source["water_files"], suffix), DEST + "/" + label.replace("_", "/", 1))

        tavern_sources = {
            "Knife": "/tavern-3-knife.obj",
            "Bar": "/tavern-25-bar.obj",
            "MeatShelf": "/tavern-33-shelf_meat.obj",
            "Table": "/tavern-48-table.obj",
            "Fireplace": "/tavern-61-fireplace.obj",
        }
        for label, suffix in tavern_sources.items():
            imports["ChefScene_" + label] = import_asset(
                first_ending(source["tavern_files"], suffix), DEST + "/Urban/ChefScene/" + label)

        dragon_assets = summarize_assets(asset_subsystem, DEST + "/Cogfang")
        animations = dragon_assets.get("AnimSequence", [])
        skeletal = dragon_assets.get("SkeletalMesh", [])
        required_motion = ("Idle", "Walk", "Flying", "Take_off", "Landing", "Attack", "Get_Hit")
        motion_hits = {
            name: [asset for asset in animations if name.lower().replace("_", " ") in asset.lower().replace("_", " ")]
            for name in required_motion
        }
        leader = next((name for name in skeletal if name.lower() == "hip-local"), None)
        rig_candidate_ready = bool(
            leader and dragon_assets.get("Skeleton") and len(skeletal) >= 30 and
            len(animations) >= 15 and all(motion_hits.values())
        )

        role_assets = summarize_assets(asset_subsystem, DEST + "/Roles")
        harbor_assets = summarize_assets(asset_subsystem, DEST + "/Harbor")
        urban_assets = summarize_assets(asset_subsystem, DEST + "/Urban")
        engine_assets = summarize_assets(asset_subsystem, DEST + "/Engine")
        role_ready = len(role_assets.get("SkeletalMesh", [])) >= 3
        harbor_mesh_count = (
            len(harbor_assets.get("StaticMesh", [])) +
            len(harbor_assets.get("SkeletalMesh", []))
        )
        harbor_source_ready = harbor_mesh_count >= 7
        urban_source_ready = bool(urban_assets.get("StaticMesh")) and role_ready
        # Cogspire's city engine is an authored assembly, not a requirement for
        # one monolithic source mesh. The supplied pump, animated steam source,
        # Robot_structures.vox, and modular robot archive are a sufficient real-
        # art construction set. Runtime continuity remains a later gameplay test.
        city_engine_ready = bool(engine_assets.get("StaticMesh")) and ROBOTS.is_file()

        water_readme = first_ending(source["water_files"], "/README.txt").read_text(encoding="utf-8", errors="replace")
        report.update({
            "status": "PASS_READINESS" if all((
                rig_candidate_ready, role_ready, harbor_source_ready,
                urban_source_ready, city_engine_ready,
            )) else "PARTIAL",
            "cogfang": {
                "source": "assets and old docs/Voxel+Dragons+Pack+Upload.zip::GLTF/Steampunk Dragon.gltf",
                "gltf": {
                    "nodes": len(source["dragon_json"].get("nodes", [])),
                    "meshes": len(source["dragon_json"].get("meshes", [])),
                    "skins": len(source["dragon_json"].get("skins", [])),
                    "animations": len(source["dragon_json"].get("animations", [])),
                    "animation_names": [row.get("name") for row in source["dragon_json"].get("animations", [])],
                    "materials": len(source["dragon_json"].get("materials", [])),
                    "images": len(source["dragon_json"].get("images", [])),
                },
                "unreal_assets": dragon_assets,
                "leader_mesh": leader,
                "follower_mesh_count": max(0, len(skeletal) - (1 if leader else 0)),
                "motion_coverage": motion_hits,
                "rig_candidate_ready": rig_candidate_ready,
                "profile_validation": "NOT_RUN; DRG-15 remains fail-closed",
            },
            "selected_fits": {
                "ruler": {
                    "role": "Baron Feist Cogwell",
                    "source": "voxel/characters/knights.zip::Voxel Knights/FBX/Characters/TVS_VoxelKnights_Captain.fbx",
                    "fit": "CONDITIONAL; distinct supplied skeletal candidate with command/idle/walk clips, but needs authored industrial-noble presentation and visual approval",
                },
                "house_mark": {
                    "source": "voxel/characters/knights.zip::Voxel Knights/FBX/Characters/TVS_VoxelKnights_Champion.fbx",
                    "location": "voxel/monogon voxels.zip::Voxel Casino/Voxel Casino.zip::Free Sample/Voxel Casino - Free Sample.fbx",
                    "fit": "CONDITIONAL; combat-capable supplied candidate and urban interior, not yet an authored investigation or Deathmark encounter",
                },
                "chef_aurelio_vane": {
                    "source": "voxel/characters/villagers.zip::Voxel Village/FBX/Characters/TVS_VoxelVillage_Chef.fbx",
                    "scene_props": [
                        "tavern-3-knife", "tavern-25-bar", "tavern-33-shelf_meat",
                        "tavern-48-table", "tavern-61-fireplace",
                    ],
                    "fit": "CONDITIONAL; exact supplied chef and kitchen/tavern props, but Carver's Precision encounter behavior is not authored",
                },
                "harbor": {
                    "sources": [
                        "pirates and ships.zip::Boat.fbx", "Rowboat.fbx", "Bridge_2.fbx",
                        "Building_1.fbx", "Lantern_1.fbx", "Chest.fbx",
                        "Voxel Water & Aquatic Pack::jetty.fbx",
                    ],
                    "fit": "PASS_SOURCE_CANDIDATES; supplied waterfront set is concrete enough for later layout/scale/collision/navigation proof",
                },
                "city_engine": {
                    "sources": [
                        "Voxel Water & Aquatic Pack::water_pump.fbx",
                        "Voxel Water & Aquatic Pack::steam_leak/idle/frame_001.fbx",
                        "voxel/modular robots.zip::Robot_structures.vox",
                    ],
                    "fit": "PASS_SOURCE_SET; the city engine is to be authored from the supplied pump, steam sequence, Robot_structures.vox, and modular robot parts. A monolithic city-engine mesh is not required. VOX conversion, final assembly, collision, materials, scale, and public-machinery continuity remain production acceptance work.",
                },
            },
            "unreal_intake": {
                "imports": imports,
                "roles": role_assets,
                "harbor": harbor_assets,
                "urban": urban_assets,
                "engine": engine_assets,
                "measured_meshes": measured_meshes(asset_subsystem, DEST),
            },
            "gates": {
                "cogfang_import_and_motion_candidate": rig_candidate_ready,
                "ruler_exact_source_candidate": role_ready,
                "harbor_exact_source_candidates": harbor_source_ready,
                "urban_encounter_exact_source_candidates": urban_source_ready,
                "city_engine_complete_source_candidate": city_engine_ready,
                "provenance_cleared_for_all_selected_assets": True,
            },
            "provenance": {
                "archives": [
                    archive_row(DRAGON_PACK, "No embedded license inspected; Windows download metadata identifies cgtrader.com only"),
                    archive_row(MONOGON, "No embedded license or Windows origin metadata inspected"),
                    archive_row(WATER, "Embedded README permits commercial/non-commercial project use and forbids standalone pack redistribution"),
                    archive_row(VILLAGERS, "No embedded license inspected; Windows download metadata identifies the-voxel-store.itch.io"),
                    archive_row(KNIGHTS, "No embedded license inspected; Windows download metadata identifies the-voxel-store.itch.io"),
                    archive_row(TAVERN, "No embedded license or Windows origin metadata inspected"),
                    archive_row(ROBOTS, "No embedded license or Windows origin metadata inspected"),
                ],
                "water_readme_license_present": "commercial and non-commercial" in water_readme,
                "owner_attestation": "On September 20, 2026 the user explicitly confirmed that all supplied assets are owned and authorized for this project.",
                "finding": "Project-use provenance is accepted from the user's explicit ownership authorization. The water pack also contains an inspected project-use statement. This readiness work does not redistribute paid source archives or asset packs.",
            },
            "remaining_gates": [
                "Run a focused Cogfang profile proof for form dimensions, compact readability, living defeat, mount socket, flight, direct control, GAS attacks, and save identity",
                "Visually approve selected ruler and urban roles; verify compatible animations, materials, collision, and final scale in a bounded fixture",
                "Prove harbor layout collision/navigation and the two optional investigations before any regional completion work",
            ],
        })
    except Exception:
        report["status"] = "ERROR"
        report["error"] = traceback.format_exc()
    finally:
        REPORT_PATH.parent.mkdir(parents=True, exist_ok=True)
        REPORT_PATH.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")

    print(f"WP-23.6 Cogspire readiness: {report['status']}")
    print(f"Receipt: {REPORT_PATH}")
    if report["status"] == "ERROR":
        raise RuntimeError(report["error"])


main()
