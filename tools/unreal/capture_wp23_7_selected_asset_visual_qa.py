"""Render the selected modular forge and Harvester comparison in UE 5.8."""

import hashlib
import json
import time
import traceback
from pathlib import Path

import unreal


ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
OUT = ROOT / "Saved/Diagnostics/WP23_7_SelectedAssetIntake/UnrealVisualQA"
REPORT_PATH = ROOT / "Saved/Diagnostics/WP23_7_SelectedAssetIntake/unreal_visual_qa.json"
BASE = "/Game/WYRMFALL/Development/Intake/WP23_7/Selected"
FORGE = BASE + "/Forge/ModularVoxelFine"
HARVESTER_SOURCE = BASE + "/Overseer/Source"
HARVESTER_VOXEL = BASE + "/Overseer/VoxelFine"

ASSETS = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
ACTORS = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
TOOLS = unreal.AssetToolsHelpers.get_asset_tools()
REPORT = {
    "kind": "wp23_7_selected_asset_unreal_visual_qa",
    "engine": unreal.SystemLibrary.get_engine_version(),
    "scope": "unsaved presentation fixture only; no animation, rigging, gameplay, map, facts, Echo, travel, save, or regional completion proof",
    "capture_status": "INITIALIZING",
    "qa_status": "UNASSESSED",
    "groups": {},
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


def solid_qa_material(name, color):
    folder = BASE + "/VisualQA"
    object_path = folder + "/" + name
    existing = ASSETS.load_asset(object_path) if ASSETS.does_asset_exist(object_path) else None
    if existing is not None:
        return existing
    material = TOOLS.create_asset(name, folder, unreal.Material, unreal.MaterialFactoryNew())
    expression = unreal.MaterialEditingLibrary.create_material_expression(material, unreal.MaterialExpressionConstant3Vector, -220, 0)
    expression.set_editor_property("constant", unreal.LinearColor(*color))
    unreal.MaterialEditingLibrary.connect_material_property(expression, "RGB", unreal.MaterialProperty.MP_BASE_COLOR)
    unreal.MaterialEditingLibrary.connect_material_property(expression, "RGB", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    roughness = unreal.MaterialEditingLibrary.create_material_expression(material, unreal.MaterialExpressionConstant, -220, 160)
    roughness.set_editor_property("r", 0.88)
    unreal.MaterialEditingLibrary.connect_material_property(roughness, "", unreal.MaterialProperty.MP_ROUGHNESS)
    unreal.MaterialEditingLibrary.recompile_material(material)
    ASSETS.save_loaded_asset(material)
    return material


def static_meshes(folder):
    result = []
    for path in ASSETS.list_assets(folder, recursive=True, include_folder=False):
        asset = ASSETS.load_asset(path)
        if isinstance(asset, unreal.StaticMesh):
            result.append(asset)
    return sorted(result, key=lambda asset: asset.get_path_name())


def group_bounds(actors):
    bounds = [actor.get_actor_bounds(False) for actor in actors]
    low = unreal.Vector(
        min(origin.x - extent.x for origin, extent in bounds),
        min(origin.y - extent.y for origin, extent in bounds),
        min(origin.z - extent.z for origin, extent in bounds),
    )
    high = unreal.Vector(
        max(origin.x + extent.x for origin, extent in bounds),
        max(origin.y + extent.y for origin, extent in bounds),
        max(origin.z + extent.z for origin, extent in bounds),
    )
    return low, high


def bounds_record(low, high):
    return {
        "min": list(low.to_tuple()), "max": list(high.to_tuple()),
        "dimensions_cm": [high.x - low.x, high.y - low.y, high.z - low.z],
    }


def spawn_group(label, folder, target_center_x, target_max_dimension, override_material=None):
    meshes = static_meshes(folder)
    if not meshes:
        raise RuntimeError("No static meshes under " + folder)
    actors = []
    for mesh in meshes:
        actor = ACTORS.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector())
        actor.set_actor_label(label + "_" + mesh.get_name())
        component = actor.get_component_by_class(unreal.StaticMeshComponent)
        component.set_static_mesh(mesh)
        component.set_mobility(unreal.ComponentMobility.MOVABLE)
        if override_material:
            for index in range(component.get_num_materials()):
                component.set_material(index, override_material)
        actors.append(actor)
    low, high = group_bounds(actors)
    dimensions = (high.x - low.x, high.y - low.y, high.z - low.z)
    factor = target_max_dimension / max(dimensions)
    for actor in actors:
        actor.set_actor_scale3d(unreal.Vector(factor, factor, factor))
    low, high = group_bounds(actors)
    center = (low + high) * 0.5
    offset = unreal.Vector(target_center_x - center.x, -center.y, -low.z)
    for actor in actors:
        actor.set_actor_location(actor.get_actor_location() + offset, False, False)
    low, high = group_bounds(actors)
    REPORT["groups"][label] = {
        "asset_folder": folder, "static_mesh_count": len(meshes),
        "normalization_scale": factor, "bounds": bounds_record(low, high),
        "materials_populated": all(
            actor.get_component_by_class(unreal.StaticMeshComponent).get_num_materials() > 0 and
            all(actor.get_component_by_class(unreal.StaticMeshComponent).get_material(i) is not None for i in range(actor.get_component_by_class(unreal.StaticMeshComponent).get_num_materials()))
            for actor in actors
        ),
    }
    return actors


def add_stage():
    cube = load("/Engine/BasicShapes/Cube.Cube", unreal.StaticMesh)
    floor = ACTORS.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(2200.0, 0.0, -15.0))
    floor.get_component_by_class(unreal.StaticMeshComponent).set_static_mesh(cube)
    floor.set_actor_scale3d(unreal.Vector(65.0, 30.0, 0.15))
    for rotation, intensity in (((-38.0, -35.0, 0.0), 7.5), ((-25.0, 145.0, 0.0), 3.5)):
        light = ACTORS.spawn_actor_from_class(unreal.DirectionalLight, unreal.Vector(0.0, 0.0, 1200.0), unreal.Rotator(*rotation))
        component = light.get_component_by_class(unreal.DirectionalLightComponent)
        component.set_mobility(unreal.ComponentMobility.MOVABLE)
        component.set_intensity(intensity)
    sky = ACTORS.spawn_actor_from_class(unreal.SkyLight, unreal.Vector(0.0, 0.0, 800.0))
    sky_component = sky.get_component_by_class(unreal.SkyLightComponent)
    sky_component.set_mobility(unreal.ComponentMobility.MOVABLE)
    sky_component.set_intensity(1.2)
    for location, intensity, color in (
        ((0.0, -1300.0, 1050.0), 28000.0, (1.0, 0.72, 0.48, 1.0)),
        ((500.0, 500.0, 850.0), 18000.0, (0.38, 0.55, 1.0, 1.0)),
        ((3600.0, -700.0, 420.0), 24000.0, (0.85, 0.9, 1.0, 1.0)),
    ):
        light = ACTORS.spawn_actor_from_class(unreal.PointLight, unreal.Vector(*location))
        component = light.get_component_by_class(unreal.PointLightComponent)
        component.set_mobility(unreal.ComponentMobility.MOVABLE)
        component.set_intensity(intensity)
        component.set_attenuation_radius(5000.0)
        component.set_light_color(unreal.LinearColor(*color))


