"""Read-only Blender check of the supplied Tamsin mesh and walk source.

It applies the imported walk action to the mesh armature only in memory and
compares evaluated bounds at frame zero and midpoint. This distinguishes a
source-animation mismatch from Unreal import/playback behavior without
creating game assets or saving a blend file.
"""

import json
from pathlib import Path

import bpy


ROOT = Path(__file__).resolve().parent.parent
MESH_SOURCE = ROOT / "Saved" / "AssetIntake" / "WP12" / "NPC" / "Voxel Rangers" / "FBX" / "Character" / "TVS_VoxelRangers_Captain.fbx"
WALK_SOURCE = ROOT / "Saved" / "AssetIntake" / "WP12" / "NPC" / "Voxel Rangers" / "Animations" / "Human_Walk_Anim.fbx"
OUTPUT = ROOT / "Saved" / "Diagnostics" / "WP12_region01_fbx_animation_compat.json"


def bounds_at(mesh_object, frame):
    scene = bpy.context.scene
    scene.frame_set(frame)
    evaluated = mesh_object.evaluated_get(bpy.context.evaluated_depsgraph_get())
    mesh = evaluated.to_mesh()
    try:
        coordinates = [evaluated.matrix_world @ vertex.co for vertex in mesh.vertices]
        return {
            "frame": frame,
            "vertex_count": len(coordinates),
            "min": [min(point[index] for point in coordinates) for index in range(3)],
            "max": [max(point[index] for point in coordinates) for index in range(3)],
        }
    finally:
        evaluated.to_mesh_clear()


def main():
    if not MESH_SOURCE.is_file() or not WALK_SOURCE.is_file():
        raise RuntimeError("Missing supplied mesh or walk source")
    bpy.ops.wm.read_factory_settings(use_empty=True)
    bpy.ops.import_scene.fbx(filepath=str(MESH_SOURCE), use_anim=False)
    mesh_object = next(object_ for object_ in bpy.context.scene.objects if object_.type == "MESH")
    mesh_armature = next(modifier.object for modifier in mesh_object.modifiers if modifier.type == "ARMATURE")
    source_armatures = set(object_.name for object_ in bpy.context.scene.objects if object_.type == "ARMATURE")

    bpy.ops.import_scene.fbx(filepath=str(WALK_SOURCE), use_anim=True)
    imported_armatures = [
        object_ for object_ in bpy.context.scene.objects
        if object_.type == "ARMATURE" and object_.name not in source_armatures
    ]
    if not imported_armatures:
        raise RuntimeError("Walk FBX did not produce an armature")
    walk_armature = imported_armatures[0]
    if not walk_armature.animation_data or not walk_armature.animation_data.action:
        raise RuntimeError("Walk FBX did not produce an action")
    action = walk_armature.animation_data.action
    frame_start, frame_end = (int(value) for value in action.frame_range)
    midpoint = frame_start + max(1, (frame_end - frame_start) // 2)

    mesh_armature.animation_data_create()
    mesh_armature.animation_data.action = action
    mesh_bones = {bone.name for bone in mesh_armature.data.bones}
    walk_bones = {bone.name for bone in walk_armature.data.bones}
    report = {
        "kind": "read_only_blender_supplied_mesh_walk_compatibility_check_not_unreal_runtime_proof",
        "mesh_source": str(MESH_SOURCE),
        "walk_source": str(WALK_SOURCE),
        "mesh_armature": mesh_armature.name,
        "walk_armature": walk_armature.name,
        "action": action.name,
        "action_frame_range": [frame_start, frame_end],
        "mesh_bone_count": len(mesh_bones),
        "walk_bone_count": len(walk_bones),
        "mesh_only_bones": sorted(mesh_bones - walk_bones),
        "walk_only_bones": sorted(walk_bones - mesh_bones),
        "evaluated_bounds": [bounds_at(mesh_object, frame_start), bounds_at(mesh_object, midpoint)],
    }
    OUTPUT.parent.mkdir(parents=True, exist_ok=True)
    OUTPUT.write_text(json.dumps(report, indent=2), encoding="utf-8")
    print("[WP12_FBX_ANIMATION_COMPAT] wrote {}".format(OUTPUT))


if __name__ == "__main__":
    main()
