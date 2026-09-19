"""Inspect Nyxaroth and Gloaming candidate content without building the region."""

import io
import json
import zipfile
from pathlib import Path
import unreal

ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
SOURCE = ROOT / "assets and old docs"
DRAGON_PACK = SOURCE / "Voxel+Dragons+Pack+Upload.zip"
CATHEDRAL = SOURCE / "voxel/characters/Voxel Cathedral.zip"
CEMETERY = SOURCE / "voxel/cemetary and church voxel set.zip"
HORROR = SOURCE / "voxel/characters/horror characters.fbx"
EXTRACTED = ROOT / "Saved/Diagnostics/WP23_5_Source/Dark Dragon.gltf"
DEST_DRAGON = "/Game/WYRMFALL/Development/Intake/WP23_5/Nyxaroth"
DEST_HORROR = "/Game/WYRMFALL/Development/Intake/WP23_5/Horror"
DEST_GLOAMING = "/Game/WYRMFALL/Development/Intake/WP23_5/Gloaming"
REPORT = ROOT / "Saved/Diagnostics/WP23_5_gloaming_readiness.json"


def names_with_suffix(names, suffix):
    return sorted(name for name in names if name.lower().endswith(suffix))


def inspect_archives():
    with zipfile.ZipFile(CATHEDRAL) as archive:
        cathedral_names = archive.namelist()
    with zipfile.ZipFile(CEMETERY) as outer:
        nested_name = outer.namelist()[0]
        with zipfile.ZipFile(io.BytesIO(outer.read(nested_name))) as nested:
            cemetery_names = nested.namelist()
    return {
        "cathedral": {
            "source": str(CATHEDRAL),
            "fbx_characters": names_with_suffix(
                [name for name in cathedral_names if "/FBX/Characters/" in name], ".fbx"),
            "fbx_environment": names_with_suffix(
                [name for name in cathedral_names if "/FBX/Environment/" in name], ".fbx"),
            "animations": names_with_suffix(cathedral_names, ".fbx")[:8],
            "texture_count": len(names_with_suffix(cathedral_names, ".png")),
        },
        "cemetery": {
            "source": str(CEMETERY),
            "nested_archive": nested_name,
            "entry_count": len(cemetery_names),
            "fbx": names_with_suffix(cemetery_names, ".fbx"),
            "obj_count": len(names_with_suffix(cemetery_names, ".obj")),
            "texture_count": len(names_with_suffix(cemetery_names, ".png")),
        },
    }


def import_asset(filename, destination):
    task = unreal.AssetImportTask()
    task.filename = str(filename.as_posix())
    task.destination_path = destination
    task.automated = True
    task.save = True
    task.replace_existing = True
    task.async_ = False
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])


def summarize_assets(asset_sub, destination):
    summary = {}
    paths = asset_sub.list_assets(destination, recursive=True)
    for path in paths:
        asset = asset_sub.load_asset(path)
        if not asset:
            continue
        class_name = asset.get_class().get_name()
        summary.setdefault(class_name, []).append(asset.get_name())
    return {key: sorted(value) for key, value in sorted(summary.items())}


