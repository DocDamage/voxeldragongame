"""Capture an unsaved WP-23.4 Frostmane, Alaric, and environment QA fixture."""

import hashlib
import json
from pathlib import Path
import time
import traceback

import unreal


ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
OUT = ROOT / "Saved/Diagnostics/WP23_4_FrostholdVisualQA"
REPORT_PATH = ROOT / "Saved/Diagnostics/WP23_4_frosthold_visual_qa.json"
DEST = "/Game/WYRMFALL/Development/Intake/WP23_4"
DRAGON_BASE = DEST + "/Frostmane/White_Dragon/SkeletalMeshes"
SOURCE = ROOT / "Saved/Diagnostics/WP23_4_Source"
QA_TEXTURES = DEST + "/VisualQA/Textures"
QA_MATERIALS = DEST + "/VisualQA/Materials"

ASSETS = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
ACTORS = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
TOOLS = unreal.AssetToolsHelpers.get_asset_tools()
REPORT = {
    "kind": "wp23_4_frosthold_visual_qa", "engine": unreal.SystemLibrary.get_engine_version(),
    "scope": "unsaved rendered intake fixture only; no profile, map, gameplay, travel, fact, Echo, or save acceptance",
    "capture_status": "INITIALIZING", "qa_status": "UNASSESSED",
    "alaric_candidates": [], "frostmane": {}, "environment": [], "captures": [],
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


def import_texture(label, source):
    object_path = f"{QA_TEXTURES}/T_{label}"
    existing = ASSETS.load_asset(object_path) if ASSETS.does_asset_exist(object_path) else None
    if existing is not None:
        return existing
    if not source.is_file():
        raise RuntimeError("Missing supplied palette: " + source.as_posix())
    task = unreal.AssetImportTask()
    task.filename = source.as_posix(); task.destination_path = QA_TEXTURES; task.destination_name = "T_" + label
    task.automated = True; task.save = True; task.replace_existing = True; task.async_ = False
    task.factory = unreal.TextureFactory()
    TOOLS.import_asset_tasks([task])
    texture = load(object_path, unreal.Texture2D)
    texture.set_editor_property("mip_gen_settings", unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS)
    texture.set_editor_property("filter", unreal.TextureFilter.TF_NEAREST)
    ASSETS.save_loaded_asset(texture)
    return texture


def qa_material(label, texture):
    object_path = f"{QA_MATERIALS}/M_{label}_SuppliedPaletteQA"
    existing = ASSETS.load_asset(object_path) if ASSETS.does_asset_exist(object_path) else None
    if existing is not None:
        return existing
    material = TOOLS.create_asset("M_" + label + "_SuppliedPaletteQA", QA_MATERIALS, unreal.Material, unreal.MaterialFactoryNew())
    material.set_editor_property("two_sided", True)
    sample = unreal.MaterialEditingLibrary.create_material_expression(material, unreal.MaterialExpressionTextureSample, -260, 0)
    sample.set_editor_property("texture", texture)
    unreal.MaterialEditingLibrary.connect_material_property(sample, "RGB", unreal.MaterialProperty.MP_BASE_COLOR)
    unreal.MaterialEditingLibrary.connect_material_property(sample, "RGB", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    roughness = unreal.MaterialEditingLibrary.create_material_expression(material, unreal.MaterialExpressionConstant, -260, 180)
    roughness.set_editor_property("r", 0.9)
    unreal.MaterialEditingLibrary.connect_material_property(roughness, "", unreal.MaterialProperty.MP_ROUGHNESS)
    unreal.MaterialEditingLibrary.recompile_material(material)
    ASSETS.save_loaded_asset(material)
    return material


def ground_actor(actor, x, y, scale):
    actor.set_actor_scale3d(unreal.Vector(scale, scale, scale))
    origin, extent = actor.get_actor_bounds(False); current = actor.get_actor_location()
    actor.set_actor_location(unreal.Vector(current.x + x - origin.x, current.y + y - origin.y, current.z - (origin.z - extent.z)), False, False)
    origin, extent = actor.get_actor_bounds(False)
    return {"origin": list(origin.to_tuple()), "extent": list(extent.to_tuple()), "dimensions_cm": [float(extent.x * 2), float(extent.y * 2), float(extent.z * 2)]}


def spawn_skeletal(label, path, x, y, scale, texture_source):
    mesh = load(path, unreal.SkeletalMesh)
    actor = ACTORS.spawn_actor_from_class(unreal.SkeletalMeshActor, unreal.Vector())
    component = actor.get_component_by_class(unreal.SkeletalMeshComponent); component.set_skinned_asset_and_update(mesh)
    material = qa_material(label, import_texture(label, texture_source))
    for index in range(component.get_num_materials()): component.set_material(index, material)
    REPORT["alaric_candidates"].append({"label": label, "mesh": mesh.get_path_name(), "materials": material_paths(component), "scale": scale, "bounds": ground_actor(actor, x, y, scale)})


def spawn_static(label, path, x, y, scale, texture_source=None, rotation=None):
    mesh = load(path, unreal.StaticMesh)
    actor = ACTORS.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector())
    component = actor.get_component_by_class(unreal.StaticMeshComponent); component.set_static_mesh(mesh)
    if rotation: actor.set_actor_rotation(unreal.Rotator(*rotation), False)
    if texture_source:
        material = qa_material(label, import_texture(label, texture_source))
        for index in range(component.get_num_materials()): component.set_material(index, material)
    REPORT["environment"].append({"label": label, "mesh": mesh.get_path_name(), "materials": material_paths(component), "scale": scale, "bounds": ground_actor(actor, x, y, scale)})


def spawn_frostmane():
    meshes = [ASSETS.load_asset(path) for path in ASSETS.list_assets(DRAGON_BASE, recursive=False, include_folder=False)]
    meshes = [mesh for mesh in meshes if isinstance(mesh, unreal.SkeletalMesh)]
    if len(meshes) != 37: raise RuntimeError(f"Expected 37 Frostmane skeletal meshes, got {len(meshes)}")
    leader = next((mesh for mesh in meshes if mesh.get_name().casefold() == "hip-local"), None)
    animation = load(DRAGON_BASE + "/White_DragonIdle_01.White_DragonIdle_01", unreal.AnimSequence)
    leader_actor = ACTORS.spawn_actor_from_class(unreal.SkeletalMeshActor, unreal.Vector(750.0, 0.0, 0.0))
    leader_component = leader_actor.get_component_by_class(unreal.SkeletalMeshComponent)
    leader_component.set_skinned_asset_and_update(leader)
    actors = [leader_actor]; components = [leader_component]
    for mesh in meshes:
        if mesh == leader: continue
        follower_actor = ACTORS.spawn_actor_from_class(unreal.SkeletalMeshActor, unreal.Vector(750.0, 0.0, 0.0))
        component = follower_actor.get_component_by_class(unreal.SkeletalMeshComponent)
        component.set_skinned_asset_and_update(mesh)
        actors.append(follower_actor); components.append(component)
    for mesh_actor in actors: mesh_actor.set_actor_scale3d(unreal.Vector(0.04, 0.04, 0.04))
    for component in components:
        component.set_animation_mode(unreal.AnimationMode.ANIMATION_SINGLE_NODE)
        component.set_animation(animation)
        component.set_update_animation_in_editor(True)
        component.play(True)
        component.set_position(animation.get_play_length() * 0.25, False)
        component.set_play_rate(0.0)
    bounds_list = [mesh_actor.get_actor_bounds(False) for mesh_actor in actors]
    min_z = min(origin.z - extent.z for origin, extent in bounds_list)
    for mesh_actor in actors:
        location = mesh_actor.get_actor_location()
        mesh_actor.set_actor_location(unreal.Vector(location.x, location.y, location.z - min_z), False, False)
    bounds_list = [mesh_actor.get_actor_bounds(False) for mesh_actor in actors]
    low = unreal.Vector(min(origin.x - extent.x for origin, extent in bounds_list), min(origin.y - extent.y for origin, extent in bounds_list), min(origin.z - extent.z for origin, extent in bounds_list))
    high = unreal.Vector(max(origin.x + extent.x for origin, extent in bounds_list), max(origin.y + extent.y for origin, extent in bounds_list), max(origin.z + extent.z for origin, extent in bounds_list))
    bounds = {"origin": list(((low + high) * 0.5).to_tuple()), "extent": list(((high - low) * 0.5).to_tuple()), "dimensions_cm": list((high - low).to_tuple())}
    REPORT["frostmane"] = {"leader_mesh": leader.get_path_name(), "part_count": len(meshes), "animation": animation.get_path_name(), "scale": 0.04, "materials_populated": all(material_paths(c) and all(material_paths(c)) for c in components), "leader_bounds": bounds, "assembly": "SYNCHRONIZED_SUPPLIED_ANIMATION_FRAME"}


def add_lighting_and_floor():
    cube = load("/Engine/BasicShapes/Cube.Cube", unreal.StaticMesh)
    floor = ACTORS.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(1200.0, 500.0, -10.0)); floor.get_component_by_class(unreal.StaticMeshComponent).set_static_mesh(cube); floor.set_actor_scale3d(unreal.Vector(38.0, 24.0, 0.2))
    for rotation, intensity in (((-45.0, -35.0, 0.0), 5.0), ((-25.0, 145.0, 0.0), 2.0)):
        light = ACTORS.spawn_actor_from_class(unreal.DirectionalLight, unreal.Vector(0.0, 0.0, 700.0), unreal.Rotator(*rotation)); component = light.get_component_by_class(unreal.DirectionalLightComponent); component.set_mobility(unreal.ComponentMobility.MOVABLE); component.set_intensity(intensity)


