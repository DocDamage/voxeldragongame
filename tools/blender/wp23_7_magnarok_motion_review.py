"""Audit deformation in Magnarok's supplied walk/run FBXs at fixed phases."""

import hashlib
import json
from pathlib import Path

import bpy


ROOT = Path(__file__).resolve().parents[2]
SOURCE = Path(r"G:\3d assets\king_demon_vulture")
OUT = ROOT / "Saved/Diagnostics/WP23_7_MagnarokIntake/motion_review.json"
CLIPS = ("Animation_Walking_withSkin.fbx", "Animation_Running_withSkin.fbx")
FINE = ROOT / "Saved/Diagnostics/WP23_7_MagnarokIntake/Magnarok_VoxelFine_Rigged.fbx"
PHASES = (0.0, 0.25, 0.5, 0.75)


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def reset():
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)
    for action in list(bpy.data.actions):
        if action.users == 0:
            bpy.data.actions.remove(action)


def sample(mesh, frame):
    bpy.context.scene.frame_set(frame)
    depsgraph = bpy.context.evaluated_depsgraph_get()
    evaluated = mesh.evaluated_get(depsgraph)
    result = evaluated.to_mesh()
    try:
        points = [evaluated.matrix_world @ vertex.co for vertex in result.vertices]
        return {
            "frame": frame,
            "bounds_min": [min(point[i] for point in points) for i in range(3)],
            "bounds_max": [max(point[i] for point in points) for i in range(3)],
            "positions": [tuple(point) for point in points],
        }
    finally:
        evaluated.to_mesh_clear()


def audit_clip(path, selected_fine=False):
    reset()
    if selected_fine:
        bpy.ops.import_scene.fbx(filepath=str(FINE), use_anim=False, ignore_leaf_bones=False)
        selected_mesh = next(obj for obj in bpy.context.scene.objects if obj.type == "MESH")
        selected_armature = next(obj for obj in bpy.context.scene.objects if obj.type == "ARMATURE")
    bpy.ops.import_scene.fbx(filepath=str(path), use_anim=True, ignore_leaf_bones=False)
    meshes = [obj for obj in bpy.context.scene.objects if obj.type == "MESH" and (not selected_fine or obj != selected_mesh)]
    armatures = [obj for obj in bpy.context.scene.objects if obj.type == "ARMATURE" and (not selected_fine or obj != selected_armature)]
    if len(meshes) != 1 or len(armatures) != 1:
        raise RuntimeError("Unexpected source rig count: " + path.name)
    mesh, armature = meshes[0], armatures[0]
    action = armature.animation_data.action if armature.animation_data else None
    if action is None:
        raise RuntimeError("No active armature action: " + path.name)
    if selected_fine:
        selected_armature.animation_data_create()
        selected_armature.animation_data.action = action
        selected_armature.animation_data.action_slot = armature.animation_data.action_slot
        mesh = selected_mesh
    first, last = action.frame_range
    frames = [round(first + (last - first) * phase) for phase in PHASES]
    samples = [sample(mesh, frame) for frame in frames]
    reference = samples[0]["positions"]
    reference_centroid = [sum(point[i] for point in reference) / len(reference) for i in range(3)]
    result = {
        "path": str(path), "sha256": digest(path),
        "evaluated_mesh": "selected_fine" if selected_fine else "source",
        "mesh_asset_path": str(FINE if selected_fine else path),
        "mesh_asset_sha256": digest(FINE if selected_fine else path),
        "bone_names_match_source": ([bone.name for bone in selected_armature.data.bones] == [bone.name for bone in armature.data.bones]) if selected_fine else True,
        "action": action.name, "frame_range": [first, last],
        "mesh_vertices": len(mesh.data.vertices),
        "bone_count": len(armature.data.bones),
        "samples": [],
    }
    for phase, item in zip(PHASES, samples):
        deltas = [
            sum((coordinate - original) ** 2 for coordinate, original in zip(position, base)) ** 0.5
            for position, base in zip(item["positions"], reference)
        ]
        centroid = [sum(point[i] for point in item["positions"]) / len(reference) for i in range(3)]
        shape_deltas = [
            sum(((coordinate - centroid[i]) - (original - reference_centroid[i])) ** 2
                for i, (coordinate, original) in enumerate(zip(position, base))) ** 0.5
            for position, base in zip(item["positions"], reference)
        ]
        result["samples"].append({
            "phase": phase, "frame": item["frame"],
            "bounds_min": item["bounds_min"], "bounds_max": item["bounds_max"],
            "vertex_delta_max_m": max(deltas),
            "vertex_delta_mean_m": sum(deltas) / len(deltas),
            "moved_vertices_over_1cm": sum(delta > 0.01 for delta in deltas),
            "centroid_shift_m": [centroid[i] - reference_centroid[i] for i in range(3)],
            "shape_delta_max_m": max(shape_deltas),
            "shape_delta_mean_m": sum(shape_deltas) / len(shape_deltas),
            "shape_vertices_over_1cm": sum(delta > 0.01 for delta in shape_deltas),
        })
    return result


def main():
    report = {
        "kind": "wp23_7_magnarok_source_motion_review",
        "scope": "source and selected fine voxel rig deformation in Blender using supplied walk/run actions; not UE animated-deformation or boss-performance proof",
        "blender": bpy.app.version_string,
        "status": "RUNNING",
        "clips": {},
    }
    OUT.parent.mkdir(parents=True, exist_ok=True)
    try:
        for name in CLIPS:
            report["clips"][name] = audit_clip(SOURCE / name)
            report["clips"][name]["selected_fine"] = audit_clip(SOURCE / name, True)
        report["status"] = "SOURCE_AND_FINE_RIG_MOTION_SAMPLED"
    except Exception as exc:
        report["status"] = "ERROR"
        report["error"] = repr(exc)
        raise
    finally:
        OUT.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print("Magnarok source motion: " + report["status"])


if __name__ == "__main__":
    main()