def make_capture():
    actor = ACTORS.spawn_actor_from_class(unreal.SceneCapture2D, unreal.Vector())
    component = actor.get_component_by_class(unreal.SceneCaptureComponent2D)
    target = unreal.RenderingLibrary.create_render_target2d(actor, 1600, 1000, unreal.TextureRenderTargetFormat.RTF_RGBA8)
    component.set_editor_property("texture_target", target)
    component.set_editor_property("capture_source", unreal.SceneCaptureSource.SCS_FINAL_COLOR_LDR)
    component.set_editor_property("fov_angle", 42.0)
    component.set_editor_property("capture_every_frame", True)
    return actor, component, target


def capture(actor, component, target, name, camera, look_at, ortho_width=None):
    if ortho_width:
        component.set_editor_property("projection_type", unreal.CameraProjectionMode.ORTHOGRAPHIC)
        component.set_editor_property("ortho_width", ortho_width)
    else:
        component.set_editor_property("projection_type", unreal.CameraProjectionMode.PERSPECTIVE)
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
    add_stage()
    spawn_group("Forge_ModularVoxelFine", FORGE, 0.0, 1800.0)
    source_material = solid_qa_material("M_HarvesterSource_QA_V2", (2.5, 0.28, 0.035, 1.0))
    voxel_material = solid_qa_material("M_HarvesterVoxelFine_QA_V2", (0.035, 0.75, 2.5, 1.0))
    spawn_group("Harvester_Source", HARVESTER_SOURCE, 3300.0, 360.0, source_material)
    spawn_group("Harvester_VoxelFine", HARVESTER_VOXEL, 3900.0, 360.0, voxel_material)
    capture_actor, capture_component, target = make_capture()
    REPORT["capture_status"] = "WAITING_FOR_RENDER"
    write_report()
    started = time.monotonic()
    stage = 0
    views = [
        ("01_forge_modular_composition.png", (0.0, -4100.0, 1450.0), (0.0, 0.0, 390.0), None),
        ("02_harvester_source_vs_voxel.png", (3600.0, -1000.0, 180.0), (3600.0, 0.0, 180.0), 1200.0),
    ]

    def tick(_delta):
        global stage
        try:
            if time.monotonic() - started < 30.0 + stage * 3.0:
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
