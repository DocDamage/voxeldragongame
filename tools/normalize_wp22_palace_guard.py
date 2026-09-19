"""Normalize the supplied Palace Guard FBX to a single Unreal-compatible root."""

from pathlib import Path
import json
import hashlib

import bpy

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "Saved/Staging/WP22/Palace/TVS_VoxelPalace_Guard.fbx"
OUTPUT = ROOT / "Saved/Staging/WP22/Palace/TVS_VoxelPalace_Guard_Normalized.fbx"
RECEIPT = ROOT / "Saved/Diagnostics/WP22_palace_guard_normalization.json"


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    if not SOURCE.is_file():
        raise RuntimeError(f"Missing extracted supplied guard: {SOURCE}")
    bpy.ops.wm.read_factory_settings(use_empty=True)
    bpy.ops.import_scene.fbx(filepath=str(SOURCE), use_anim=False)
    armatures = [obj for obj in bpy.context.scene.objects if obj.type == "ARMATURE"]
    meshes = [obj for obj in bpy.context.scene.objects if obj.type == "MESH"]
    if len(armatures) != 1 or not meshes:
        raise RuntimeError(f"Expected one armature and at least one mesh; got {len(armatures)} and {len(meshes)}")
    armature = armatures[0]
    bpy.context.view_layer.objects.active = armature
    armature.select_set(True)
    bpy.ops.object.mode_set(mode="EDIT")
    roots = [bone for bone in armature.data.edit_bones if bone.parent is None]
    source_roots = len(roots)
    if source_roots > 1:
        root = armature.data.edit_bones.new("Wyrmfall_NormalizedRoot")
        root.head = (0.0, 0.0, 0.0)
        root.tail = (0.0, 0.0, 1.0)
        for bone in roots:
            bone.parent = root
            bone.use_connect = False
    bpy.ops.object.mode_set(mode="OBJECT")
    bpy.ops.object.select_all(action="DESELECT")
    armature.select_set(True)
    for mesh in meshes:
        mesh.select_set(True)
    bpy.context.view_layer.objects.active = armature
    bpy.ops.export_scene.fbx(
        filepath=str(OUTPUT), use_selection=True,
        object_types={"ARMATURE", "MESH"}, add_leaf_bones=False, bake_anim=False)
    RECEIPT.parent.mkdir(parents=True, exist_ok=True)
    RECEIPT.write_text(json.dumps({
        "status": "PASS", "source": str(SOURCE), "source_sha256": digest(SOURCE),
        "output": str(OUTPUT), "output_sha256": digest(OUTPUT),
        "source_root_count": source_roots, "normalized_bone_count": len(armature.data.bones),
    }, indent=2), encoding="utf-8")


if __name__ == "__main__":
    main()
