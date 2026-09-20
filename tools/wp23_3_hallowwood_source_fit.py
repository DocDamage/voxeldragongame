"""Inventory the supplied Hallowwood candidates without claiming Unreal acceptance."""

import hashlib
import io
import json
import zipfile
from pathlib import Path


ROOT = Path(__file__).resolve().parent.parent
SOURCE = ROOT / "assets and old docs"
REPORT = ROOT / "Saved/Diagnostics/WP23_3_hallowwood_source_fit.json"

DRAGON_PACK = SOURCE / "Voxel+Dragons+Pack+Upload.zip"
FOREST_PACK = SOURCE / "voxel/Low_poly_voxel_forest-c8c1e081.zip"
TREE_PACK = SOURCE / "voxel/trees.zip"
PARK_PACK = SOURCE / "voxel/playground park.zip"
VILLAGER_PACK = SOURCE / "voxel/characters/villagers.zip"
WIZARD_PACK = SOURCE / "voxel/characters/wizards.zip"
MONSTER_PACK = SOURCE / "voxel/characters/voxel_monsters.zip"


def digest(path: Path) -> str:
    value = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            value.update(chunk)
    return value.hexdigest()


def require_members(path: Path, expected: list[str]) -> dict:
    with zipfile.ZipFile(path) as archive:
        names = {name.replace("\\", "/") for name in archive.namelist()}
    missing = [name for name in expected if name not in names]
    return {
        "path": str(path.relative_to(ROOT)).replace("\\", "/"),
        "bytes": path.stat().st_size,
        "sha256": digest(path),
        "required_members": expected,
        "missing_members": missing,
        "status": "PASS" if not missing else "FAIL",
    }


def park_inventory() -> dict:
    nested_name = "3D Voxel Park Pack - Obj File.zip"
    with zipfile.ZipFile(PARK_PACK) as outer:
        payload = outer.read(nested_name)
    with zipfile.ZipFile(io.BytesIO(payload)) as nested:
        models = sorted(name for name in nested.namelist() if name.lower().endswith(".obj"))
    trace_models = [
        name for name in models
        if any(token in name for token in ("Playground_Rocking", "Playground_Slide", "Playground_Swing"))
    ]
    return {
        "path": str(PARK_PACK.relative_to(ROOT)).replace("\\", "/"),
        "bytes": PARK_PACK.stat().st_size,
        "sha256": digest(PARK_PACK),
        "nested_archive": nested_name,
        "model_count": len(models),
        "abandoned_carnival_trace_candidates": trace_models,
        "status": "PASS" if trace_models else "FAIL",
    }


def dragon_inventory() -> dict:
    member = "GLTF/Wooden Dragon.gltf"
    with zipfile.ZipFile(DRAGON_PACK) as archive:
        payload = archive.read(member)
    document = json.loads(payload)
    animations = sorted(item.get("name", "") for item in document.get("animations", []))
    required = ["Idle 01", "Flying 01", "Take off 01", "Landing 01", "Attack 01", "Walk 01"]
    missing = [name for name in required if name not in animations]
    return {
        "pack": str(DRAGON_PACK.relative_to(ROOT)).replace("\\", "/"),
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
        "acceptance_boundary": "Source structure only; not an Unreal import, rig profile, or live-PIE proof",
    }


def main() -> None:
    required_files = [
        DRAGON_PACK, FOREST_PACK, TREE_PACK, PARK_PACK,
        VILLAGER_PACK, WIZARD_PACK, MONSTER_PACK,
    ]
    missing_files = [str(path) for path in required_files if not path.is_file()]
    if missing_files:
        raise SystemExit(f"Missing Hallowwood candidate sources: {missing_files}")

    sources = {
        "grovemaw": dragon_inventory(),
        "forest": require_members(FOREST_PACK, [
            "Low_poly_voxel_forest-c8c1e081/fbx/low-poly-voxel-forest_extracted/source/Forest_extracted/Forest/ForestScene.fbx",
            "Low_poly_voxel_forest-c8c1e081/fbx/low-poly-voxel-forest_extracted/source/Forest_extracted/Forest/Forest_tex.png",
        ]),
        "trees": require_members(TREE_PACK, ["Dead_Tree.vox", "Fall_Tree.vox", "Willow.vox"]),
        "park_trace": park_inventory(),
        "osk_candidates": {
            "villager": require_members(VILLAGER_PACK, [
                "Voxel Village/FBX/Characters/TVS_VoxelVillage_OldMan.fbx",
                "Voxel Village/Animations/Human_Idle_Anim.fbx",
                "Voxel Village/Animations/Human_Walk_Anim.fbx",
            ]),
            "wizard": require_members(WIZARD_PACK, [
                "Voxel Wizards/FBX/Characters/TVS_VoxelWizards_MasterWizard.fbx",
                "Voxel Wizards/Animations/Humans/Human_Idle_Anim.fbx",
                "Voxel Wizards/Animations/Humans/Human_Walk_Anim.fbx",
            ]),
            "acceptance_boundary": "Candidate presentations require Unreal import and rendered visual QA",
        },
        "generic_monsters": require_members(MONSTER_PACK, [
            "voxel_monsters/bat.fbx",
            "voxel_monsters/wolf.fbx",
        ]),
    }

    source_checks = [
        sources["grovemaw"]["status"].startswith("PASS"),
        sources["forest"]["status"] == "PASS",
        sources["trees"]["status"] == "PASS",
        sources["park_trace"]["status"] == "PASS",
        sources["osk_candidates"]["villager"]["status"] == "PASS",
        sources["osk_candidates"]["wizard"]["status"] == "PASS",
        sources["generic_monsters"]["status"] == "PASS",
    ]
    report = {
        "kind": "wp23_3_hallowwood_source_fit",
        "status": "PASS_WITH_CONTENT_BLOCKERS" if all(source_checks) else "FAIL",
        "scope": "Portable source-fit evidence only; no Unreal import, gameplay, or acceptance claim",
        "selected_child": "WP-23.3",
        "sources": sources,
        "capability_assessment": {
            "grovemaw_source": "READY_FOR_FOCUSED_UNREAL_INTAKE",
            "hallowwood_environment": "READY_FOR_FOCUSED_UNREAL_INTAKE",
            "osk_grownroot": "CANDIDATES_REQUIRE_RENDERED_VISUAL_QA",
            "carnival_trace": "CANDIDATE_REQUIRES_RENDERED_VISUAL_QA",
            "hollow_harvestman": "BLOCKED_ON_DISTINCT_PRESENTATION_FIT",
            "carnival_presence": "BLOCKED_ON_DISTINCT_PRESENTATION_FIT",
            "unfinished_puppet": "BLOCKED_ON_DISTINCT_PRESENTATION_FIT",
        },
        "next_bounded_task": (
            "Import and measure Wooden Dragon as Grovemaw and the two Osk candidates in an ignored intake folder; "
            "select no ruler identity until rendered QA passes"
        ),
    }
    REPORT.parent.mkdir(parents=True, exist_ok=True)
    REPORT.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(f"WP-23.3 Hallowwood source fit: {report['status']}")
    print(f"Receipt: {REPORT}")


if __name__ == "__main__":
    main()
