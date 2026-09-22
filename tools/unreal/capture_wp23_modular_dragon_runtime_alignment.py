"""Render blocked dragon candidates through the real runtime component hierarchy."""

import hashlib
import json
from pathlib import Path
import time
import traceback

import unreal


ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
OUT = ROOT / "Saved/Diagnostics/WP23_ModularDragonRuntimeAlignment"
REPORT_PATH = ROOT / "Saved/Diagnostics/WP23_modular_dragon_runtime_alignment.json"
ASSETS = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
ACTORS = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)

ROWS = (
    {
        "label": "VerdanceControl",
        "base": "/Game/WYRMFALL/Development/Intake/WP00/GreenDragon/Green_Dragon/SkeletalMeshes",
        "animation": "Green_DragonIdle_01",
        "expected_parts": 44,
        "swap": False,
    },
    {
        "label": "FrostmaneCandidate",
        "base": "/Game/WYRMFALL/Development/Intake/WP23_4/Frostmane/White_Dragon/SkeletalMeshes",
        "animation": "White_DragonIdle_01",
        "expected_parts": 37,
        "swap": True,
    },
    {
        "label": "PyraxisCandidate",
        "base": "/Game/WYRMFALL/Development/Intake/WP23_7/Pyraxis/Lava_Dragon/SkeletalMeshes",
        "animation": "Lava_DragonIdle_01",
        "expected_parts": 35,
        "swap": True,
    },
    {
        "label": "MechaCandidate",
        "base": "/Game/WYRMFALL/Development/Intake/WP23_10/MechaDragon/Mecha_Dragon/SkeletalMeshes",
        "animation": "Mecha_DragonIdle_01",
        "expected_parts": 35,
        "swap": True,
    },
)

REPORT = {
    "kind": "wp23_modular_dragon_runtime_alignment",
    "engine": unreal.SystemLibrary.get_engine_version(),
    "scope": "unsaved diagnostic runtime-component fixture only; no profile, gameplay, travel, fact, Echo, or persistence acceptance",
    "capture_status": "INITIALIZING",
    "qa_status": "UNASSESSED",
    "assemblies": [],
    "captures": [],
}
CURRENT_ACTOR = None


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


def available_meshes(base):
    result = {}
    for path in ASSETS.list_assets(base, recursive=False, include_folder=False):
        asset = ASSETS.load_asset(path)
        if isinstance(asset, unreal.SkeletalMesh):
            result[asset.get_name()] = asset
    return result


def component_materials(component):
    return [component.get_material(index).get_path_name() if component.get_material(index) else None for index in range(component.get_num_materials())]


def spawn_runtime_assembly(row, x):
    actor = ACTORS.spawn_actor_from_class(unreal.WyrmDragonCharacter, unreal.Vector(x, 0.0, 200.0))
    actor.set_actor_label("WP23_RuntimeAlignment_" + row["label"])
    actor.set_dragon_id(unreal.Name("Verdance"))
    actor.set_dragon_form(unreal.WyrmDragonForm.TRUE_FORM)
    actor.set_actor_enable_collision(False)
    movement = actor.get_component_by_class(unreal.CharacterMovementComponent)
    if movement:
        movement.stop_movement_immediately()
        movement.disable_movement()
    actor.set_actor_tick_enabled(False)
    leader = actor.get_component_by_class(unreal.SkeletalMeshComponent)
    followers = list(actor.follower_mesh_components)
    source_meshes = available_meshes(row["base"])
    if len(source_meshes) != row["expected_parts"]:
        raise RuntimeError(f"{row['label']} expected {row['expected_parts']} meshes, got {len(source_meshes)}")

    active = []
    active_components = [leader]
    hidden = []
    if row["swap"]:
        leader.set_skinned_asset_and_update(source_meshes["Hip-Local"])
        for component in followers:
            name = component.get_name()
            mesh = source_meshes.get(name)
            if mesh is None:
                component.set_visibility(False, True)
                hidden.append(name)
                continue
            component.set_skinned_asset_and_update(mesh)
            component.set_visibility(True, True)
            component.set_leader_pose_component(leader, True, False)
            active.append(name)
            active_components.append(component)
    else:
        active = [component.get_name() for component in followers]
        active_components.extend(followers)

    animation = load(f"{row['base']}/{row['animation']}.{row['animation']}", unreal.AnimSequence)
    leader.set_animation_mode(unreal.AnimationMode.ANIMATION_SINGLE_NODE)
    leader.set_animation(animation)
    leader.set_update_animation_in_editor(True)
    leader.play(True)
    leader.set_position(animation.get_play_length() * 0.25, False)
    origin, extent = actor.get_actor_bounds(False)
    actor.set_actor_location(actor.get_actor_location() + unreal.Vector(0.0, 0.0, -(origin.z - extent.z)), False, False)
    origin, extent = actor.get_actor_bounds(False)
    REPORT["assemblies"].append({
        "label": row["label"],
        "fixture": "AWyrmDragonCharacter Verdance runtime hierarchy with candidate assets substituted in place",
        "leader_mesh": leader.get_skinned_asset().get_path_name(),
        "animation": animation.get_path_name(),
        "source_part_count": len(source_meshes),
        "active_follower_count": len(active),
        "hidden_control_followers": sorted(hidden),
        "leader_pose_links_assigned": len(active) if row["swap"] else len(followers),
        "materials_populated": all(component_materials(component) and all(component_materials(component)) for component in active_components),
        "bounds": {"origin": list(origin.to_tuple()), "extent": list(extent.to_tuple()), "dimensions_cm": [float(extent.x * 2), float(extent.y * 2), float(extent.z * 2)]},
    })
    return actor


