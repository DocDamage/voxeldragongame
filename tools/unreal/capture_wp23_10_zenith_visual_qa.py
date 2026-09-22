"""Capture an unsaved WP-23.10 Mecha Dragon, robot, and colony QA fixture."""

import hashlib
import json
from pathlib import Path
import time
import traceback

import unreal


ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
OUT = ROOT / "Saved/Diagnostics/WP23_10_ZenithVisualQA"
REPORT_PATH = ROOT / "Saved/Diagnostics/WP23_10_zenith_visual_qa.json"
DEST = "/Game/WYRMFALL/Development/Intake/WP23_10"
DRAGON_BASE = DEST + "/MechaDragon/Mecha_Dragon/SkeletalMeshes"

ASSETS = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
ACTORS = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
REPORT = {
    "kind": "wp23_10_zenith_visual_qa",
    "engine": unreal.SystemLibrary.get_engine_version(),
    "scope": "unsaved rendered intake fixture only; no production map, profile, colony, gameplay, travel, fact, Echo, or save acceptance",
    "capture_status": "INITIALIZING", "qa_status": "UNASSESSED",
    "mecha_dragon": {}, "robots": [], "colony": [], "captures": [],
}


def write_report():
    REPORT_PATH.parent.mkdir(parents=True, exist_ok=True)
    REPORT_PATH.write_text(json.dumps(REPORT, indent=2, default=str) + "\n", encoding="utf-8")


def sha256(path):
    value = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            value.update(chunk)
    return value.hexdigest()


def load(path, expected):
    asset = ASSETS.load_asset(path)
    if asset is None or not isinstance(asset, expected):
        raise RuntimeError("Missing or wrong-class asset: " + path)
    return asset


def material_paths(component):
    return [component.get_material(i).get_path_name() if component.get_material(i) else None for i in range(component.get_num_materials())]


def place_on_ground(actor, x, y, scale):
    actor.set_actor_scale3d(unreal.Vector(scale, scale, scale))
    origin, extent = actor.get_actor_bounds(False)
    current = actor.get_actor_location()
    actor.set_actor_location(unreal.Vector(current.x + x - origin.x, current.y + y - origin.y, current.z - (origin.z - extent.z)), False, False)
    origin, extent = actor.get_actor_bounds(False)
    return {"origin": list(origin.to_tuple()), "extent": list(extent.to_tuple()), "dimensions_cm": [float(extent.x * 2), float(extent.y * 2), float(extent.z * 2)]}


def spawn_static(label, path, x, y, scale):
    mesh = load(path, unreal.StaticMesh)
    actor = ACTORS.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector())
    component = actor.get_component_by_class(unreal.StaticMeshComponent)
    component.set_static_mesh(mesh)
    REPORT["robots"].append({"label": label, "mesh": mesh.get_path_name(), "materials": material_paths(component), "bounds": place_on_ground(actor, x, y, scale)})


def spawn_colony(label, path, x, y, scale):
    mesh = load(path, unreal.SkeletalMesh)
    actor = ACTORS.spawn_actor_from_class(unreal.SkeletalMeshActor, unreal.Vector())
    component = actor.get_component_by_class(unreal.SkeletalMeshComponent)
    component.set_skinned_asset_and_update(mesh)
    REPORT["colony"].append({"label": label, "mesh": mesh.get_path_name(), "materials": material_paths(component), "bounds": place_on_ground(actor, x, y, scale)})


def spawn_mecha_dragon():
    meshes = [ASSETS.load_asset(path) for path in ASSETS.list_assets(DRAGON_BASE, recursive=False, include_folder=False)]
    meshes = [mesh for mesh in meshes if isinstance(mesh, unreal.SkeletalMesh)]
    if len(meshes) != 35:
        raise RuntimeError(f"Expected 35 Mecha Dragon skeletal meshes, got {len(meshes)}")
    leader = next((mesh for mesh in meshes if mesh.get_name().casefold() == "hip-local"), None)
    if leader is None:
        raise RuntimeError("Mecha Dragon Hip-Local leader is missing")
    animation = load(DRAGON_BASE + "/Mecha_DragonIdle_01.Mecha_DragonIdle_01", unreal.AnimSequence)
    actors = []
    components = []
    leader_actor = None
    leader_component = None
    for mesh in meshes:
        actor = ACTORS.spawn_actor_from_class(unreal.SkeletalMeshActor, unreal.Vector())
        component = actor.get_component_by_class(unreal.SkeletalMeshComponent)
        component.set_skinned_asset_and_update(mesh)
        component.set_animation_mode(unreal.AnimationMode.ANIMATION_SINGLE_NODE)
        component.set_animation(animation)
        component.set_update_animation_in_editor(True)
        component.play(True)
        component.set_position(animation.get_play_length() * 0.25, False)
        actor.set_actor_scale3d(unreal.Vector(0.04, 0.04, 0.04))
        actors.append(actor)
        components.append(component)
        if mesh == leader:
            leader_actor = actor
            leader_component = component
    for component in components:
        if component != leader_component:
            component.set_leader_pose_component(leader_component, True, False)
    origin, extent = leader_actor.get_actor_bounds(False)
    offset = unreal.Vector(700.0 - origin.x, 0.0 - origin.y, -(origin.z - extent.z))
    for actor in actors:
        actor.set_actor_location(actor.get_actor_location() + offset, False, False)
    origin, extent = leader_actor.get_actor_bounds(False)
    REPORT["mecha_dragon"] = {
        "leader_mesh": leader.get_path_name(), "part_count": len(meshes),
        "animation": animation.get_path_name(), "scale": 0.04,
        "materials_populated": all(material_paths(component) and all(material_paths(component)) for component in components),
        "leader_bounds": {"origin": list(origin.to_tuple()), "extent": list(extent.to_tuple())},
        "assembly": "RUNTIME_STYLE_LEADER_POSE",
    }


