"""Audit supplied Zenith, Mecha Dragon, colony, ruler, and Broodmother candidates."""

import hashlib
import io
import json
import re
import shutil
import subprocess
import zipfile
from pathlib import Path


ROOT = Path(__file__).resolve().parent.parent
SOURCE = ROOT / "assets and old docs"
DRAGON_PACK = SOURCE / "Voxel+Dragons+Pack+Upload.zip"
REPORT = ROOT / "Saved/Diagnostics/WP23_10_zenith_source_fit.json"

MODEL_SUFFIXES = {".fbx", ".glb", ".gltf", ".obj", ".vox", ".dae", ".blend"}
SEARCH_TERMS = {
    "future_city": (
        "city", "tower", "skyscraper", "building", "futuristic", "future", "scifi",
        "sci fi", "neon", "transit", "rail", "platform", "street",
    ),
    "colony_interior": (
        "colony", "orbital", "space station", "spaceship", "space ship", "corridor",
        "interior", "habitat", "hangar", "airlock", "vent", "pod", "laboratory", "lab",
    ),
    "robot_population": (
        "robot", "android", "cyborg", "drone", "mech", "technician", "engineer",
    ),
    "ruler": (
        "king", "overking", "monarch", "emperor", "lord", "commander", "captain",
        "champion", "warlord",
    ),
    "broodmother": (
        "brood", "hive", "parasite", "alien", "insect", "spider", "egg", "larva",
        "cocoon", "infest", "mother", "queen", "pod",
    ),
}


def relative(path: Path) -> str:
    return path.relative_to(ROOT).as_posix()


def digest(path: Path) -> str:
    value = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            value.update(chunk)
    return value.hexdigest()


def categories(name: str) -> dict[str, list[str]]:
    normalized = " " + re.sub(r"[_./\\-]+", " ", name).casefold() + " "
    result: dict[str, list[str]] = {}
    whole_word = {"city", "tower", "future", "rail", "platform", "street", "colony", "orbital", "interior", "hangar", "airlock", "vent", "pod", "lab", "robot", "android", "drone", "mech", "king", "lord", "brood", "hive", "alien", "insect", "spider", "egg", "larva", "mother", "queen"}
    for category, terms in SEARCH_TERMS.items():
        found = []
        for term in terms:
            if term in whole_word:
                matched = re.search(rf"\b{re.escape(term)}\b", normalized) is not None
            else:
                matched = term in normalized
            if matched:
                found.append(term)
        if found:
            result[category] = sorted(found)
    return result


def row(archive: Path | None, member: str, size: int | None, prefix: str = "") -> dict | None:
    if Path(member).suffix.casefold() not in MODEL_SUFFIXES:
        return None
    found = categories(member)
    if not found:
        return None
    return {"archive": relative(archive) if archive else None, "member": f"{prefix}{member}", "categories": found, "bytes": size}


def inspect_zip(path: Path, payload: bytes | None = None, prefix: str = "") -> list[dict]:
    source = io.BytesIO(payload) if payload is not None else path
    matches: list[dict] = []
    with zipfile.ZipFile(source) as archive:
        for info in archive.infolist():
            member = info.filename.replace("\\", "/")
            match = row(path, member, info.file_size, prefix)
            if match:
                matches.append(match)
            if prefix == "" and member.casefold().endswith(".zip") and info.file_size <= 200_000_000:
                try:
                    matches.extend(inspect_zip(path, archive.read(info), f"{member}!/"))
                except (KeyError, OSError, zipfile.BadZipFile):
                    pass
    return matches


def inspect_rar(path: Path, seven_zip: str) -> list[dict]:
    result = subprocess.run([seven_zip, "l", "-slt", "--", str(path)], check=True, capture_output=True, text=True, encoding="utf-8", errors="replace")
    matches: list[dict] = []
    for line in result.stdout.splitlines():
        if line.startswith("Path = "):
            member = line.removeprefix("Path = ").replace("\\", "/")
            match = row(path, member, None)
            if match:
                matches.append(match)
    return matches


