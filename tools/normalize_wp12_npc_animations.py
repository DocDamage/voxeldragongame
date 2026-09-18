"""Normalize selected supplied WP-12 NPC FBXs through Blender.

The direct UE 5.8 mesh route retained fewer bones than the matching supplied
walk source. This tool exports each selected mesh and its own idle/walk actions
from the same source armature, yielding local intake-only FBX derivatives with
a common, single-root hierarchy for an Unreal compatibility probe. It never
edits the source FBXs and writes only under Saved/AssetIntake and Saved/
Diagnostics.
"""

import hashlib
import json
from pathlib import Path

import bpy


ROOT = Path(__file__).resolve().parent.parent
SOURCE_ROOT = ROOT / "Saved" / "AssetIntake" / "WP12" / "NPC"
OUTPUT_ROOT = SOURCE_ROOT / "Normalized"
RECEIPT = ROOT / "Saved" / "Diagnostics" / "WP12_region01_normalized_npc_sources.json"


ROLES = (
    {
        "role": "Tamsin",
        "mesh": "Voxel Rangers/FBX/Character/TVS_VoxelRangers_Captain.fbx",
        "idle": "Voxel Rangers/Animations/Human_Idle_Anim.fbx",
        "walk": "Voxel Rangers/Animations/Human_Walk_Anim.fbx",
    },
    {
        "role": "Mara",
        "mesh": "Voxel Village/FBX/Characters/TVS_VoxelVillage_MarketWoman.fbx",
        "idle": "Voxel Village/Animations/Human_Idle_Anim.fbx",
        "walk": "Voxel Village/Animations/Human_Walk_Anim.fbx",
    },
    {
        "role": "Sella",
        "mesh": "Voxel Farm/FBX/Characters/TVS_VoxelFarm_FarmersDaughter.fbx",
        "idle": "Voxel Farm/Animations/Human/Human_Idle_Anim.fbx",
        "walk": "Voxel Farm/Animations/Human/Human_Walk_Anim.fbx",
    },
    {
        "role": "Pell",
        "mesh": "Voxel Farm/FBX/Characters/TVS_VoxelFarm_FarmHand.fbx",
        "idle": "Voxel Farm/Animations/Human/Human_Idle_Anim.fbx",
        "walk": "Voxel Farm/Animations/Human/Human_Walk_Anim.fbx",
    },
    {
        "role": "Iven",
        "mesh": "Voxel Village/FBX/Characters/TVS_VoxelVillage_YoungMan.fbx",
        "idle": "Voxel Village/Animations/Human_Idle_Anim.fbx",
        "walk": "Voxel Village/Animations/Human_Walk_Anim.fbx",
    },
    {
        "role": "Rusk",
        "mesh": "Voxel Knights/FBX/Characters/TVS_VoxelKnights_Commander.fbx",
        "idle": "Voxel Knights/Animations/Humans/Human_Idle_Anim.fbx",
        "walk": "Voxel Knights/Animations/Humans/Human_Walk_Anim.fbx",
    },
)


def sha256(path):
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for chunk in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def deselect_all():
    bpy.ops.object.select_all(action="DESELECT")


def ensure_single_root(armature):
    """Add an identity parent for a source skeleton with several roots."""
    deselect_all()
    armature.select_set(True)
    bpy.context.view_layer.objects.active = armature
    bpy.ops.object.mode_set(mode="EDIT")
    roots = [bone for bone in armature.data.edit_bones if bone.parent is None]
    source_root_count = len(roots)
    if source_root_count > 1:
        root = armature.data.edit_bones.new("Wyrmfall_NormalizedRoot")
        root.head = (0.0, 0.0, 0.0)
        root.tail = (0.0, 0.0, 1.0)
        for bone in roots:
            bone.parent = root
            bone.use_connect = False
    bpy.ops.object.mode_set(mode="OBJECT")
    return {"source_root_bone_count": source_root_count, "normalized_bone_count": len(armature.data.bones)}


def export_mesh(mesh_object, armature, output):
    deselect_all()
    mesh_object.select_set(True)
    armature.select_set(True)
    bpy.context.view_layer.objects.active = armature
    bpy.ops.export_scene.fbx(
        filepath=str(output),
        use_selection=True,
        object_types={"ARMATURE", "MESH"},
        add_leaf_bones=False,
        bake_anim=False,
    )


