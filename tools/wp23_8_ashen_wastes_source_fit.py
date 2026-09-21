"""Inventory supplied Ashen Wastes candidates without claiming Unreal acceptance."""

import hashlib
import json
import zipfile
from pathlib import Path


ROOT = Path(__file__).resolve().parent.parent
SOURCE = ROOT / "assets and old docs"
REPORT = ROOT / "Saved/Diagnostics/WP23_8_ashen_wastes_source_fit.json"

DRAGON_PACK = SOURCE / "Voxel+Dragons+Pack+Upload.zip"
MONOGON_PACK = SOURCE / "voxel/monogon voxels.zip"
SAND_PACK = SOURCE / "voxel/Voxel_Sand___Beach_Pack_-_35_Assets__Static___Animated_-b0ef1ece.zip"
WATER_PACK = SOURCE / "voxel/Voxel_Water___Aquatic_Pack_-_115_Assets__Static___Animated_-8b7379eb.zip"


def relative(path: Path) -> str:
    return path.relative_to(ROOT).as_posix()


def digest(path: Path) -> str:
    value = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            value.update(chunk)
    return value.hexdigest()


def member_row(archive_path: Path, member: str) -> dict:
    with zipfile.ZipFile(archive_path) as archive:
        info = archive.getinfo(member)
    return {
        "archive": relative(archive_path),
        "archive_sha256": digest(archive_path),
        "member": member,
        "bytes": info.file_size,
    }


def members_under(archive_path: Path, prefix: str, suffix: str) -> list[str]:
    with zipfile.ZipFile(archive_path) as archive:
        return sorted(
            name.replace("\\", "/")
            for name in archive.namelist()
            if name.replace("\\", "/").startswith(prefix)
            and name.casefold().endswith(suffix.casefold())
        )


def dragon_row() -> dict:
    member = "GLTF/Zombie Dragon.gltf"
    with zipfile.ZipFile(DRAGON_PACK) as archive:
        document = json.loads(archive.read(member))
    animations = sorted(item.get("name", "") for item in document.get("animations", []))
    required = ["Idle 01", "Flying 01", "Take off 01", "Landing 01", "Attack 01", "Walk 01"]
    missing = [name for name in required if name not in animations]
    return {
        "pack": relative(DRAGON_PACK),
        "pack_sha256": digest(DRAGON_PACK),
        "member": member,
        "generator": document.get("asset", {}).get("generator"),
        "node_count": len(document.get("nodes", [])),
        "mesh_count": len(document.get("meshes", [])),
        "material_count": len(document.get("materials", [])),
        "animation_count": len(animations),
        "animation_names": animations,
        "missing_required_animations": missing,
        "status": "PASS_FOR_UNREAL_INTAKE" if not missing else "FAIL",
    }


def main() -> None:
    required_files = [DRAGON_PACK, MONOGON_PACK, SAND_PACK, WATER_PACK]
    missing_files = [str(path) for path in required_files if not path.is_file()]
    if missing_files:
        raise SystemExit(f"Missing Ashen Wastes candidate sources: {missing_files}")

    bunker_models = members_under(MONOGON_PACK, "Voxel Nuclear Bunker/Free Sample/", ".obj")
    apocalypse_models = members_under(MONOGON_PACK, "Voxel Zombie Apocalypse/Free Sample/", ".fbx")
    ruler_members = [
        name for name in apocalypse_models
        if Path(name).name in {"Character_Hero.fbx", "Character_Zombie.fbx"}
    ]
    laboratory_terms = ("laboratory", "/lab/", "scientist", "doctor", "medical")
    with zipfile.ZipFile(MONOGON_PACK) as archive:
        laboratory_members = sorted(
            name.replace("\\", "/")
            for name in archive.namelist()
            if any(term in name.replace("\\", "/").casefold() for term in laboratory_terms)
        )

    desert_member = next(
        name for name in members_under(SAND_PACK, "", ".fbx")
        if name.casefold().endswith("desert_ruin/desert_ruin.fbx")
    )
    toxic_member = next(
        name for name in members_under(WATER_PACK, "", ".fbx")
        if name.casefold().endswith("toxic_puddle/toxic_puddle.fbx")
    )

    dragon = dragon_row()
    report = {
        "kind": "wp23_8_ashen_wastes_source_fit",
        "status": "PASS_WITH_LABORATORY_AND_CAST_BLOCKERS",
        "scope": "Portable source-fit evidence only; no Unreal import, rig profile, or gameplay claim",
        "rotwing": dragon,
        "bunker": {
            "pack": relative(MONOGON_PACK),
            "pack_sha256": digest(MONOGON_PACK),
            "model_count": len(bunker_models),
            "representative_models": bunker_models[:12],
            "status": "READY_FOR_FOCUSED_UNREAL_INTAKE" if bunker_models else "FAIL",
        },
        "ashen_environment": {
            "desert_ruin": member_row(SAND_PACK, desert_member),
            "toxic_puddle": member_row(WATER_PACK, toxic_member),
            "status": "CANDIDATES_REQUIRE_RENDERED_VISUAL_QA",
        },
        "rotking_candidates": {
            "members": ruler_members,
            "status": "CANDIDATES_REQUIRE_RENDERED_VISUAL_QA" if len(ruler_members) == 2 else "FAIL",
            "boundary": "Names and source presence do not establish a ruler presentation",
        },
        "laboratory": {
            "matching_members": laboratory_members,
            "status": "BLOCKED_NO_EXACT_SUPPLIED_LABORATORY_SOURCE" if not laboratory_members else "CANDIDATE",
        },
        "optional_echo_presentations": {
            "doctor_hollowmend": "BLOCKED_ON_DISTINCT_NON_FRANCHISE_PRESENTATION_FIT",
            "bunker_false_rescuer": "BLOCKED_ON_DISTINCT_NON_FRANCHISE_PRESENTATION_FIT",
        },
        "next_bounded_task": (
            "Import and measure Zombie Dragon as Rotwing plus the two Rotking candidates, "
            "representative bunker props, desert ruin, and toxic puddle in an ignored intake folder"
        ),
    }
    if dragon["status"] != "PASS_FOR_UNREAL_INTAKE" or not bunker_models:
        report["status"] = "FAIL"
    REPORT.parent.mkdir(parents=True, exist_ok=True)
    REPORT.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(f"WP-23.8 Ashen Wastes source fit: {report['status']}")
    print(f"Rotwing source: {dragon['mesh_count']} meshes, {dragon['animation_count']} animations")
    print(f"Bunker models: {len(bunker_models)}")
    print(f"Receipt: {REPORT}")


if __name__ == "__main__":
    main()