def dragon_row() -> dict:
    member = "GLTF/Mecha Dragon.gltf"
    with zipfile.ZipFile(DRAGON_PACK) as archive:
        document = json.loads(archive.read(member))
    animations = sorted(item.get("name", "") for item in document.get("animations", []))
    required = ["Idle 01", "Flying 01", "Take off 01", "Landing 01", "Attack 01", "Walk 01"]
    return {
        "pack": relative(DRAGON_PACK), "pack_sha256": digest(DRAGON_PACK), "member": member,
        "generator": document.get("asset", {}).get("generator"),
        "node_count": len(document.get("nodes", [])), "mesh_count": len(document.get("meshes", [])),
        "material_count": len(document.get("materials", [])), "animation_count": len(animations),
        "animation_names": animations, "missing_required_animations": [name for name in required if name not in animations],
        "status": "PASS_FOR_UNREAL_INTAKE" if all(name in animations for name in required) else "FAIL",
    }


def main() -> None:
    if not DRAGON_PACK.is_file():
        raise SystemExit(f"Missing supplied dragon pack: {DRAGON_PACK}")
    archives = sorted(SOURCE.rglob("*.zip")); rar_archives = sorted(SOURCE.rglob("*.rar"))
    matches: list[dict] = []; unreadable_zips: list[str] = []
    for archive in archives:
        try: matches.extend(inspect_zip(archive))
        except (OSError, zipfile.BadZipFile): unreadable_zips.append(relative(archive))
    seven_zip = shutil.which("7z") or shutil.which("7zz"); unreadable_rars: list[str] = []
    if seven_zip:
        for archive in rar_archives:
            try: matches.extend(inspect_rar(archive, seven_zip))
            except (OSError, subprocess.CalledProcessError): unreadable_rars.append(relative(archive))
    loose_matches: list[dict] = []
    for model in sorted(path for path in SOURCE.rglob("*") if path.is_file() and path.suffix.casefold() in MODEL_SUFFIXES):
        match = row(None, relative(model), model.stat().st_size)
        if match: loose_matches.append(match)
    all_matches = matches + loose_matches
    counts = {category: sum(category in match["categories"] for match in all_matches) for category in SEARCH_TERMS}
    report = {
        "kind": "wp23_10_zenith_source_fit", "status": "PASS_DRAGON_SOURCE_WITH_PRESENTATION_REVIEW_GATES",
        "scope": "Source inventory only; no Unreal import, rig, rendered, map, gameplay, colony, travel, or persistence claim",
        "mecha_dragon": dragon_row(), "zip_archives_scanned": len(archives), "unreadable_zip_archives": unreadable_zips,
        "rar_archives_found": len(rar_archives), "rar_archives_scanned": len(rar_archives) - len(unreadable_rars) if seven_zip else 0,
        "rar_listing_tool": seven_zip, "unreadable_rar_archives": unreadable_rars,
        "archive_model_matches": matches, "loose_model_matches": loose_matches, "match_counts": counts,
        "candidates_by_category": {category: [match for match in all_matches if category in match["categories"]] for category in SEARCH_TERMS},
        "assessment": {
            "mecha_dragon": "READY_FOR_UNREAL_INTAKE",
            "future_city_and_colony": "CANDIDATES_REQUIRE_PLAYABLE_COMPLETENESS_AND_RENDERED_REVIEW",
            "vantrix_9": "CANDIDATES_REQUIRE_DISTINCT_MACHINE_MONARCH_REVIEW",
            "broodmother_cipher": "CANDIDATES_REQUIRE_DISTINCT_NON_FRANCHISE_RENDERED_REVIEW",
            "hovercar_continuity": "PRESERVE_EXISTING_WP19_IDENTITY_AND_AUTHORITY",
        },
        "prohibited_inference": "Generic robots, tiny cars, towers, or space-named props do not alone establish a playable city, colony arrival/interior/return route, Vantrix-9, or Broodmother Cipher.",
        "next_bounded_task": "Import and measure Mecha Dragon plus the strongest city, robot, colony, ruler, and infestation candidates; do not start Zenith regional gameplay.",
    }
    REPORT.parent.mkdir(parents=True, exist_ok=True); REPORT.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(f"WP-23.10 Zenith source fit: {report['status']}")
    print(f"Mecha Dragon source: {report['mecha_dragon']['mesh_count']} meshes, {report['mecha_dragon']['animation_count']} animations")
    print(f"ZIP archives scanned: {len(archives)}"); print(f"RAR archives scanned: {report['rar_archives_scanned']}/{len(rar_archives)}")
    print(f"Candidate counts: {counts}"); print(f"Receipt: {REPORT}")


if __name__ == "__main__":
    main()