def add_lighting_and_floor():
    cube = load("/Engine/BasicShapes/Cube.Cube", unreal.StaticMesh)
    floor = ACTORS.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(1500.0, 500.0, -10.0))
    floor.get_component_by_class(unreal.StaticMeshComponent).set_static_mesh(cube)
    floor.set_actor_scale3d(unreal.Vector(42.0, 26.0, 0.2))
    for rotation, intensity in (((-45.0, -35.0, 0.0), 5.0), ((-25.0, 145.0, 0.0), 2.0)):
        light = ACTORS.spawn_actor_from_class(unreal.DirectionalLight, unreal.Vector(0.0, 0.0, 700.0), unreal.Rotator(*rotation))
        component = light.get_component_by_class(unreal.DirectionalLightComponent)
        component.set_mobility(unreal.ComponentMobility.MOVABLE)
        component.set_intensity(intensity)


def make_capture():
    actor = ACTORS.spawn_actor_from_class(unreal.SceneCapture2D, unreal.Vector())
    component = actor.get_component_by_class(unreal.SceneCaptureComponent2D)
    target = unreal.RenderingLibrary.create_render_target2d(actor, 1440, 1080, unreal.TextureRenderTargetFormat.RTF_RGBA8)
    component.set_editor_property("texture_target", target)
    component.set_editor_property("capture_source", unreal.SceneCaptureSource.SCS_FINAL_COLOR_LDR)
    component.set_editor_property("fov_angle", 40.0)
    component.set_editor_property("capture_every_frame", True)
    return actor, component, target


def capture(actor, component, target, name, camera, look_at):
    actor.set_actor_location(unreal.Vector(*camera), False, False)
    actor.set_actor_rotation(unreal.MathLibrary.find_look_at_rotation(unreal.Vector(*camera), unreal.Vector(*look_at)), False)
    component.capture_scene()
    unreal.RenderingLibrary.export_render_target(component, target, OUT.as_posix(), name)
    path = OUT / name
    if not path.is_file() or path.stat().st_size <= 0:
        raise RuntimeError("Capture was not written: " + path.as_posix())
    REPORT["captures"].append({"view": name.removesuffix(".png"), "path": path.as_posix(), "sha256": sha256(path)})


try:
    OUT.mkdir(parents=True, exist_ok=True)
    unreal.EditorLoadingAndSavingUtils.new_blank_map(False)
    add_lighting_and_floor()
    spawn_mecha_dragon()
    for column, index in enumerate((0, 10, 20, 30, 40, 50, 60, 70, 78)):
        stem = f"Robot_vox-{index}"
        spawn_static(stem, f"{DEST}/Robots/{stem}/{stem}.{stem}", 1450.0 + (column % 5) * 230.0, (column // 5) * 300.0, 120.0)
    colony = (
        ("BuildingBlock_2", 1800.0, 1050.0, 1.2), ("GroundTile_008", 2300.0, 1050.0, 1.2),
        ("Lander", 2800.0, 1050.0, 1.2), ("Prop_14", 1800.0, 1550.0, 1.2),
        ("SatelliteDish_1", 2300.0, 1550.0, 1.2), ("SolarPanel_4", 2800.0, 1550.0, 1.2),
    )
    for label, x, y, scale in colony:
        spawn_colony(label, f"{DEST}/Colony/{label}/{label}.{label}", x, y, scale)
    capture_actor, capture_component, target = make_capture()
    REPORT["capture_status"] = "WAITING_FOR_RENDER"
    write_report()
    start = time.monotonic()
    stage = 0
    views = [
        ("01_mecha_dragon_assembly.png", (1250.0, -1150.0, 500.0), (700.0, 0.0, 160.0)),
        ("02_robot_parts.png", (2050.0, -850.0, 480.0), (1900.0, 160.0, 120.0)),
        ("03_colony_exterior_sample.png", (2450.0, -500.0, 900.0), (2300.0, 1300.0, 150.0)),
    ]

    def tick(_delta):
        global stage
        try:
            if time.monotonic() - start < 25.0 + stage * 2.0:
                return
            if stage < len(views):
                capture(capture_actor, capture_component, target, *views[stage])
                stage += 1
                write_report()
                return
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