def make_capture():
    actor = ACTORS.spawn_actor_from_class(unreal.SceneCapture2D, unreal.Vector()); component = actor.get_component_by_class(unreal.SceneCaptureComponent2D)
    target = unreal.RenderingLibrary.create_render_target2d(actor, 1440, 1080, unreal.TextureRenderTargetFormat.RTF_RGBA8)
    component.set_editor_property("texture_target", target); component.set_editor_property("capture_source", unreal.SceneCaptureSource.SCS_FINAL_COLOR_LDR); component.set_editor_property("fov_angle", 40.0); component.set_editor_property("capture_every_frame", True)
    return actor, component, target


def capture(actor, component, target, name, camera, look_at):
    actor.set_actor_location(unreal.Vector(*camera), False, False); actor.set_actor_rotation(unreal.MathLibrary.find_look_at_rotation(unreal.Vector(*camera), unreal.Vector(*look_at)), False); component.capture_scene()
    unreal.RenderingLibrary.export_render_target(component, target, OUT.as_posix(), name); path = OUT / name
    if not path.is_file() or path.stat().st_size <= 0: raise RuntimeError("Capture was not written: " + path.as_posix())
    REPORT["captures"].append({"view": name.removesuffix(".png"), "path": path.as_posix(), "sha256": sha256(path)})


