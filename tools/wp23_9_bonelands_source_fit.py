"""Inventory supplied Bonelands candidates without claiming Unreal acceptance."""

import hashlib
import io
import json
import re
import zipfile
from pathlib import Path


ROOT = Path(__file__).resolve().parent.parent
SOURCE = ROOT / "assets and old docs"
REPORT = ROOT / "Saved/Diagnostics/WP23_9_bonelands_source_fit.json"

DRAGON_PACK = SOURCE / "Voxel+Dragons+Pack+Upload.zip"
CEMETERY_PACK = SOURCE / "voxel/cemetary and church voxel set.zip"
CATHEDRAL_PACK = SOURCE / "voxel/characters/Voxel Cathedral.zip"
KNIGHTS_PACK = SOURCE / "voxel/characters/knights.zip"
HORROR_SOURCE = SOURCE / "voxel/characters/horror characters.fbx"
MODEL_SUFFIXES = {".fbx", ".obj", ".gltf", ".glb", ".dae", ".vox", ".blend"}


def relative(path: Path) -> str:
    return path.relative_to(ROOT).as_posix()


def digest(path: Path) -> str:
    value = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            value.update(chunk)
    return value.hexdigest()


def model_members(path: Path) -> list[str]:
    with zipfile.ZipFile(path) as archive:
        return sorted(
            name.replace("\\", "/")
            for name in archive.namelist()
            if Path(name).suffix.casefold() in MODEL_SUFFIXES
        )


def nested_model_members(path: Path) -> list[str]:
    members: list[str] = []
    with zipfile.ZipFile(path) as archive:
        for name in archive.namelist():
            if not name.casefold().endswith(".zip"):
                continue
            with zipfile.ZipFile(io.BytesIO(archive.read(name))) as nested:
                members.extend(
                    f"{name}!/{member.replace(chr(92), '/')}"
                    for member in nested.namelist()
                    if Path(member).suffix.casefold() in MODEL_SUFFIXES
                )
    return sorted(members)


def contains_any(name: str, terms: tuple[str, ...]) -> bool:
    normalized = re.sub(r"[_./\\-]+", " ", name).casefold()
    return any(term in normalized for term in terms)


def dragon_row() -> dict:
    member = "GLTF/Skull Dragon.gltf"
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


def source_row(path: Path, members: list[str]) -> dict:
    return {
        "path": relative(path),
        "bytes": path.stat().st_size,
        "sha256": digest(path),
        "model_count": len(members),
    }


def main() -> None:
    required = [DRAGON_PACK, CEMETERY_PACK, CATHEDRAL_PACK, KNIGHTS_PACK, HORROR_SOURCE]
    missing = [str(path) for path in required if not path.is_file()]
    if missing:
        raise SystemExit(f"Missing Bonelands candidate sources: {missing}")

    cemetery = nested_model_members(CEMETERY_PACK)
    cathedral = model_members(CATHEDRAL_PACK)
    knights = model_members(KNIGHTS_PACK)
    tomb_terms = ("cata", "catac", "crypt", "coffin", "grave", "tomb", "skull", "church", "cathedral")
    guardian_terms = ("gargoyle", "skele", "mummy", "wrapped", "guardian")
    ruler_terms = ("commander", "champion", "captain", "crusader", "priest")

    horror_payload = HORROR_SOURCE.read_bytes()
    horror_identifiers = sorted({
        value.decode("latin1", "ignore")
        for value in re.findall(rb"[ -~]{8,}", horror_payload)
        if b"mummy" in value.lower() or b"leatherface" in value.lower()
    })

    report = {
        "kind": "wp23_9_bonelands_source_fit",
        "status": "PASS_WITH_GUARDIAN_AND_OPTIONAL_HORROR_BLOCKERS",
        "scope": "Source inventory only; no Unreal import, rendered acceptance, map, gameplay, or persistence claim",
        "ossuroth": dragon_row(),
        "cemetery_source": {
            **source_row(CEMETERY_PACK, cemetery),
            "nested_model_count": len(cemetery),
            "tomb_trace_members": [name for name in cemetery if contains_any(name, tomb_terms)],
            "guardian_candidates": [name for name in cemetery if contains_any(name, guardian_terms)],
        },
        "cathedral_source": {
            **source_row(CATHEDRAL_PACK, cathedral),
            "tomb_trace_members": [name for name in cathedral if contains_any(name, tomb_terms)],
            "kael_candidates": [name for name in cathedral if contains_any(name, ruler_terms)],
        },
        "knights_source": {
            **source_row(KNIGHTS_PACK, knights),
            "kael_candidates": [name for name in knights if contains_any(name, ruler_terms)],
        },
        "excluded_horror_source": {
            "path": relative(HORROR_SOURCE),
            "bytes": HORROR_SOURCE.stat().st_size,
            "sha256": digest(HORROR_SOURCE),
            "mummy_or_leatherface_identifiers": horror_identifiers,
            "disposition": "EXCLUDED_FRANCHISE_SPECIFIC_LIKENESSES",
            "authority": "Documentation/DesignPack/docs/ART_DIRECTION.md:51",
        },
        "assessment": {
            "ossuroth": "READY_FOR_UNREAL_INTAKE",
            "kael_marrow": "CANDIDATES_REQUIRE_RENDERED_REVIEW",
            "bonelands_tomb_environment": "PASS_SOURCE_FIT_FOR_FOCUSED_INTAKE",
            "wrapped_guardian": "BLOCKED_NO_DISTINCT_NON_FRANCHISE_WRAPPED_PRESENTATION",
            "skinning_man": "BLOCKED_NO_DISTINCT_NON_FRANCHISE_PRESENTATION",
        },
        "next_bounded_task": (
            "Import and render the Skull Dragon, strongest Kael candidates, and representative tomb props; "
            "do not start regional gameplay or relabel franchise likenesses"
        ),
    }
    REPORT.parent.mkdir(parents=True, exist_ok=True)
    REPORT.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(f"WP-23.9 Bonelands source fit: {report['status']}")
    print(f"Ossuroth source: {report['ossuroth']['mesh_count']} meshes, {report['ossuroth']['animation_count']} animations")
    print(f"Cemetery models: {len(cemetery)}; Cathedral models: {len(cathedral)}; Knight models: {len(knights)}")
    print(f"Receipt: {REPORT}")


if __name__ == "__main__":
    main()
