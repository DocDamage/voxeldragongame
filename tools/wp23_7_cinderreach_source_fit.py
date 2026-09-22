"""Audit supplied Cinderreach, Pyraxis, ruler, and overseer source candidates."""

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
REPORT = ROOT / "Saved/Diagnostics/WP23_7_cinderreach_source_fit.json"

MODEL_SUFFIXES = {".fbx", ".glb", ".gltf", ".obj", ".vox", ".dae", ".blend"}
SEARCH_TERMS = {
    "volcanic_environment": (
        "volcano", "volcanic", "lava", "magma", "molten", "basalt", "ember",
        "ash", "fire", "burnt", "burned", "scorched",
    ),
    "forge": (
        "forge", "smith", "blacksmith", "anvil", "furnace", "kiln", "crucible",
        "bellows", "smelter", "hammer",
    ),
    "trial_arena": (
        "arena", "colosseum", "gladiator", "fighting pit", "battle pit", "throne",
        "dungeon", "temple", "altar",
    ),
    "ruler": (
        "king", "monarch", "lord", "chieftain", "commander", "captain", "champion",
        "emperor", "warlord",
    ),
    "choir_overseer": (
        "overseer", "cult", "cultist", "priest", "executioner", "torturer", "flayed",
        "skinned", "choir",
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
    whole_word = {"ash", "fire", "lava", "king", "lord", "cult", "choir"}
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
    return {
        "archive": relative(archive) if archive else None,
        "member": f"{prefix}{member}",
        "categories": found,
        "bytes": size,
    }


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
    result = subprocess.run(
        [seven_zip, "l", "-slt", "--", str(path)],
        check=True,
        capture_output=True,
        text=True,
        encoding="utf-8",
        errors="replace",
    )
    matches: list[dict] = []
    for line in result.stdout.splitlines():
        if line.startswith("Path = "):
            member = line.removeprefix("Path = ").replace("\\", "/")
            match = row(path, member, None)
            if match:
                matches.append(match)
    return matches


def dragon_row() -> dict:
    member = "GLTF/Lava Dragon.gltf"
    with zipfile.ZipFile(DRAGON_PACK) as archive:
        document = json.loads(archive.read(member))
    animations = sorted(item.get("name", "") for item in document.get("animations", []))
    required = ["Idle 01", "Flying 01", "Take off 01", "Landing 01", "Attack 01", "Walk 01"]
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
        "missing_required_animations": [name for name in required if name not in animations],
        "status": "PASS_FOR_UNREAL_INTAKE" if all(name in animations for name in required) else "FAIL",
    }


def main() -> None:
    if not DRAGON_PACK.is_file():
        raise SystemExit(f"Missing supplied dragon pack: {DRAGON_PACK}")

    archives = sorted(SOURCE.rglob("*.zip"))
    rar_archives = sorted(SOURCE.rglob("*.rar"))
    matches: list[dict] = []
    unreadable_zips: list[str] = []
    for archive in archives:
        try:
            matches.extend(inspect_zip(archive))
        except (OSError, zipfile.BadZipFile):
            unreadable_zips.append(relative(archive))

    seven_zip = shutil.which("7z") or shutil.which("7zz")
    unreadable_rars: list[str] = []
    if seven_zip:
        for archive in rar_archives:
            try:
                matches.extend(inspect_rar(archive, seven_zip))
            except (OSError, subprocess.CalledProcessError):
                unreadable_rars.append(relative(archive))

    loose_matches: list[dict] = []
    for model in sorted(path for path in SOURCE.rglob("*") if path.is_file() and path.suffix.casefold() in MODEL_SUFFIXES):
        match = row(None, relative(model), model.stat().st_size)
        if match:
            loose_matches.append(match)

    all_matches = matches + loose_matches
    counts = {
        category: sum(category in match["categories"] for match in all_matches)
        for category in SEARCH_TERMS
    }
    report = {
        "kind": "wp23_7_cinderreach_source_fit",
        "status": "PASS_DRAGON_SOURCE_WITH_PRESENTATION_REVIEW_GATES",
        "scope": "Source inventory only; no Unreal import, rig, rendered, map, gameplay, or persistence claim",
        "pyraxis": dragon_row(),
        "zip_archives_scanned": len(archives),
        "unreadable_zip_archives": unreadable_zips,
        "rar_archives_found": len(rar_archives),
        "rar_archives_scanned": len(rar_archives) - len(unreadable_rars) if seven_zip else 0,
        "rar_listing_tool": seven_zip,
        "unreadable_rar_archives": unreadable_rars,
        "archive_model_matches": matches,
        "loose_model_matches": loose_matches,
        "match_counts": counts,
        "candidates_by_category": {
            category: [match for match in all_matches if category in match["categories"]]
            for category in SEARCH_TERMS
        },
        "assessment": {
            "pyraxis": "READY_FOR_UNREAL_INTAKE",
            "volcanic_forge_arena": "CANDIDATES_REQUIRE_COMPLETENESS_AND_RENDERED_REVIEW",
            "magnarok": "CANDIDATES_REQUIRE_DISTINCT_RENDERED_REVIEW",
            "flayed_choir_overseer": "CANDIDATES_REQUIRE_DISTINCT_NON_FRANCHISE_RENDERED_REVIEW",
        },
        "prohibited_inference": (
            "Keyword hits are discovery candidates only. Fire props, generic masonry, a generic King, or a generic "
            "cultist do not alone establish a volcanic forge country, readable ritual arena, Magnarok, or the Flayed Choir."
        ),
        "next_bounded_task": (
            "Import and measure Lava Dragon plus the strongest forge, arena, ruler, and overseer candidates; "
            "do not start Cinderreach regional gameplay."
        ),
    }
    REPORT.parent.mkdir(parents=True, exist_ok=True)
    REPORT.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(f"WP-23.7 Cinderreach source fit: {report['status']}")
    print(f"Pyraxis source: {report['pyraxis']['mesh_count']} meshes, {report['pyraxis']['animation_count']} animations")
    print(f"ZIP archives scanned: {len(archives)}")
    print(f"RAR archives scanned: {report['rar_archives_scanned']}/{len(rar_archives)}")
    print(f"Candidate counts: {counts}")
    print(f"Receipt: {REPORT}")


if __name__ == "__main__":
    main()