try:
    OUT.mkdir(parents=True, exist_ok=True); unreal.EditorLoadingAndSavingUtils.new_blank_map(False); add_lighting_and_floor()
    spawn_skeletal("Alaric_Captain", DEST + "/Rulers/TVS_VoxelKnights_Captain/TVS_VoxelKnights_Captain.TVS_VoxelKnights_Captain", -250.0, 0.0, 0.45, SOURCE / "Rulers/TVS_VoxelKnights_Captain_Texture.png")
    spawn_skeletal("Alaric_Champion", DEST + "/Rulers/TVS_VoxelKnights_Champion/TVS_VoxelKnights_Champion.TVS_VoxelKnights_Champion", 0.0, 0.0, 0.45, SOURCE / "Rulers/TVS_VoxelKnights_Champion_Texture.png")
    spawn_skeletal("Alaric_King", DEST + "/Rulers/TVS_VoxelPalace_King/TVS_VoxelPalace_King.TVS_VoxelPalace_King", 250.0, 0.0, 0.50, SOURCE / "Rulers/TVS_VoxelPalace_King_Texture.png")
    spawn_frostmane()
    spawn_static("Keep", DEST + "/Environment/Fort/TVS_VoxelKnights_Keep/TVS_VoxelKnights_Keep.TVS_VoxelKnights_Keep", 1900.0, 800.0, 0.22, SOURCE / "Fort/TVS_VoxelKnights_Keep_Texture.png")
    spawn_static("Tower", DEST + "/Environment/Fort/TVS_VoxelKnights_Tower/TVS_VoxelKnights_Tower.TVS_VoxelKnights_Tower", 2450.0, 900.0, 0.22, SOURCE / "Fort/TVS_VoxelKnights_Tower_Texture.png")
    spawn_static("FrozenLake", DEST + "/Environment/FrozenWater/frozen_lake/frozen_lake.frozen_lake", 1700.0, 200.0, 9.0, SOURCE / "FrozenWater/palette.png")
    spawn_static("FrozenWaterfall", DEST + "/Environment/FrozenWater/frozen_waterfall/frozen_waterfall.frozen_waterfall", 2450.0, 350.0, 7.0, SOURCE / "FrozenWater/palette.png")
    spawn_static("SnowIgloo", DEST + "/Environment/SnowTiles/overworld-8-snow_igloo/overworld-8-snow_igloo.overworld-8-snow_igloo", 2050.0, 350.0, 100.0, SOURCE / "SnowTiles/overworld-8-snow_igloo.png", (90.0, 0.0, 0.0))
    capture_actor, capture_component, target = make_capture(); REPORT["capture_status"] = "WAITING_FOR_RENDER"; write_report(); start = time.monotonic(); stage = 0
    views = [("01_alaric_candidates.png", (0.0, -1050.0, 220.0), (0.0, 0.0, 95.0)), ("02_frostmane_assembly.png", (1650.0, -1800.0, 900.0), (750.0, 0.0, 320.0)), ("03_frosthold_environment.png", (2600.0, -1500.0, 900.0), (2100.0, 650.0, 250.0))]
    def tick(_delta):
        global stage
        try:
            if time.monotonic() - start < 25.0 + stage * 2.0: return
            if stage < len(views): capture(capture_actor, capture_component, target, *views[stage]); stage += 1; write_report(); return
            REPORT["capture_status"] = "PASS"; REPORT["qa_status"] = "CAPTURED_REQUIRES_VISUAL_REVIEW"; write_report(); unreal.unregister_slate_post_tick_callback(handle); unreal.SystemLibrary.quit_editor()
        except Exception:
            REPORT["capture_status"] = "ERROR"; REPORT["error"] = traceback.format_exc(); write_report(); unreal.unregister_slate_post_tick_callback(handle); unreal.SystemLibrary.quit_editor()
    handle = unreal.register_slate_post_tick_callback(tick)
except Exception:
    REPORT["capture_status"] = "ERROR"; REPORT["error"] = traceback.format_exc(); write_report(); unreal.SystemLibrary.quit_editor()
