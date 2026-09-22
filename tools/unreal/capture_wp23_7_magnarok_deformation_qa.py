"""Capture four phases of each supplied Magnarok clip from two angles.

This is an unsaved editor presentation fixture, not animation acceptance.
"""

import hashlib
import json
import time
import traceback
from pathlib import Path

import unreal


ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
OUT = ROOT / "Saved/Diagnostics/WP23_7_MagnarokIntake/DeformationQA"
RECEIPT = OUT / "deformation_qa.json"
BASE = "/Game/WYRMFALL/Development/Intake/WP23_7/Selected/Magnarok"
MESH = BASE + "/Mesh/SK_Magnarok_VoxelFine.SK_Magnarok_VoxelFine"
CLIPS = {
    "walk": BASE + "/Animations/A_Magnarok_Walk.A_Magnarok_Walk",
    "run": BASE + "/Animations/A_Magnarok_Run.A_Magnarok_Run",
}
PHASES = (0.0, 0.25, 0.5, 0.75)
ASSETS = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
ACTORS = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
REPORT = {
    "kind": "wp23_7_magnarok_deformation_qa",
    "engine": unreal.SystemLibrary.get_engine_version(),
    "scope": "unsaved multi-phase walk/run presentation fixture; no combat animation, gameplay, collision, or regional proof",
    "status": "INITIALIZING",
    "phases": list(PHASES),
    "bone_samples": [],
    "captures": [],
}


def save_report():
    OUT.mkdir(parents=True, exist_ok=True)
    RECEIPT.write_text(json.dumps(REPORT, indent=2, default=str) + "\n", encoding="utf-8")


def load(path, kind):
    asset = ASSETS.load_asset(path)
    if asset is None or not isinstance(asset, kind):
        raise RuntimeError("Missing or wrong-class asset: " + path)
    return asset


def add_stage():
    cube = load("/Engine/BasicShapes/Cube.Cube", unreal.StaticMesh)
    floor = ACTORS.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(0, 0, -10))
    floor.get_component_by_class(unreal.StaticMeshComponent).set_static_mesh(cube)
    floor.set_actor_scale3d(unreal.Vector(15, 5, 0.1))
    for rotation, intensity in (((-38, -35, 0), 2.0), ((-25, 145, 0), 0.75)):
        light = ACTORS.spawn_actor_from_class(unreal.DirectionalLight, unreal.Vector(0, 0, 500), unreal.Rotator(*rotation))
        component = light.get_component_by_class(unreal.DirectionalLightComponent)
        component.set_mobility(unreal.ComponentMobility.MOVABLE)
        component.set_intensity(intensity)
    for location, color in (
        ((-440, -350, 260), (1.0, 0.55, 0.25, 1.0)),
        ((440, -350, 260), (0.35, 0.55, 1.0, 1.0)),
    ):
        light = ACTORS.spawn_actor_from_class(unreal.PointLight, unreal.Vector(*location))
        component = light.get_component_by_class(unreal.PointLightComponent)
        component.set_mobility(unreal.ComponentMobility.MOVABLE)
        component.set_intensity(1800)
        component.set_attenuation_radius(1400)
        component.set_light_color(unreal.LinearColor(*color))


def spawn_phases(clip_name):
    mesh = load(MESH, unreal.SkeletalMesh)
    animation = load(CLIPS[clip_name], unreal.AnimSequence)
    poses = []
    for phase, x in zip(PHASES, (-420, -140, 140, 420)):
        actor = ACTORS.spawn_actor_from_class(unreal.SkeletalMeshActor, unreal.Vector())
        actor.set_actor_label("Magnarok_{}_{}".format(clip_name, int(phase * 100)))
        component = actor.get_component_by_class(unreal.SkeletalMeshComponent)
        component.set_skinned_asset_and_update(mesh)
        component.set_editor_property("visibility_based_anim_tick_option", unreal.VisibilityBasedAnimTickOption.ALWAYS_TICK_POSE_AND_REFRESH_BONES)
        component.set_animation_mode(unreal.AnimationMode.ANIMATION_SINGLE_NODE)
        component.set_animation(animation)
        component.set_update_animation_in_editor(True)
        component.play(True)
        component.set_position(animation.get_play_length() * phase, False)
        component.set_play_rate(0.0)
        origin, extent = actor.get_actor_bounds(False)
        actor.set_actor_location(unreal.Vector(x - origin.x, -origin.y, -(origin.z - extent.z)), False, False)
        if hasattr(component, "get_bone_location"):
            left_foot = component.get_bone_location("LeftFoot") - actor.get_actor_location()
            right_foot = component.get_bone_location("RightFoot") - actor.get_actor_location()
            REPORT["bone_samples"].append({
                "clip": clip_name,
                "phase": phase,
                "left_foot_local": list(left_foot.to_tuple()),
                "right_foot_local": list(right_foot.to_tuple()),
            })
        poses.append(actor)
    return poses


