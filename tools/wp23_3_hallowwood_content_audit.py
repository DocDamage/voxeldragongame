"""Audit supplied Hallowwood presentation candidates without clearing content gates."""

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
REPORT = ROOT / "Saved/Diagnostics/WP23_3_hallowwood_content_audit.json"

MODEL_SUFFIXES = {".fbx", ".glb", ".gltf", ".obj", ".vox", ".dae", ".blend"}
SEARCH_TERMS = {
    "authored_carnival": (
        "carnival", "circus", "carousel", "ferris", "big top", "ringmaster",
        "ticket booth", "marionette", "puppet", "scarecrow", "harvestman",
    ),
    "supporting_trace": (
        "tent", "banner", "wagon", "cart", "bouncy horse", "hot dog stand",
        "ice cream stand", "gallows",
    ),
}
FRANCHISE_MARKERS = {
    "Chucky": b"Chucky",
    "It": b"Head_It",
    "Ghostface": b"Ghostface",
    "PyramidHead": b"PyramidHead",
    "MicahelMeyers": b"MicahelMeyers",
    "Jason": b"Head_Jason",
    "Freddy": b"Head_Freddy",
    "Hannibal": b"Hannibal",
    "Leatherface": b"Leatherface",
    "Hellraiser": b"Hellraiser",
    "Predator": b"Head_Predator",
    "Alien": b"Head_Alien",
}


def relative(path: Path) -> str:
    return path.relative_to(ROOT).as_posix()


def digest(path: Path) -> str:
    value = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            value.update(chunk)
    return value.hexdigest()


def term_matches(name: str) -> set[str]:
    normalized = re.sub(r"[_./\\-]+", " ", name).casefold()
    return {
        term
        for terms in SEARCH_TERMS.values()
        for term in terms
        if term in normalized
    }


def inspect_zip(path: Path, payload: bytes | None = None, prefix: str = "") -> list[dict]:
    source = io.BytesIO(payload) if payload is not None else path
    matches: list[dict] = []
    with zipfile.ZipFile(source) as archive:
        for info in archive.infolist():
            member = info.filename.replace("\\", "/")
            terms = term_matches(member)
            if terms and Path(member).suffix.casefold() in MODEL_SUFFIXES:
                matches.append({
                    "archive": relative(path),
                    "member": f"{prefix}{member}",
                    "terms": sorted(terms),
                    "bytes": info.file_size,
                })
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
        if not line.startswith("Path = "):
            continue
        member = line.removeprefix("Path = ").replace("\\", "/")
        terms = term_matches(member)
        if terms and Path(member).suffix.casefold() in MODEL_SUFFIXES:
            matches.append({
                "archive": relative(path),
                "member": member,
                "terms": sorted(terms),
                "bytes": None,
            })
    return matches


def inspect_franchise_source(path: Path) -> dict:
    payload = path.read_bytes()
    detected = [name for name, marker in FRANCHISE_MARKERS.items() if marker in payload]
    return {
        "path": relative(path),
        "bytes": path.stat().st_size,
        "sha256": digest(path),
        "detected_source_identifiers": detected,
        "disposition": "EXCLUDED_FRANCHISE_SPECIFIC_LIKENESSES",
        "authority": "Documentation/DesignPack/docs/ART_DIRECTION.md:51",
    }


def main() -> None:
    archives = sorted(SOURCE.rglob("*.zip"))
    rar_archives = sorted(SOURCE.rglob("*.rar"))
    matches: list[dict] = []
    unreadable: list[str] = []
    for archive in archives:
        try:
            matches.extend(inspect_zip(archive))
        except (OSError, zipfile.BadZipFile):
            unreadable.append(relative(archive))

    seven_zip = shutil.which("7z") or shutil.which("7zz")
    unreadable_rars: list[str] = []
    if seven_zip:
        for archive in rar_archives:
            try:
                matches.extend(inspect_rar(archive, seven_zip))
            except (OSError, subprocess.CalledProcessError):
                unreadable_rars.append(relative(archive))

    authored_terms = set(SEARCH_TERMS["authored_carnival"])
    authored = [row for row in matches if authored_terms.intersection(row["terms"])]
    trace = [row for row in matches if not authored_terms.intersection(row["terms"])]
    horror_source = SOURCE / "voxel/characters/horror characters.fbx"
    if not horror_source.is_file():
        raise SystemExit(f"Missing required source: {horror_source}")

    report = {
        "kind": "wp23_3_hallowwood_content_audit",
        "status": "PASS_AUDIT_CONTENT_GATES_REMAIN_BLOCKED",
        "scope": "Supplied ZIP member names and source FBX identifiers; no import or gameplay claim",
        "zip_archives_scanned": len(archives),
        "unreadable_zip_archives": unreadable,
        "rar_archives_found": len(rar_archives),
        "rar_archives_scanned": len(rar_archives) - len(unreadable_rars) if seven_zip else 0,
        "rar_listing_tool": seven_zip,
        "unreadable_rar_archives": unreadable_rars,
        "authored_carnival_model_matches": authored,
        "supporting_trace_model_matches": trace,
        "franchise_source": inspect_franchise_source(horror_source),
        "assessment": {
            "abandoned_traveling_carnival": (
                "BLOCKED_EXACT_FIT; supporting props can strengthen a future trace composition "
                "but do not constitute an authored traveling-carnival kit"
            ),
            "hollow_harvestman": "BLOCKED_NO_DISTINCT_NON_FRANCHISE_PRESENTATION",
            "carnival_presence": "BLOCKED_NO_DISTINCT_NON_FRANCHISE_PRESENTATION",
            "unfinished_puppet": "BLOCKED_NO_DISTINCT_NON_FRANCHISE_PRESENTATION",
        },
        "prohibited_inference": (
            "Do not relabel playground props, generic monsters, or recognizable movie-character "
            "parts as the required Hallowwood identities"
        ),
        "next_bounded_task": (
            "Keep WP-23.3 region work gated and evaluate the next readiness-ranked WP-23 child "
            "against supplied content"
        ),
    }
    REPORT.parent.mkdir(parents=True, exist_ok=True)
    REPORT.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(f"WP-23.3 Hallowwood content audit: {report['status']}")
    print(f"ZIP archives scanned: {len(archives)}")
    print(f"RAR archives scanned: {report['rar_archives_scanned']}/{len(rar_archives)}")
    print(f"Authored carnival model matches: {len(authored)}")
    print(f"Supporting trace model matches: {len(trace)}")
    print(f"Receipt: {REPORT}")


if __name__ == "__main__":
    main()
