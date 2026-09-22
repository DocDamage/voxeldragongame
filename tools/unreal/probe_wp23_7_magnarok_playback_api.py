"""Read-only API and imported-track probe for Magnarok playback diagnosis."""

import json
import traceback
from pathlib import Path

import unreal


ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
OUT = ROOT / "Saved/Diagnostics/WP23_7_MagnarokIntake/playback_api_probe.json"
BASE = "/Game/WYRMFALL/Development/Intake/WP23_7/Selected/Magnarok"
ASSETS = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
REPORT = {"kind": "wp23_7_magnarok_playback_api_probe", "engine": unreal.SystemLibrary.get_engine_version(), "status": "RUNNING"}


try:
    mesh = ASSETS.load_asset(BASE + "/Mesh/SK_Magnarok_VoxelFine.SK_Magnarok_VoxelFine")
    component = unreal.SkeletalMeshComponent()
    component.set_skinned_asset_and_update(mesh)
    REPORT["mesh_bone_names"] = [str(component.get_bone_name(i)) for i in range(component.get_num_bones())]
    library = getattr(unreal, "AnimationLibrary", None)
    REPORT["pose_api_doc"] = getattr(library.get_bone_pose_for_time, "__doc__", None) if library else None
    REPORT["bone_transform_doc"] = getattr(component.get_bone_transform, "__doc__", None)
    REPORT["component_methods"] = [name for name in dir(component) if any(word in name.lower() for word in ("anim", "bone", "pose", "refresh", "tick", "force"))]
    REPORT["animation_library_methods"] = [name for name in dir(library) if any(word in name.lower() for word in ("track", "bone", "pose", "frame"))] if library else []
    REPORT["clips"] = {}
    for name in ("Walk", "Run"):
        animation = ASSETS.load_asset(BASE + f"/Animations/A_Magnarok_{name}.A_Magnarok_{name}")
        entry = {
            "path": animation.get_path_name(),
            "seconds": animation.get_play_length(),
            "sampled_keys": animation.get_editor_property("number_of_sampled_keys"),
            "skeleton": animation.get_editor_property("skeleton").get_path_name(),
        }
        if library and hasattr(library, "get_animation_track_names"):
            entry["track_names"] = list(library.get_animation_track_names(animation))
            entry["pose_samples"] = []
            for phase in (0.0, 0.25, 0.5, 0.75):
                sample = {"phase": phase, "bones": {}}
                for bone in ("Hips", "LeftFoot", "RightFoot", "LeftArm", "RightArm", "Head"):
                    pose = library.get_bone_pose_for_time(animation, bone, animation.get_play_length() * phase, False)
                    sample["bones"][bone] = {
                        "translation": list(pose.translation.to_tuple()),
                        "rotation": list(pose.rotation.to_tuple()),
                    }
                entry["pose_samples"].append(sample)
        REPORT["clips"][name.lower()] = entry
    REPORT["status"] = "PASS_READ_ONLY_PROBE"
except Exception:
    REPORT["status"] = "ERROR"
    REPORT["error"] = traceback.format_exc()
finally:
    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_text(json.dumps(REPORT, indent=2, default=str) + "\n", encoding="utf-8")
    unreal.SystemLibrary.quit_editor()
