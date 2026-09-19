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
        "dragon_validated": False,
        "candidates": [
            "voxel/Low_poly_voxel_forest-c8c1e081.zip",
            "voxel/trees.zip",
            "voxel/characters/voxel_monsters.zip",
        ],
        "readiness": "BLOCKED_ON_RIG_AND_CONTENT_FIT",
        "blockers": ["Grovemaw rig/profile/animation proof", "ruler and three horror encounter fits"],
    },
    {
        "child": "WP-23.4", "region": "Frosthold", "dragon": "White Dragon",
        "dragon_validated": False,
        "candidates": ["voxel/characters/knights.zip"],
        "readiness": "BLOCKED_ON_RIG_AND_REGION_ART",
        "blockers": ["Frostmane rig/profile/animation proof", "snow/ice region and ruler content"],
    },
    {
        "child": "WP-23.5", "region": "Gloaming Marches", "dragon": "Dark Dragon",
        "dragon_validated": False,
        "candidates": [
            "voxel/characters/Voxel Cathedral.zip",
            "voxel/cemetary and church voxel set.zip",
            "voxel/characters/horror characters.fbx",
        ],
        "readiness": "BLOCKED_ON_RIG_AND_CONTENT_FIT",
        "blockers": ["Nyxaroth rig/profile/animation proof", "Ashgrave and optional-horror character fit"],
    },
    {
        "child": "WP-23.6", "region": "Cogspire Harbor", "dragon": "Steampunk Dragon",
        "dragon_validated": False,
        "candidates": [
            "voxel/medievil buildings.zip",
            "voxel/modular robots.zip",
            "voxel/Voxel_Water___Aquatic_Pack_-_115_Assets__Static___Animated_-8b7379eb.zip",
        ],
        "readiness": "BLOCKED_ON_RIG_AND_CONTENT_FIT",
        "blockers": ["Cogfang rig/profile/animation proof", "harbor/city/ruler content fit"],
    },
    {
        "child": "WP-23.7", "region": "Cinderreach", "dragon": "Lava Dragon",
        "dragon_validated": False,
        "candidates": ["voxel/hexagon set.zip", "voxel/characters/knights.zip"],
        "readiness": "BLOCKED_ON_RIG_AND_REGION_ART",
        "blockers": ["Pyraxis rig/profile/animation proof", "volcanic trial/forge/ruler content"],
    },
    {
        "child": "WP-23.8", "region": "Ashen Wastes", "dragon": "Zombie Dragon",
        "dragon_validated": False,
        "candidates": [
            "voxel/cemetary and church voxel set.zip",
            "voxel/characters/horror characters.fbx",
            "voxel/characters/voxel_monsters.zip",
        ],
        "readiness": "BLOCKED_ON_RIG_AND_CONTENT_FIT",
        "blockers": ["Rotwing rig/profile/animation proof", "Rotking/laboratory/bunker content fit"],
    },
    {
        "child": "WP-23.9", "region": "Bonelands", "dragon": "Skull Dragon",
        "dragon_validated": False,
        "candidates": [
            "voxel/cemetary and church voxel set.zip",
            "voxel/characters/horror characters.fbx",
            "voxel/characters/Voxel Cathedral.zip",
        ],
        "readiness": "BLOCKED_ON_RIG_AND_CONTENT_FIT",
        "blockers": ["Ossuroth rig/profile/animation proof", "Kael/tomb/guardian content fit"],
    },
    {
        "child": "WP-23.10", "region": "Zenith Spire", "dragon": "Mecha Dragon",
        "dragon_validated": False,
        "candidates": ["voxel/modular robots.zip", "voxel/tiny cars.zip"],
        "readiness": "BLOCKED_ON_RIG_AND_COLONY_ART",
        "blockers": ["Mecha Dragon rig/profile/animation proof", "playable city/colony and ruler content"],
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
        "validated_dragon_ids": ["Verdance", "Jadefang"],
        "regions": REGIONS,
        "recommended_order": [
            "WP-23.2", "WP-23.1", "WP-23.5", "WP-23.8", "WP-23.9",
            "WP-23.3", "WP-23.6", "WP-23.4", "WP-23.7", "WP-23.10",
        ],
        "first_eligible_child": "WP-23.2",
    }
    REPORT.parent.mkdir(parents=True, exist_ok=True)
    REPORT.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(f"WP-23.0 readiness inventory: {report['status']}")
    print(f"Receipt: {REPORT}")


if __name__ == "__main__":
    main()
