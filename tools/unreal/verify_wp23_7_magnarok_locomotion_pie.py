"""Prove selected Magnarok rig evaluates supplied locomotion in live PIE."""

import hashlib
import json
import math
import time
import traceback
from pathlib import Path

import unreal


ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
OUT = ROOT / "Saved/Diagnostics/WP23_7_MagnarokIntake/locomotion_pie.json"
BASE = "/Game/WYRMFALL/Development/Intake/WP23_7/Selected/Magnarok"
MESH = BASE + "/Mesh/SK_Magnarok_VoxelFine.SK_Magnarok_VoxelFine"
CLIPS = {
    "Walk": BASE + "/Animations/A_Magnarok_Walk.A_Magnarok_Walk",
    "Run": BASE + "/Animations/A_Magnarok_Run.A_Magnarok_Run",
}
ASSETS = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
ACTORS = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
LEVELS = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
EDITOR = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
REPORT = {
    "kind": "wp23_7_magnarok_locomotion_pie",
    "engine": unreal.SystemLibrary.get_engine_version(),
    "scope": "unsaved blank-map live PIE skeletal playback; no gameplay collision, boss performance, or regional proof",
    "status": "INITIALIZING",
    "samples": {name.lower(): [] for name in CLIPS},
}


def save():
    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_text(json.dumps(REPORT, indent=2, default=str) + "\n", encoding="utf-8")


def load(path, kind):
    asset = ASSETS.load_asset(path)
    if asset is None or not isinstance(asset, kind):
        raise RuntimeError("Missing or wrong-class asset: " + path)
    return asset


def sample(actor, elapsed):
    component = actor.get_component_by_class(unreal.SkeletalMeshComponent)
    bones = {}
    for name in ("Hips", "LeftFoot", "RightFoot", "LeftArm", "RightArm", "Head"):
        pose = component.get_bone_transform(name)
        bones[name] = {
            "translation": list(pose.translation.to_tuple()),
            "rotation": list(pose.rotation.to_tuple()),
        }
    return {"elapsed_seconds": elapsed, "bones": bones}


def add_stage():
    cube = load("/Engine/BasicShapes/Cube.Cube", unreal.StaticMesh)
    floor = ACTORS.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(150, 0, -10))
    floor.get_component_by_class(unreal.StaticMeshComponent).set_static_mesh(cube)
    floor.set_actor_scale3d(unreal.Vector(8, 5, 0.1))
    for rotation, intensity in (((-38, -35, 0), 2.0), ((-25, 145, 0), 0.75)):
        light = ACTORS.spawn_actor_from_class(unreal.DirectionalLight, unreal.Vector(150, 0, 500), unreal.Rotator(*rotation))
        component = light.get_component_by_class(unreal.DirectionalLightComponent)
        component.set_mobility(unreal.ComponentMobility.MOVABLE)
        component.set_intensity(intensity)
    for location, color in (
        ((-150, -350, 260), (1.0, 0.55, 0.25, 1.0)),
        ((450, -350, 260), (0.35, 0.55, 1.0, 1.0)),
    ):
        light = ACTORS.spawn_actor_from_class(unreal.PointLight, unreal.Vector(*location))
        component = light.get_component_by_class(unreal.PointLightComponent)
        component.set_mobility(unreal.ComponentMobility.MOVABLE)
        component.set_intensity(1800)
        component.set_attenuation_radius(1200)
        component.set_light_color(unreal.LinearColor(*color))
    camera = ACTORS.spawn_actor_from_class(unreal.CameraActor, unreal.Vector(150, -540, 145))
    camera.set_actor_label("MagnarokPIE_Camera")
    camera.get_component_by_class(unreal.CameraComponent).set_editor_property("field_of_view", 65.0)
    camera.set_actor_rotation(unreal.MathLibrary.find_look_at_rotation(camera.get_actor_location(), unreal.Vector(150, 0, 105)), False)


