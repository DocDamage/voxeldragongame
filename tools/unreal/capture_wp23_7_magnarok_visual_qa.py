"""Render the selected Magnarok voxel candidate in its supplied walk and run clips."""

import hashlib
import json
import time
import traceback
from pathlib import Path

import unreal


ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
OUT = ROOT / "Saved/Diagnostics/WP23_7_MagnarokIntake/UnrealVisualQA"
REPORT_PATH = ROOT / "Saved/Diagnostics/WP23_7_MagnarokIntake/unreal_visual_qa.json"
BASE = "/Game/WYRMFALL/Development/Intake/WP23_7/Selected/Magnarok"
MESH_PATH = BASE + "/Mesh/SK_Magnarok_VoxelFine.SK_Magnarok_VoxelFine"
WALK_PATH = BASE + "/Animations/A_Magnarok_Walk.A_Magnarok_Walk"
RUN_PATH = BASE + "/Animations/A_Magnarok_Run.A_Magnarok_Run"

ASSETS = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
ACTORS = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
REPORT = {
    "kind": "wp23_7_magnarok_unreal_visual_qa",
    "engine": unreal.SystemLibrary.get_engine_version(),
    "scope": "unsaved presentation fixture for the selected rigged voxel candidate and supplied walk/run only; no boss combat, gameplay, map, travel, facts, Echo, save, or regional completion proof",
    "capture_status": "INITIALIZING",
    "qa_status": "UNASSESSED",
    "poses": [],
    "captures": [],
}


def write_report():
    REPORT_PATH.parent.mkdir(parents=True, exist_ok=True)
    REPORT_PATH.write_text(json.dumps(REPORT, indent=2, default=str) + "\n", encoding="utf-8")


def sha256(path):
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest().upper()


def load(path, expected):
    asset = ASSETS.load_asset(path)
    if asset is None or not isinstance(asset, expected):
        raise RuntimeError("Missing or wrong-class asset: " + path)
    return asset


def spawn_pose(label, animation_path, x, phase):
    mesh = load(MESH_PATH, unreal.SkeletalMesh)
    animation = load(animation_path, unreal.AnimSequence)
    actor = ACTORS.spawn_actor_from_class(unreal.SkeletalMeshActor, unreal.Vector())
    actor.set_actor_label(label)
    component = actor.get_component_by_class(unreal.SkeletalMeshComponent)
    component.set_skinned_asset_and_update(mesh)
    component.set_animation_mode(unreal.AnimationMode.ANIMATION_SINGLE_NODE)
    component.set_animation(animation)
    component.set_update_animation_in_editor(True)
    component.play(True)
    component.set_position(animation.get_play_length() * phase, False)
    origin, extent = actor.get_actor_bounds(False)
    actor.set_actor_location(unreal.Vector(x - origin.x, -origin.y, -(origin.z - extent.z)), False, False)
    origin, extent = actor.get_actor_bounds(False)
    REPORT["poses"].append({
        "label": label,
        "mesh": mesh.get_path_name(),
        "animation": animation.get_path_name(),
        "phase": phase,
        "bounds_origin": list(origin.to_tuple()),
        "bounds_extent": list(extent.to_tuple()),
        "materials": [component.get_material(index).get_path_name() if component.get_material(index) else None for index in range(component.get_num_materials())],
    })


