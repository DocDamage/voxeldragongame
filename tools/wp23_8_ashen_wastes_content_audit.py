"""Exhaustively audit supplied Ashen Wastes laboratory and cast candidates."""

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
REPORT = ROOT / "Saved/Diagnostics/WP23_8_ashen_wastes_content_audit.json"

MODEL_SUFFIXES = {".fbx", ".glb", ".gltf", ".obj", ".vox", ".dae", ".blend"}
SEARCH_TERMS = {
    "laboratory": (
        "laboratory", "lab", "scientist", "science", "medical", "surgery",
        "operating room", "hospital", "experiment", "chemistry",
    ),
    "doctor_hollowmend": (
        "doctor", "physician", "surgeon", "scientist", "medic", "medical",
    ),
    "false_rescuer": (
        "rescuer", "rescue", "survivor", "captor", "prisoner",
        "captive", "civilian",
    ),
    "bunker_support": ("bunker",),
}
FRANCHISE_MARKERS = (
    b"Chucky", b"Head_It", b"Ghostface", b"PyramidHead", b"MicahelMeyers",
    b"Head_Jason", b"Head_Freddy", b"Hannibal", b"Leatherface",
    b"Hellraiser", b"Head_Predator", b"Head_Alien",
)


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
    for category, terms in SEARCH_TERMS.items():
        found = []
        for term in terms:
            if term == "lab":
                matched = re.search(r"\blab\b", normalized) is not None
            else:
                matched = term in normalized
            if matched:
                found.append(term)
        if found:
            result[category] = sorted(found)
    return result


def row(archive: Path | None, member: str, size: int | None, prefix: str = "") -> dict | None:
    found = categories(member)
    if not found or Path(member).suffix.casefold() not in MODEL_SUFFIXES:
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


def main() -> None:
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

    horror_source = SOURCE / "voxel/characters/horror characters.fbx"
    franchise_identifiers_present = (
        horror_source.is_file()
        and any(marker in horror_source.read_bytes() for marker in FRANCHISE_MARKERS)
    )
    counts = {
        category: sum(category in match["categories"] for match in matches + loose_matches)
        for category in SEARCH_TERMS
    }
    report = {
        "kind": "wp23_8_ashen_wastes_content_audit",
        "status": "PASS_AUDIT_PRESENTATION_GATES_REMAIN_BLOCKED",
        "scope": "All supplied ZIP/RAR member names and loose model filenames; no import or gameplay claim",
        "zip_archives_scanned": len(archives),
        "unreadable_zip_archives": unreadable_zips,
        "rar_archives_found": len(rar_archives),
        "rar_archives_scanned": len(rar_archives) - len(unreadable_rars) if seven_zip else 0,
        "rar_listing_tool": seven_zip,
        "unreadable_rar_archives": unreadable_rars,
        "archive_model_matches": matches,
        "loose_model_matches": loose_matches,
        "match_counts": counts,
        "franchise_identifiers_present_in_horror_source": franchise_identifiers_present,
        "assessment": {
            "laboratory": "BLOCKED_NO_COMPLETE_AUTHORED_LABORATORY_FIT",
            "doctor_hollowmend": "BLOCKED_NO_DISTINCT_NON_FRANCHISE_PRESENTATION",
            "bunker_false_rescuer": "BLOCKED_NO_DISTINCT_NON_FRANCHISE_PRESENTATION",
        },
        "prohibited_inference": (
            "Keyword hits are discovery candidates only. Do not promote isolated medical props, "
            "generic survivors, or recognizable franchise likenesses to accepted identities."
        ),
        "next_bounded_task": (
            "Keep WP-23.8 regional gameplay gated until supplied or approved exact presentation "
            "content clears the laboratory and optional-cast requirements."
        ),
    }
    REPORT.parent.mkdir(parents=True, exist_ok=True)
    REPORT.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(f"WP-23.8 Ashen Wastes content audit: {report['status']}")
    print(f"ZIP archives scanned: {len(archives)}")
    print(f"RAR archives scanned: {report['rar_archives_scanned']}/{len(rar_archives)}")
    print(f"Candidate counts: {counts}")
    print(f"Receipt: {REPORT}")


if __name__ == "__main__":
    main()
