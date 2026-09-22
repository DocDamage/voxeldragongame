"""Measure three owned Quaternius idle-source loops; no retarget or export."""

import hashlib
import json
import math
from pathlib import Path

import bpy


ROOT = Path(__file__).resolve().parents[2]
SOURCE = Path(r"G:\3d assets\Universal Animation Library[Standard]\Universal Animation Library[Standard]\Unreal-Godot\UAL1_Standard.glb")
OUT = ROOT / "Saved/Diagnostics/WP23_7_MagnarokIntake/ual1_idle_source_probe.json"
ACTIONS = ("Idle_Loop", "Sword_Idle", "Spell_Simple_Idle_Loop")
BONES = ("pelvis", "spine_01", "spine_02", "spine_03", "neck_01", "Head", "clavicle_l", "upperarm_l", "lowerarm_l", "clavicle_r", "upperarm_r", "lowerarm_r", "thigh_l", "calf_l", "foot_l", "thigh_r", "calf_r", "foot_r")


def digest(path):
    sha = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            sha.update(chunk)
    return sha.hexdigest().upper()


def sample(armature, action):
    armature.animation_data_create()
    armature.animation_data.action = action
    if action.slots:
        armature.animation_data.action_slot = next(
            (slot for slot in action.slots if slot.target_id_type == "OBJECT"),
            action.slots[0],
        )
    start, end = action.frame_range
    frames = [round(start + (end - start) * i / 8) for i in range(9)]
    poses = []
    roots = []
    for frame in frames:
        bpy.context.scene.frame_set(frame)
        bpy.context.view_layer.update()
        poses.append({name: armature.pose.bones[name].matrix.to_quaternion().copy() for name in BONES})
        roots.append(tuple(armature.pose.bones["pelvis"].matrix.translation))
    midpoint = [
        math.degrees(min(angle, 2 * math.pi - angle))
        for pose in poses[1:-1] for name in BONES
        for angle in (poses[0][name].rotation_difference(pose[name]).angle,)
    ]
    return {
        "action": action.name,
        "frame_range": [start, end],
        "sample_frames": frames,
        "sampled_joint_rotation_from_first_mean_deg": sum(midpoint) / len(midpoint),
        "sampled_joint_rotation_from_first_max_deg": max(midpoint),
        "sampled_first_last_max_joint_gap_deg": max(
            math.degrees(min(angle, 2 * math.pi - angle))
            for name in BONES
            for angle in (poses[0][name].rotation_difference(poses[-1][name]).angle,)
        ),
        "sampled_pelvis_translation_range_source_units": [
            max(point[i] for point in roots) - min(point[i] for point in roots)
            for i in range(3)
        ],
    }


def main():
    OUT.parent.mkdir(parents=True, exist_ok=True)
    report = {
        "kind": "wp23_7_ual1_idle_source_probe",
        "scope": "source-armature motion only; no retarget, render, animation selection, export, or UE import",
        "source": str(SOURCE),
        "sha256": digest(SOURCE),
        "blender": bpy.app.version_string,
    }
    try:
        bpy.ops.wm.read_factory_settings(use_empty=True)
        bpy.ops.import_scene.gltf(filepath=str(SOURCE))
        armature = next(obj for obj in bpy.context.scene.objects if obj.type == "ARMATURE")
        report["armature_bones"] = len(armature.data.bones)
        report["actions"] = [sample(armature, bpy.data.actions[name]) for name in ACTIONS]
        report["status"] = "SOURCE_MOTION_SAMPLED"
    except Exception as exc:
        report["status"] = "ERROR"
        report["error"] = repr(exc)
        raise
    finally:
        OUT.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print("UAL1 idle source probe: " + report["status"])


if __name__ == "__main__":
    main()