try:
    unreal.EditorLoadingAndSavingUtils.new_blank_map(False)
    mesh = load(MESH, unreal.SkeletalMesh)
    animations = {name: load(path, unreal.AnimSequence) for name, path in CLIPS.items()}
    add_stage()
    for index, (name, path) in enumerate(CLIPS.items()):
        animation = animations[name]
        actor = ACTORS.spawn_actor_from_class(unreal.SkeletalMeshActor, unreal.Vector(index * 300, 0, 0))
        actor.set_actor_label("MagnarokPIE_" + name)
        component = actor.get_component_by_class(unreal.SkeletalMeshComponent)
        component.set_skinned_asset_and_update(mesh)
        component.set_editor_property("visibility_based_anim_tick_option", unreal.VisibilityBasedAnimTickOption.ALWAYS_TICK_POSE_AND_REFRESH_BONES)
        component.set_animation_mode(unreal.AnimationMode.ANIMATION_SINGLE_NODE)
        component.set_animation(animation)
        component.play(True)
    REPORT["status"] = "WAITING_FOR_PIE"
    save()
    started = time.monotonic()
    launched_wall = time.time()
    state = {"first": None, "next": 0.0, "count": 0, "finished": False, "configured": False, "capture_count": 0, "bone_pass": False}

    def tick(_delta):
        try:
            if state["finished"]:
                if not LEVELS.is_in_play_in_editor():
                    unreal.unregister_slate_post_tick_callback(handle)
                    unreal.SystemLibrary.quit_editor()
                return
            if time.monotonic() - started > 50.0:
                raise RuntimeError("Timed out waiting for PIE playback samples")
            world = EDITOR.get_game_world()
            if not world or not LEVELS.is_in_play_in_editor():
                return
            by_label = {
                actor.get_actor_label(): actor
                for actor in unreal.GameplayStatics.get_all_actors_of_class(world, unreal.SkeletalMeshActor)
            }
            if any("MagnarokPIE_" + name not in by_label for name in CLIPS):
                return
            if not state["configured"]:
                REPORT["runtime_setup"] = {}
                for name in CLIPS:
                    component = by_label["MagnarokPIE_" + name].get_component_by_class(unreal.SkeletalMeshComponent)
                    component.set_editor_property("visibility_based_anim_tick_option", unreal.VisibilityBasedAnimTickOption.ALWAYS_TICK_POSE_AND_REFRESH_BONES)
                    component.play_animation(animations[name], True)
                    REPORT["runtime_setup"][name.lower()] = {
                        "animation_mode": str(component.get_animation_mode()),
                        "has_valid_animation_instance": bool(component.has_valid_animation_instance()),
                        "tick_enabled": bool(component.is_component_tick_enabled()),
                    }
                state["configured"] = True
                cameras = unreal.GameplayStatics.get_all_actors_of_class(world, unreal.CameraActor)
                camera = next((actor for actor in cameras if actor.get_actor_label() == "MagnarokPIE_Camera"), None)
                if camera:
                    controller = unreal.GameplayStatics.get_player_controller(world, 0)
                    if controller:
                        controller.set_view_target_with_blend(camera, 0.0)
                return
            if state["first"] is None:
                state["first"] = time.monotonic()
            elapsed = time.monotonic() - state["first"]
            if elapsed < state["next"]:
                return
            for name in CLIPS:
                REPORT["samples"][name.lower()].append(sample(by_label["MagnarokPIE_" + name], elapsed))
            state["count"] += 1
            state["next"] += 0.15
            if state["count"] in (3, 7):
                cameras = unreal.GameplayStatics.get_all_actors_of_class(world, unreal.CameraActor)
                camera = next((actor for actor in cameras if actor.get_actor_label() == "MagnarokPIE_Camera"), None)
                if camera:
                    image = OUT.parent / ("locomotion_live_early.png" if state["count"] == 3 else "locomotion_live_late.png")
                    state["capture_count"] += 1
                    REPORT.setdefault("screenshots", []).append({"path": str(image), "requested": bool(unreal.AutomationLibrary.take_high_res_screenshot(1600, 900, str(image), camera))})
            if state["count"] >= 8 and not state["bone_pass"]:
                REPORT["max_bone_displacement_cm"] = {}
                for name in CLIPS:
                    samples = REPORT["samples"][name.lower()]
                    metrics = {}
                    for bone in ("LeftFoot", "RightFoot", "LeftArm", "RightArm"):
                        reference = samples[0]["bones"][bone]["translation"]
                        metrics[bone] = max(math.dist(reference, item["bones"][bone]["translation"]) for item in samples)
                    REPORT["max_bone_displacement_cm"][name.lower()] = metrics
                REPORT["status"] = (
                    "PASS_LIVE_PIE_BONE_DEFORMATION"
                    if all(REPORT["max_bone_displacement_cm"][name.lower()][bone] > 10.0
                           for name in CLIPS for bone in ("LeftFoot", "RightFoot"))
                    else "FAIL_NO_MEANINGFUL_BONE_DEFORMATION"
                )
                state["bone_pass"] = True
                state["last_sample"] = time.monotonic()
                save()
            if state["bone_pass"] and time.monotonic() - state["last_sample"] > 2.0:
                for item in REPORT.get("screenshots", []):
                    path = Path(item["path"])
                    item["fresh"] = path.is_file() and path.stat().st_mtime >= launched_wall
                    if item["fresh"]:
                        item["bytes"] = path.stat().st_size
                        item["sha256"] = hashlib.sha256(path.read_bytes()).hexdigest().upper()
                REPORT["screenshot_status"] = "CAPTURED" if state["capture_count"] == 2 and all(item.get("requested") and item.get("fresh") and item.get("bytes", 0) > 0 for item in REPORT.get("screenshots", [])) else "INCOMPLETE"
                save()
                state["finished"] = True
                LEVELS.editor_request_end_play()
        except Exception:
            REPORT["status"] = "ERROR"
            REPORT["error"] = traceback.format_exc()
            save()
            state["finished"] = True
            if LEVELS.is_in_play_in_editor():
                LEVELS.editor_request_end_play()
            else:
                unreal.unregister_slate_post_tick_callback(handle)
                unreal.SystemLibrary.quit_editor()

    handle = unreal.register_slate_post_tick_callback(tick)
    LEVELS.editor_play_simulate()
except Exception:
    REPORT["status"] = "ERROR"
    REPORT["error"] = traceback.format_exc()
    save()
    unreal.SystemLibrary.quit_editor()
