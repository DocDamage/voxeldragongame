"""Audit the bounded Magnarok FBX source set in Blender 4.5+."""

import hashlib
import json
from pathlib import Path

import bpy
from mathutils import Vector


ROOT = Path(__file__).resolve().parents[2]
SOURCE = Path(r"G:\3d assets\king_demon_vulture")
OUT = ROOT / "Saved/Diagnostics/WP23_7_MagnarokIntake"
REPORT_PATH = OUT / "source_audit.json"


def sha256(path):
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest().upper()


def reset_scene():
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)
    for actions in list(bpy.data.actions):
        bpy.data.actions.remove(actions)
    for datablocks in (bpy.data.meshes, bpy.data.armatures, bpy.data.materials, bpy.data.images):
        for item in list(datablocks):
            if item.users == 0:
                datablocks.remove(item)


def bounds(objects):
    points = [obj.matrix_world @ Vector(corner) for obj in objects for corner in obj.bound_box]
    if not points:
        return None
    low = [min(point[i] for point in points) for i in range(3)]
    high = [max(point[i] for point in points) for i in range(3)]
    return {"min": low, "max": high, "dimensions": [high[i] - low[i] for i in range(3)]}


def action_record(action):
    start, end = action.frame_range
    slots = []
    for slot in action.slots:
        slots.append({
            "identifier": getattr(slot, "identifier", None),
            "target_id_type": getattr(slot, "target_id_type", None),
        })
    return {
        "name": action.name,
        "frame_range": [float(start), float(end)],
        "frame_count_inclusive": int(round(end - start)) + 1,
        "slots": slots,
    }


def audit_fbx(path):
    reset_scene()
    bpy.ops.import_scene.fbx(filepath=str(path), use_anim=True, ignore_leaf_bones=False)
    meshes = [obj for obj in bpy.context.scene.objects if obj.type == "MESH"]
    armatures = [obj for obj in bpy.context.scene.objects if obj.type == "ARMATURE"]
    return {
        "path": str(path),
        "sha256": sha256(path),
        "mesh_count": len(meshes),
        "armature_count": len(armatures),
        "object_types": {kind: len([obj for obj in bpy.context.scene.objects if obj.type == kind]) for kind in sorted({obj.type for obj in bpy.context.scene.objects})},
        "mesh_names": [obj.name for obj in meshes],
        "vertex_count": sum(len(obj.data.vertices) for obj in meshes),
        "polygon_count": sum(len(obj.data.polygons) for obj in meshes),
        "material_names": sorted({slot.material.name for obj in meshes for slot in obj.material_slots if slot.material}),
        "material_slot_count": sum(len(obj.material_slots) for obj in meshes),
        "image_sources": sorted({bpy.path.abspath(image.filepath) for image in bpy.data.images if image.filepath}),
        "bounds": bounds(meshes),
        "armatures": [
            {
                "name": armature.name,
                "bone_count": len(armature.data.bones),
                "bone_names": [bone.name for bone in armature.data.bones],
            }
            for armature in armatures
        ],
        "armature_modifiers": [
            {"mesh": obj.name, "targets": [modifier.object.name if modifier.object else None for modifier in obj.modifiers if modifier.type == "ARMATURE"]}
            for obj in meshes
        ],
        "vertex_group_counts": {obj.name: len(obj.vertex_groups) for obj in meshes},
        "actions": [action_record(action) for action in bpy.data.actions],
        "scene_frame_range": [bpy.context.scene.frame_start, bpy.context.scene.frame_end],
        "fps": bpy.context.scene.render.fps / bpy.context.scene.render.fps_base,
    }


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    fbxs = sorted(SOURCE.glob("*.fbx"), key=lambda path: path.name.casefold())
    textures = sorted((path for path in SOURCE.glob("*.png")), key=lambda path: path.name.casefold())
    if not fbxs:
        raise RuntimeError("No FBX files found")
    report = {
        "kind": "wp23_7_magnarok_source_audit",
        "scope": "source and animation integrity only; no selection, Unreal acceptance, gameplay, map, facts, Echo, travel, save, or regional completion",
        "blender": bpy.app.version_string,
        "source_root": str(SOURCE),
        "status": "RUNNING",
        "fbx": {},
        "textures": {path.name: {"path": str(path), "sha256": sha256(path), "bytes": path.stat().st_size} for path in textures},
    }
    REPORT_PATH.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    try:
        for path in fbxs:
            report["fbx"][path.name] = audit_fbx(path)
        skeletons = {
            name: entry["armatures"][0]["bone_names"]
            for name, entry in report["fbx"].items() if len(entry["armatures"]) == 1
        }
        signatures = {name: hashlib.sha256("\n".join(bones).encode("utf-8")).hexdigest().upper() for name, bones in skeletons.items()}
        report["skeleton_name_signatures"] = signatures
        report["all_skinned_fbx_skeleton_names_match"] = len(set(signatures.values())) == 1 and len(signatures) >= 2
        report["status"] = "PASS_AUDIT"
    except Exception as exc:
        report["status"] = "ERROR"
        report["error"] = repr(exc)
        raise
    finally:
        REPORT_PATH.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(f"WP-23.7 Magnarok source audit: {report['status']}")
    print(f"Receipt: {REPORT_PATH}")


if __name__ == "__main__":
    main()
