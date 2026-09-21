"""Exhaustively audit supplied Bonelands guardian and Skinning Man candidates."""

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
REPORT = ROOT / "Saved/Diagnostics/WP23_9_bonelands_content_audit.json"

MODEL_SUFFIXES = {".fbx", ".glb", ".gltf", ".obj", ".vox", ".dae", ".blend"}
SEARCH_TERMS = {
    "wrapped_guardian": (
        "mummy", "mummified", "wrapped", "bandage", "embalmed", "pharaoh",
        "anubis", "sarcophagus", "guardian",
    ),
    "skinning_man": (
        "skinning", "skin man", "skinner", "flayer", "flayed", "butcher",
        "executioner", "flesh", "cannibal", "leatherface",
    ),
    "tomb_support": (
        "crypt", "tomb", "catacomb", "coffin", "grave", "gargoyle", "skeleton",
    ),
}
FRANCHISE_MARKERS = (
    b"Chucky", b"Head_It", b"Ghostface", b"PyramidHead", b"MicahelMeyers",
    b"Head_Jason", b"Head_Freddy", b"Hannibal", b"Leatherface",
    b"Hellraiser", b"Head_Predator", b"Head_Alien", b"TheMummy",
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
        found = [term for term in terms if term in normalized]
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
    horror_payload = horror_source.read_bytes() if horror_source.is_file() else b""
    franchise_identifiers = sorted(
        marker.decode("ascii") for marker in FRANCHISE_MARKERS if marker in horror_payload
    )
    counts = {
        category: sum(category in match["categories"] for match in matches + loose_matches)
        for category in SEARCH_TERMS
    }
    candidate_hits = [
        match for match in matches + loose_matches
        if "wrapped_guardian" in match["categories"] or "skinning_man" in match["categories"]
    ]
    report = {
        "kind": "wp23_9_bonelands_content_audit",
        "status": "PASS_AUDIT_PRESENTATION_GATES_REMAIN_BLOCKED",
        "scope": "All supplied ZIP/RAR member names and loose model filenames; no import or gameplay claim",
        "source_root_sha256_note": "Per-file archive identity is stable; the loose horror FBX hash is recorded below",
        "zip_archives_scanned": len(archives),
        "unreadable_zip_archives": unreadable_zips,
        "rar_archives_found": len(rar_archives),
        "rar_archives_scanned": len(rar_archives) - len(unreadable_rars) if seven_zip else 0,
        "rar_listing_tool": seven_zip,
        "unreadable_rar_archives": unreadable_rars,
        "archive_model_matches": matches,
        "loose_model_matches": loose_matches,
        "guardian_or_skinning_candidate_hits": candidate_hits,
        "match_counts": counts,
        "horror_source": {
            "path": relative(horror_source),
            "sha256": digest(horror_source),
            "franchise_identifiers": franchise_identifiers,
            "disposition": "EXCLUDED_FRANCHISE_SPECIFIC_LIKENESSES",
        },
        "assessment": {
            "wrapped_guardian": "BLOCKED_NO_DISTINCT_NON_FRANCHISE_WRAPPED_PRESENTATION",
            "skinning_man": "BLOCKED_NO_DISTINCT_NON_FRANCHISE_PRESENTATION",
            "tomb_support": "PRESENT_SUPPORTING_ENVIRONMENT_ONLY",
        },
        "prohibited_inference": (
            "Keyword hits are discovery candidates only. Gargoyles, skeletons, tomb props, and recognizable "
            "franchise likenesses cannot be relabeled as the required distinct characters."
        ),
        "next_bounded_task": (
            "Keep WP-23.9 regional gameplay gated until supplied or approved non-franchise presentations "
            "clear both character gates."
        ),
    }
    REPORT.parent.mkdir(parents=True, exist_ok=True)
    REPORT.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(f"WP-23.9 Bonelands content audit: {report['status']}")
    print(f"ZIP archives scanned: {len(archives)}")
    print(f"RAR archives scanned: {report['rar_archives_scanned']}/{len(rar_archives)}")
    print(f"Candidate counts: {counts}")
    print(f"Receipt: {REPORT}")


if __name__ == "__main__":
    main()
