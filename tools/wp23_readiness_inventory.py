"""Create the bounded WP-23.0 real-content readiness receipt."""

import hashlib
import json
import zipfile
from pathlib import Path


ROOT = Path(__file__).resolve().parent.parent
SOURCE = ROOT / "assets and old docs"
DRAGON_PACK = SOURCE / "Voxel+Dragons+Pack+Upload.zip"
REPORT = ROOT / "Saved/Diagnostics/WP23_readiness_inventory.json"

REGIONS = [
    {
        "child": "WP-23.1", "region": "Verdant Reach", "dragon": "Green Dragon",
        "dragon_validated": True,
        "candidates": [
            "voxel/Low_poly_voxel_forest-c8c1e081.zip",
            "voxel/characters/rangers.zip",
            "voxel/trees.zip",
        ],
        "readiness": "READY_FOR_PACKET",
        "blockers": [],
    },
    {
        "child": "WP-23.2", "region": "Jade Peaks", "dragon": "Chinese Dragon",
        "dragon_validated": True,
        "candidates": ["voxel/characters/palace.zip"],
        "readiness": "READY_FOR_PACKET",
        "blockers": [],
    },
    {
        "child": "WP-23.3", "region": "Hallowwood", "dragon": "Wooden Dragon",
        "dragon_validated": True,
        "candidates": [
            "voxel/Low_poly_voxel_forest-c8c1e081.zip",
            "voxel/trees.zip",
            "voxel/playground park.zip",
            "voxel/characters/villagers.zip",
            "voxel/characters/wizards.zip",
            "voxel/characters/voxel_monsters.zip",
        ],
        "readiness": "PROFILE_COMPLETE_REGION_BLOCKED",
        "blockers": [
            "exact abandoned traveling-carnival content fit",
            "Hollow Harvestman, carnival presence, and unfinished-puppet presentation fits",
        ],
    },
    {
        "child": "WP-23.4", "region": "Frosthold", "dragon": "White Dragon",
        "dragon_validated": True,
        "candidates": [
            "voxel/characters/knights.zip",
            "voxel/hexagon set.zip",
            "voxel/Voxel_Water___Aquatic_Pack_-_115_Assets__Static___Animated_-8b7379eb.zip",
            "voxel/Voxel_Grass___Mushrooms_Pack_-_84_Assets__Static___Animated_-ebf10438.zip",
            "voxel/trees.zip",
        ],
        "readiness": "FROSTMANE_PROFILE_COMPLETE_REGION_BLOCKED",
        "blockers": [
            "clean animated King Alaric presentation review",
            "complete rendered storybook winter-castle composition",
        ],
    },
    {
        "child": "WP-23.5", "region": "Gloaming Marches", "dragon": "Dark Dragon",
        "dragon_validated": True,
        "candidates": [
            "voxel/characters/Voxel Cathedral.zip",
            "voxel/cemetary and church voxel set.zip",
            "voxel/characters/horror characters.fbx",
        ],
        "readiness": "COMPLETE",
        "blockers": [],
    },
    {
        "child": "WP-23.6", "region": "Cogspire Harbor", "dragon": "Steampunk Dragon",
        "dragon_validated": True,
        "candidates": [
            "voxel/medievil buildings.zip",
            "voxel/modular robots.zip",
            "voxel/Voxel_Water___Aquatic_Pack_-_115_Assets__Static___Animated_-8b7379eb.zip",
        ],
        "readiness": "COMPLETE",
        "blockers": [],
    },
    {
        "child": "WP-23.7", "region": "Cinderreach", "dragon": "Lava Dragon",
        "dragon_validated": True,
        "candidates": [
            "voxel/characters/knights.zip",
            "voxel/characters/Voxel Cathedral.zip",
            "voxel/Free_Voxel_Dungeon___Execution_Starter_Kit_for_Indie_Games_-3cdb8f01.zip",
            "voxel/Voxel_Sand___Beach_Pack_-_35_Assets__Static___Animated_-b0ef1ece.zip",
            "voxel/Voxel_Water___Aquatic_Pack_-_115_Assets__Static___Animated_-8b7379eb.zip",
        ],
        "readiness": "PYRAXIS_PROFILE_AND_SELECTED_FORGE_OVERSEER_MAGNAROK_BASE_INTAKE_PASS_REGION_BLOCKED",
        "blockers": [
            "readable ritual-arena composition and production-map validation",
            "Magnarok idle/attack/hit/defeat/phase and authored boss-performance coverage",
            "refined rigged animated Flayed Choir overseer with supporting choir staging",
        ],
    },
    {
        "child": "WP-23.8", "region": "Ashen Wastes", "dragon": "Zombie Dragon",
        "dragon_validated": True,
        "candidates": [
            "Voxel+Dragons+Pack+Upload.zip",
            "voxel/monogon voxels.zip",
            "voxel/Voxel_Sand___Beach_Pack_-_35_Assets__Static___Animated_-b0ef1ece.zip",
            "voxel/Voxel_Water___Aquatic_Pack_-_115_Assets__Static___Animated_-8b7379eb.zip",
            "voxel/characters/horror characters.fbx",
        ],
        "readiness": "DRAGON_VALIDATED_REGION_BLOCKED_ON_EXACT_CONTENT",
        "blockers": ["exact authored laboratory", "Doctor Hollowmend presentation", "false-rescuer presentation"],
    },
    {
        "child": "WP-23.9", "region": "Bonelands", "dragon": "Skull Dragon",
        "dragon_validated": True,
        "candidates": [
            "voxel/cemetary and church voxel set.zip",
            "voxel/characters/horror characters.fbx",
            "voxel/characters/Voxel Cathedral.zip",
            "voxel/characters/knights.zip",
            "voxel/characters/villagers.zip",
        ],
        "readiness": "DRAGON_VALIDATED_REGION_BLOCKED_ON_CAST_FIT",
        "blockers": ["wrapped guardian presentation", "distinct Skinning Man presentation"],
    },
    {
        "child": "WP-23.10", "region": "Zenith Spire", "dragon": "Mecha Dragon",
        "dragon_validated": False,
        "candidates": [
            "voxel/monogon voxels.zip",
            "voxel/modular robots/Modular_Robots.zip",
            "voxel/tiny cars.zip",
        ],
        "readiness": "SOURCE_AND_UE_INTAKE_COMPLETE_ASSEMBLY_AND_PRESENTATION_BLOCKED",
        "blockers": [
            "coherent Mecha Dragon modular assembly and DRG-15 profile proof",
            "playable future-city and enclosed colony route content",
            "authored robot population and distinct Overking Vantrix-9 presentation",
            "distinct Broodmother Cipher and infestation-dungeon presentation",
        ],
    },
]