def capture(camera, target, name):
    component = camera.get_component_by_class(unreal.SceneCaptureComponent2D)
    component.capture_scene()
    unreal.RenderingLibrary.export_render_target(component, target, OUT.as_posix(), name)
    path = OUT / name
    if not path.is_file() or path.stat().st_size == 0:
        raise RuntimeError("Capture missing: " + str(path))
    REPORT["captures"].append({
        "name": name,
        "path": path.as_posix(),
        "sha256": hashlib.sha256(path.read_bytes()).hexdigest().upper(),
    })


try:
    unreal.EditorLoadingAndSavingUtils.new_blank_map(False)
    add_stage()
    camera = ACTORS.spawn_actor_from_class(unreal.SceneCapture2D, unreal.Vector(0, -900, 145))
    component = camera.get_component_by_class(unreal.SceneCaptureComponent2D)
    target = unreal.RenderingLibrary.create_render_target2d(camera, 2400, 800, unreal.TextureRenderTargetFormat.RTF_RGBA8)
    component.set_editor_property("texture_target", target)
    component.set_editor_property("capture_source", unreal.SceneCaptureSource.SCS_FINAL_COLOR_LDR)
    component.set_editor_property("projection_type", unreal.CameraProjectionMode.ORTHOGRAPHIC)
    component.set_editor_property("ortho_width", 1200.0)
    component.set_editor_property("capture_every_frame", True)
    camera.set_actor_rotation(unreal.MathLibrary.find_look_at_rotation(camera.get_actor_location(), unreal.Vector(0, 0, 105)), False)
    REPORT["status"] = "WAITING_FOR_RENDER"
    save_report()
    started = time.monotonic()
    state = {"clip": 0, "view": 0, "poses": [], "last": started}

    def tick(_delta):
        try:
            if time.monotonic() - state["last"] < 5.0:
                return
            clip_name = tuple(CLIPS)[state["clip"]]
            if not state["poses"]:
                state["poses"] = spawn_phases(clip_name)
                state["last"] = time.monotonic()
                return
            if state["view"] == 0:
                capture(camera, target, clip_name + "_front.png")
                for pose in state["poses"]:
                    pose.set_actor_rotation(unreal.Rotator(0, 65, 0), False)
                state["view"] = 1
                state["last"] = time.monotonic()
                return
            capture(camera, target, clip_name + "_oblique.png")
            for pose in state["poses"]:
                ACTORS.destroy_actor(pose)
            state["poses"] = []
            state["view"] = 0
            state["clip"] += 1
            if state["clip"] == len(CLIPS):
                REPORT["status"] = "CAPTURED_REQUIRES_VISUAL_REVIEW"
                save_report()
                unreal.unregister_slate_post_tick_callback(handle)
                unreal.SystemLibrary.quit_editor()
            else:
                state["last"] = time.monotonic()
        except Exception:
            REPORT["status"] = "ERROR"
            REPORT["error"] = traceback.format_exc()
            save_report()
            unreal.unregister_slate_post_tick_callback(handle)
            unreal.SystemLibrary.quit_editor()

    handle = unreal.register_slate_post_tick_callback(tick)
except Exception:
    REPORT["status"] = "ERROR"
    REPORT["error"] = traceback.format_exc()
    save_report()
    unreal.SystemLibrary.quit_editor()