def add_stage():
    cube = load("/Engine/BasicShapes/Cube.Cube", unreal.StaticMesh)
    floor = ACTORS.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(0.0, 0.0, -10.0))
    floor.get_component_by_class(unreal.StaticMeshComponent).set_static_mesh(cube)
    floor.set_actor_scale3d(unreal.Vector(8.0, 5.0, 0.1))
    for rotation, intensity in (((-38.0, -35.0, 0.0), 2.0), ((-25.0, 145.0, 0.0), 0.75)):
        light = ACTORS.spawn_actor_from_class(unreal.DirectionalLight, unreal.Vector(0.0, 0.0, 500.0), unreal.Rotator(*rotation))
        component = light.get_component_by_class(unreal.DirectionalLightComponent)
        component.set_mobility(unreal.ComponentMobility.MOVABLE)
        component.set_intensity(intensity)
    sky = ACTORS.spawn_actor_from_class(unreal.SkyLight, unreal.Vector(0.0, 0.0, 500.0))
    sky_component = sky.get_component_by_class(unreal.SkyLightComponent)
    sky_component.set_mobility(unreal.ComponentMobility.MOVABLE)
    sky_component.set_intensity(0.35)
    for location, intensity, color in (
        ((-260.0, -350.0, 260.0), 1800.0, (1.0, 0.55, 0.25, 1.0)),
        ((260.0, -300.0, 260.0), 1800.0, (0.35, 0.55, 1.0, 1.0)),
    ):
        light = ACTORS.spawn_actor_from_class(unreal.PointLight, unreal.Vector(*location))
        component = light.get_component_by_class(unreal.PointLightComponent)
        component.set_mobility(unreal.ComponentMobility.MOVABLE)
        component.set_intensity(intensity)
        component.set_attenuation_radius(1200.0)
        component.set_light_color(unreal.LinearColor(*color))


def capture():
    actor = ACTORS.spawn_actor_from_class(unreal.SceneCapture2D, unreal.Vector(0.0, -720.0, 145.0))
    component = actor.get_component_by_class(unreal.SceneCaptureComponent2D)
    target = unreal.RenderingLibrary.create_render_target2d(actor, 1600, 1000, unreal.TextureRenderTargetFormat.RTF_RGBA8)
    component.set_editor_property("texture_target", target)
    component.set_editor_property("capture_source", unreal.SceneCaptureSource.SCS_FINAL_COLOR_LDR)
    component.set_editor_property("projection_type", unreal.CameraProjectionMode.ORTHOGRAPHIC)
    component.set_editor_property("ortho_width", 780.0)
    component.set_editor_property("capture_every_frame", True)
    actor.set_actor_rotation(unreal.MathLibrary.find_look_at_rotation(actor.get_actor_location(), unreal.Vector(0.0, 0.0, 105.0)), False)
    component.capture_scene()
    name = "magnarok_walk_vs_run.png"
    unreal.RenderingLibrary.export_render_target(component, target, OUT.as_posix(), name)
    path = OUT / name
    if not path.is_file() or path.stat().st_size <= 0:
        raise RuntimeError("Capture was not written: " + path.as_posix())
    REPORT["captures"].append({"view": "walk_vs_run", "path": path.as_posix(), "sha256": sha256(path)})


try:
    OUT.mkdir(parents=True, exist_ok=True)
    unreal.EditorLoadingAndSavingUtils.new_blank_map(False)
    add_stage()
    spawn_pose("Magnarok_Walk", WALK_PATH, -190.0, 0.47)
    spawn_pose("Magnarok_Run", RUN_PATH, 190.0, 0.42)
    REPORT["capture_status"] = "WAITING_FOR_RENDER"
    write_report()
    started = time.monotonic()

    def tick(_delta):
        try:
            if time.monotonic() - started < 30.0:
                return
            capture()
            REPORT["capture_status"] = "PASS"
            REPORT["qa_status"] = "CAPTURED_REQUIRES_VISUAL_REVIEW"
            write_report()
            unreal.unregister_slate_post_tick_callback(handle)
            unreal.SystemLibrary.quit_editor()
        except Exception:
            REPORT["capture_status"] = "ERROR"
            REPORT["error"] = traceback.format_exc()
            write_report()
            unreal.unregister_slate_post_tick_callback(handle)
            unreal.SystemLibrary.quit_editor()

    handle = unreal.register_slate_post_tick_callback(tick)
except Exception:
    REPORT["capture_status"] = "ERROR"
    REPORT["error"] = traceback.format_exc()
    write_report()
    unreal.SystemLibrary.quit_editor()