def add_lighting_and_floor():
    cube = load("/Engine/BasicShapes/Cube.Cube", unreal.StaticMesh)
    floor = ACTORS.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(0.0, 0.0, -10.0))
    floor.get_component_by_class(unreal.StaticMeshComponent).set_static_mesh(cube)
    floor.set_actor_scale3d(unreal.Vector(42.0, 32.0, 0.2))
    for rotation, intensity in (((-45.0, -35.0, 0.0), 5.0), ((-25.0, 145.0, 0.0), 2.0)):
        light = ACTORS.spawn_actor_from_class(unreal.DirectionalLight, unreal.Vector(0.0, 0.0, 900.0), unreal.Rotator(*rotation))
        component = light.get_component_by_class(unreal.DirectionalLightComponent)
        component.set_mobility(unreal.ComponentMobility.MOVABLE)
        component.set_intensity(intensity)
    sky = ACTORS.spawn_actor_from_class(unreal.SkyLight, unreal.Vector(0.0, 0.0, 900.0))
    sky_component = sky.get_component_by_class(unreal.SkyLightComponent)
    sky_component.set_mobility(unreal.ComponentMobility.MOVABLE)
    sky_component.set_editor_property("source_type", unreal.SkyLightSourceType.SLS_SPECIFIED_CUBEMAP)
    sky_component.set_cubemap(load("/Engine/MapTemplates/Sky/DaylightAmbientCubemap.DaylightAmbientCubemap", unreal.TextureCube))
    sky_component.set_intensity(2.0)


def make_capture():
    actor = ACTORS.spawn_actor_from_class(unreal.SceneCapture2D, unreal.Vector())
    component = actor.get_component_by_class(unreal.SceneCaptureComponent2D)
    target = unreal.RenderingLibrary.create_render_target2d(actor, 1440, 1080, unreal.TextureRenderTargetFormat.RTF_RGBA8)
    component.set_editor_property("texture_target", target)
    component.set_editor_property("capture_source", unreal.SceneCaptureSource.SCS_FINAL_COLOR_LDR)
    component.set_editor_property("fov_angle", 42.0)
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
    CURRENT_ACTOR = spawn_runtime_assembly(ROWS[0], 0.0)
    capture_actor, capture_component, target = make_capture()
    REPORT["capture_status"] = "WAITING_FOR_RENDER"
    write_report()
    start = time.monotonic()
    stage = 0
    views = []
    for row in ROWS:
        views.append((f"{stage + 1:02d}_{row['label']}.png", (1900.0, -3200.0, 1700.0), (0.0, 0.0, 650.0)))
        stage += 1
    stage = 0

    def tick(_delta):
        global stage, CURRENT_ACTOR
        try:
            if time.monotonic() - start < 25.0 + stage * 3.0:
                return
            if stage < len(views):
                capture(capture_actor, capture_component, target, *views[stage])
                CURRENT_ACTOR.destroy_actor()
                stage += 1
                if stage < len(ROWS):
                    CURRENT_ACTOR = spawn_runtime_assembly(ROWS[stage], 0.0)
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