def digest(path: Path) -> str:
    value = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            value.update(chunk)
    return value.hexdigest()


def main() -> None:
    if not DRAGON_PACK.is_file():
        raise SystemExit(f"Missing supplied dragon pack: {DRAGON_PACK}")
    with zipfile.ZipFile(DRAGON_PACK) as archive:
        names = set(archive.namelist())

    for row in REGIONS:
        gltf = f"GLTF/{row['dragon']}.gltf"
        row["dragon_source"] = gltf
        row["dragon_source_present"] = gltf in names
        row["candidate_files"] = []
        for relative in row.pop("candidates"):
            path = SOURCE / relative
            row["candidate_files"].append({
                "path": relative.replace("\\", "/"),
                "present": path.is_file(),
                "bytes": path.stat().st_size if path.is_file() else None,
                "sha256": digest(path) if path.is_file() else None,
            })

    report = {
        "kind": "wp23_readiness_inventory",
        "status": "PASS" if all(row["dragon_source_present"] for row in REGIONS) else "FAIL",
        "scope": "WP-23.0 only; archive presence is not Unreal rig or production acceptance",
        "dragon_pack": {
            "path": str(DRAGON_PACK.relative_to(ROOT)).replace("\\", "/"),
            "bytes": DRAGON_PACK.stat().st_size,
            "sha256": digest(DRAGON_PACK),
        },
        "validated_dragon_ids": ["Verdance", "Jadefang", "Nyxaroth", "Cogfang", "Grovemaw", "Rotwing", "Ossuroth"],
        "regions": REGIONS,
        "recommended_order": [
            "WP-23.1", "WP-23.2", "WP-23.5", "WP-23.6", "WP-23.3",
            "WP-23.8", "WP-23.9", "WP-23.4", "WP-23.7", "WP-23.10",
        ],
        "first_eligible_child": None,
        "next_focused_readiness_child": None,
        "next_focused_readiness_scope": (
            "Archive-only regional readiness is exhausted; diagnose the shared White/Lava/Mecha "
            "Dragon import/transform problem or obtain exact blocked presentation content before gameplay"
        ),
    }
    REPORT.parent.mkdir(parents=True, exist_ok=True)
    REPORT.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(f"WP-23.0 readiness inventory: {report['status']}")
    print(f"Receipt: {REPORT}")


if __name__ == "__main__":
    main()
