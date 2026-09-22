"""Bounded owned-source animation audit for Magnarok's missing boss motions."""

import hashlib
import json
from pathlib import Path

import bpy


ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / "Saved/Diagnostics/WP23_7_MagnarokIntake/animation_source_audit.json"
SOURCE_AUDIT = ROOT / "Saved/Diagnostics/WP23_7_MagnarokIntake/source_audit.json"
CANDIDATES = {
    "loose_idle_unknown_provenance": Path(r"G:\3d assets\fbx\New folder\Idle2.fbx"),
    "free_animation_levitation_attack": Path(r"G:\3d assets\FreeAnim66603c4535e6V1\FreeAnimationsPack\Animations\AS_AttackInLevitation2.fbx"),
    "human_basic_male_idle": Path(r"G:\3d assets\Human Basic Motions FREE\Animations\Male\Idles\HumanM@Idle01.fbx"),
    "free_knight_embedded": Path(r"G:\3d assets\Free_Rigged_Knights___Game_Ready_Characters_with_Animations-ada783ab\fbx\knight_extracted\knight\Knight 1\Knight.fbx"),
    "universal_library_1": Path(r"G:\3d assets\Universal Animation Library[Standard]\Universal Animation Library[Standard]\Unreal-Godot\UAL1_Standard.glb"),
    "universal_library_2": Path(r"G:\3d assets\Universal Animation Library 2[Standard]\Universal Animation Library 2[Standard]\Unreal-Godot\UAL2_Standard.glb"),
}
SEMANTIC = ("idle", "attack", "hit", "damage", "death", "defeat", "recover", "phase", "roar", "cast", "levitat", "stagger")


def sha256(path):
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest().upper()


def reset():
    bpy.ops.wm.read_factory_settings(use_empty=True)


def audit(path, expected):
    result = {"path": str(path), "source_listing_and_license_recorded": False}
    if not path.is_file():
        result["status"] = "MISSING"
        return result
    result["sha256"] = sha256(path)
    result["bytes"] = path.stat().st_size
    try:
        reset()
        if path.suffix.lower() == ".fbx":
            bpy.ops.import_scene.fbx(filepath=str(path), use_anim=True, ignore_leaf_bones=False)
        else:
            bpy.ops.import_scene.gltf(filepath=str(path))
        armatures = [obj for obj in bpy.context.scene.objects if obj.type == "ARMATURE"]
        actions = list(bpy.data.actions)
        expected_lower = {name.lower() for name in expected}
        result["armatures"] = []
        for armature in armatures:
            names = [bone.name for bone in armature.data.bones]
            normalized = {name.lower().removeprefix("mixamorig:") for name in names}
            result["armatures"].append({
                "name": armature.name,
                "bone_count": len(names),
                "bone_names": names,
                "parent_names": {bone.name: bone.parent.name if bone.parent else None for bone in armature.data.bones},
                "magnarok_bone_name_overlap": len(normalized & expected_lower),
                "magnarok_exact_bone_set": normalized == expected_lower,
                "magnarok_missing_bones": sorted(expected_lower - normalized),
            })
        result["actions"] = [
            {"name": action.name, "frame_range": list(action.frame_range)}
            for action in actions
        ]
        result["semantic_name_hits"] = [action.name for action in actions if any(word in action.name.lower() for word in SEMANTIC)]
        result["mesh_count"] = sum(obj.type == "MESH" for obj in bpy.context.scene.objects)
        result["status"] = "STRUCTURE_AUDITED"
    except Exception as exc:
        result["status"] = "ERROR"
        result["error"] = repr(exc)
    return result


def main():
    expected = json.loads(SOURCE_AUDIT.read_text(encoding="utf-8"))["fbx"]["Character_output.fbx"]["armatures"][0]["bone_names"]
    report = {
        "kind": "wp23_7_magnarok_owned_animation_source_audit",
        "scope": "bounded structural comparison only; filenames, animation actions, and bone-name overlap do not prove retarget quality or boss-performance acceptance",
        "blender": bpy.app.version_string,
        "status": "RUNNING",
        "magnarok_bone_names": expected,
        "candidates": {},
    }
    OUT.parent.mkdir(parents=True, exist_ok=True)
    for label, path in CANDIDATES.items():
        report["candidates"][label] = audit(path, expected)
    report["status"] = "AUDITED_WITHOUT_SELECTION"
    OUT.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print("Magnarok owned animation source audit: " + report["status"])


if __name__ == "__main__":
    main()