def imported_action_armature(existing_armatures):
    candidates = [
        object_ for object_ in bpy.context.scene.objects
        if object_.type == "ARMATURE" and object_.name not in existing_armatures
    ]
    if len(candidates) != 1:
        raise RuntimeError("Expected exactly one imported animation armature, found {}".format(len(candidates)))
    armature = candidates[0]
    if not armature.animation_data or not armature.animation_data.action:
        raise RuntimeError("Imported animation armature has no action: " + armature.name)
    return armature


def export_action(mesh_armature, source, output):
    existing_armatures = {object_.name for object_ in bpy.context.scene.objects if object_.type == "ARMATURE"}
    bpy.ops.import_scene.fbx(filepath=str(source), use_anim=True)
    source_armature = imported_action_armature(existing_armatures)
    action = source_armature.animation_data.action
    mesh_armature.animation_data_create()
    mesh_armature.animation_data.action = action
    if hasattr(action, "slots") and len(action.slots) > 0 and hasattr(mesh_armature.animation_data, "action_slot"):
        mesh_armature.animation_data.action_slot = action.slots[0]
    # FBX baking otherwise follows Blender's default 1..250 scene range,
    # turning the supplied 60/41-frame clips into 10.375-second sequences
    # with a frozen tail. Export only the active supplied action's range.
    frame_start, frame_end = action.frame_range
    bpy.context.scene.frame_start = int(frame_start)
    bpy.context.scene.frame_end = int(frame_end)
    bpy.context.scene.frame_set(int(frame_start))

    deselect_all()
    mesh_armature.select_set(True)
    bpy.context.view_layer.objects.active = mesh_armature
    bpy.ops.export_scene.fbx(
        filepath=str(output),
        use_selection=True,
        object_types={"ARMATURE"},
        add_leaf_bones=False,
        bake_anim=True,
        bake_anim_use_all_bones=True,
        bake_anim_force_startend_keying=True,
        bake_anim_use_nla_strips=False,
        bake_anim_use_all_actions=False,
        bake_anim_simplify_factor=0.0,
    )
    bpy.data.objects.remove(source_armature, do_unlink=True)
    return {"action": action.name, "frame_range": [float(value) for value in action.frame_range]}


def normalize(candidate):
    sources = {kind: SOURCE_ROOT / candidate[kind] for kind in ("mesh", "idle", "walk")}
    for source in sources.values():
        if not source.is_file():
            raise RuntimeError("Missing supplied source: {}".format(source))
    destination = OUTPUT_ROOT / candidate["role"]
    destination.mkdir(parents=True, exist_ok=True)
    outputs = {
        "mesh": destination / "SK_{}.fbx".format(candidate["role"]),
        "idle": destination / "A_{}_Idle.fbx".format(candidate["role"]),
        "walk": destination / "A_{}_Walk.fbx".format(candidate["role"]),
    }

    bpy.ops.wm.read_factory_settings(use_empty=True)
    bpy.ops.import_scene.fbx(filepath=str(sources["mesh"]), use_anim=False)
    mesh_object = next(object_ for object_ in bpy.context.scene.objects if object_.type == "MESH")
    armature = next(modifier.object for modifier in mesh_object.modifiers if modifier.type == "ARMATURE")
    source_armature_bone_count = len(armature.data.bones)
    root_normalization = ensure_single_root(armature)
    export_mesh(mesh_object, armature, outputs["mesh"])
    animations = {
        kind: export_action(armature, sources[kind], outputs[kind])
        for kind in ("idle", "walk")
    }
    return {
        "role": candidate["role"],
        "source_files": {
            kind: {"path": str(path), "sha256": sha256(path)} for kind, path in sources.items()
        },
        "source_armature_bone_count": source_armature_bone_count,
        "root_normalization": root_normalization,
        "normalized_files": {
            kind: {"path": str(path), "sha256": sha256(path)} for kind, path in outputs.items()
        },
        "animations": animations,
    }


def main():
    receipt = {
        "kind": "supplied_npc_blender_normalization_for_ue_animation_compatibility_not_production_content",
        "roles": [normalize(candidate) for candidate in ROLES],
    }
    RECEIPT.parent.mkdir(parents=True, exist_ok=True)
    RECEIPT.write_text(json.dumps(receipt, indent=2), encoding="utf-8")
    print("[WP12_NPC_NORMALIZATION] wrote {}".format(RECEIPT))


if __name__ == "__main__":
    main()