def main():
    required = (DRAGON_PACK, CATHEDRAL, CEMETERY, HORROR)
    missing = [str(path) for path in required if not path.is_file()]
    if missing:
        raise RuntimeError(f"Missing Gloaming candidate sources: {missing}")

    with zipfile.ZipFile(DRAGON_PACK) as archive:
        dragon_bytes = archive.read("GLTF/Dark Dragon.gltf")
    dragon_json = json.loads(dragon_bytes)
    EXTRACTED.parent.mkdir(parents=True, exist_ok=True)
    EXTRACTED.write_bytes(dragon_bytes)

    cathedral_root = EXTRACTED.parent / "Cathedral"
    with zipfile.ZipFile(CATHEDRAL) as archive:
        archive.extractall(cathedral_root)

    asset_sub = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
    if len(asset_sub.list_assets(DEST_DRAGON, recursive=True)) < 25:
        import_asset(EXTRACTED, DEST_DRAGON)
    if not asset_sub.list_assets(DEST_HORROR, recursive=True):
        import_asset(HORROR, DEST_HORROR)
    if not asset_sub.list_assets(DEST_GLOAMING, recursive=True):
        cathedral_files = cathedral_root / "Voxel Cathedral/FBX"
        selected = [
            cathedral_files / "Characters/TVS_VoxelCathedral_Crusader.fbx",
            cathedral_files / "Characters/TVS_VoxelCathedral_Priest.fbx",
            cathedral_files / "Characters/TVS_VoxelCathedral_Nun.fbx",
            cathedral_files / "Environment/TVS_VoxelCathedral_Cathedral.fbx",
            cathedral_files / "Environment/TVS_VoxelCathedral_CrossGrave.fbx",
            cathedral_files / "Environment/TVS_VoxelCathedral_Grave.fbx",
            cathedral_files / "Environment/TVS_VoxelCathedral_Statue.fbx",
            cathedral_files / "Environment/TVS_VoxelCathedral_Tree.fbx",
        ]
        for source in selected:
            import_asset(source, DEST_GLOAMING)

    dragon_assets = summarize_assets(asset_sub, DEST_DRAGON)
    horror_assets = summarize_assets(asset_sub, DEST_HORROR)
    gloaming_assets = summarize_assets(asset_sub, DEST_GLOAMING)
    skeletal = dragon_assets.get("SkeletalMesh", [])
    animations = dragon_assets.get("AnimSequence", [])
    materials = dragon_assets.get("Material", []) + dragon_assets.get("MaterialInstanceConstant", [])
    has_leader = "Hip-Local" in skeletal
    required_motion = ("Idle", "Walk", "Flying", "Take_off", "Landing", "Attack")
    motion_hits = {needle: [name for name in animations if needle.lower() in name.lower()]
                   for needle in required_motion}
    rig_import_ready = bool(
        has_leader and dragon_assets.get("Skeleton") and len(skeletal) >= 25 and
        len(animations) >= 10 and all(motion_hits.values()))

    archive_summary = inspect_archives()
    horror_static = horror_assets.get("StaticMesh", [])
    known_horror_names = sorted({
        candidate for candidate in (
            "Carrie", "PyramidHead", "MicahelMeyers", "AnnieWilkes", "Leatherface",
            "Werewolf", "Chucky", "Alien", "Hellraiser", "Sadoko", "Predator",
            "Ghostface", "GradyDaughter", "It", "Hannibal", "TheMummy", "Dracula",
            "Frankenstein", "Freddy", "Jason")
        if any(candidate.lower() in name.lower() for name in horror_static)
    })
    production_aliases = {
        "Alien": "Ail-Yen",
        "AnnieWilkes": "Annie Wails",
        "Carrie": "Scarrie",
        "Chucky": "Chuckles",
        "Dracula": "Count Dripula",
        "Frankenstein": "Frank N. Shrine",
        "Freddy": "Dready Freddie",
        "Ghostface": "Roastface",
        "GradyDaughter": "The Gravy Daughters",
        "Hannibal": "Canniball",
        "Hellraiser": "Bellraiser",
        "It": "Knit",
        "Jason": "Machete Mason",
        "Leatherface": "Pleatherface",
        "MicahelMeyers": "Michael Mire",
        "Predator": "Dreadator",
        "PyramidHead": "Pyre-Midhead",
        "Sadoko": "Sad Echo",
        "TheMummy": "Mum's the Wyrd",
        "Werewolf": "Wherewolf",
    }
    content_import_ready = bool(
        len(gloaming_assets.get("StaticMesh", [])) + len(gloaming_assets.get("SkeletalMesh", [])) >= 8)
    horror_import_ready = len(horror_static) >= 300
    report = {
        "kind": "wp23_5_gloaming_readiness",
        "engine": unreal.SystemLibrary.get_engine_version(),
        "scope": "readiness only; no Nyxaroth rig profile, gameplay, region map, or PIE acceptance",
        "status": "PASS" if rig_import_ready and content_import_ready and horror_import_ready else "PARTIAL",
        "nyxaroth": {
            "source": "assets and old docs/Voxel+Dragons+Pack+Upload.zip::GLTF/Dark Dragon.gltf",
            "gltf": {
                "nodes": len(dragon_json.get("nodes", [])),
                "meshes": len(dragon_json.get("meshes", [])),
                "animations": len(dragon_json.get("animations", [])),
                "materials": len(dragon_json.get("materials", [])),
                "embedded_images": len(dragon_json.get("images", [])),
                "skins": len(dragon_json.get("skins", [])),
            },
            "unreal_assets": dragon_assets,
            "leader_mesh": "Hip-Local" if has_leader else None,
            "follower_mesh_count": max(0, len(skeletal) - (1 if has_leader else 0)),
            "motion_coverage": motion_hits,
            "rig_import_ready": rig_import_ready,
            "profile_validated": False,
        },
        "gloaming_content": archive_summary,
        "selected_cathedral_import": gloaming_assets,
        "horror_fbx": {
            "source": str(HORROR),
            "bytes": HORROR.stat().st_size,
            "asset_class_counts": {key: len(value) for key, value in horror_assets.items()},
            "inferred_source_identities": known_horror_names,
            "production_aliases": {
                source_name: production_aliases[source_name]
                for source_name in known_horror_names
            },
            "required_content": True,
            "import_ready": horror_import_ready,
            "production_fit": "CONDITIONAL",
            "finding": "Imports as hundreds of separate static body parts. Retain as required horror content under original WYRMFALL pun-name aliases; prove authored assembly, animation strategy, collision, scale, materials, and provenance clearance before production acceptance.",
        },
        "selected_fits": {
            "region_environment": "Voxel Cathedral cathedral/tree/grave/statue set plus cemetery/church modular set",
            "ashgrave": "TVS_VoxelCathedral_Crusader",
            "count_malvaine": "TVS_VoxelCathedral_Priest",
            "hollow_twins": "two authored instances of TVS_VoxelCathedral_Nun",
            "horror_roster": "horror characters.fbx (required; modular static-part assembly and production proof pending)",
        },
        "remaining_gates": [
            "Implement and compile a fail-closed Nyxaroth FWyrmDragonRigProfile",
            "Prove Companion/TrueForm dimensions, Heartfold clearance, mount, flight, combat, direct control, and save in real PIE",
            "Visually review scale/materials for the selected cathedral character and environment subset",
            "Author and prove the required horror-character assemblies, animation strategy, collision, scale, materials, and provenance clearance",
            "Prove Ashgrave, Count Malvaine, and Hollow Twins as distinct authored actors in the eventual regional slice",
        ],
    }
    REPORT.parent.mkdir(parents=True, exist_ok=True)
    REPORT.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(f"WP-23.5 Gloaming readiness: {report['status']}")
    print(f"Nyxaroth import: {len(skeletal)} skeletal meshes, {len(animations)} animations, {len(materials)} materials")
    print(f"Receipt: {REPORT}")
    if report["status"] != "PASS":
        raise RuntimeError("WP-23.5 readiness did not meet the bounded import/content gate")


main()
