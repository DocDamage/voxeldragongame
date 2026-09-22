"""Capture an unsaved WP-23.7 Pyraxis, cast, and environment QA fixture."""

import hashlib
import json
from pathlib import Path
import time
import traceback

import unreal


ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
OUT = ROOT / "Saved/Diagnostics/WP23_7_CinderreachVisualQA"
REPORT_PATH = ROOT / "Saved/Diagnostics/WP23_7_cinderreach_visual_qa.json"
DEST = "/Game/WYRMFALL/Development/Intake/WP23_7"
DRAGON_BASE = DEST + "/Pyraxis/Lava_Dragon/SkeletalMeshes"
SOURCE = ROOT / "Saved/Diagnostics/WP23_7_Source"
QA_TEXTURES = DEST + "/VisualQA/Textures"
QA_MATERIALS = DEST + "/VisualQA/Materials"

ASSETS = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
ACTORS = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
TOOLS = unreal.AssetToolsHelpers.get_asset_tools()
REPORT = {
    "kind": "wp23_7_cinderreach_visual_qa",
    "engine": unreal.SystemLibrary.get_engine_version(),
    "scope": "unsaved rendered intake fixture only; no production map, profile, gameplay, travel, fact, Echo, or save acceptance",
    "capture_status": "INITIALIZING", "qa_status": "UNASSESSED",
    "cast_candidates": [], "pyraxis": {}, "environment": [], "captures": [],
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
    task.factory = unreal.TextureFactory(); TOOLS.import_asset_tasks([task])
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
    unreal.MaterialEditingLibrary.recompile_material(material); ASSETS.save_loaded_asset(material)
    return material


def place_on_ground(actor, x, y, scale):
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
    REPORT["cast_candidates"].append({"label": label, "mesh": mesh.get_path_name(), "materials": material_paths(component), "bounds": place_on_ground(actor, x, y, scale)})


def spawn_static(label, path, x, y, scale, texture_source=None):
    mesh = load(path, unreal.StaticMesh)
    actor = ACTORS.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector())
    component = actor.get_component_by_class(unreal.StaticMeshComponent); component.set_static_mesh(mesh)
    if texture_source:
        material = qa_material(label, import_texture(label, texture_source))
        for index in range(component.get_num_materials()): component.set_material(index, material)
    REPORT["environment"].append({"label": label, "mesh": mesh.get_path_name(), "materials": material_paths(component), "bounds": place_on_ground(actor, x, y, scale)})


def spawn_pyraxis():
    meshes = [ASSETS.load_asset(path) for path in ASSETS.list_assets(DRAGON_BASE, recursive=False, include_folder=False)]
    meshes = [mesh for mesh in meshes if isinstance(mesh, unreal.SkeletalMesh)]
    if len(meshes) != 35: raise RuntimeError(f"Expected 35 Pyraxis skeletal meshes, got {len(meshes)}")
    leader = next((mesh for mesh in meshes if mesh.get_name().casefold() == "hip-local"), None)
    animation = load(DRAGON_BASE + "/Lava_DragonIdle_01.Lava_DragonIdle_01", unreal.AnimSequence)
    actors = []; components = []; leader_actor = None; leader_component = None
    for mesh in meshes:
        actor = ACTORS.spawn_actor_from_class(unreal.SkeletalMeshActor, unreal.Vector())
        component = actor.get_component_by_class(unreal.SkeletalMeshComponent); component.set_skinned_asset_and_update(mesh)
        component.set_animation_mode(unreal.AnimationMode.ANIMATION_SINGLE_NODE); component.set_animation(animation)
        component.set_update_animation_in_editor(True); component.play(True); component.set_position(animation.get_play_length() * 0.25, False)
        actor.set_actor_scale3d(unreal.Vector(0.04, 0.04, 0.04)); actors.append(actor); components.append(component)
        if mesh == leader: leader_actor = actor; leader_component = component
    for component in components:
        if component != leader_component:
            component.set_leader_pose_component(leader_component, True, False)
    origin, extent = leader_actor.get_actor_bounds(False); offset = unreal.Vector(800.0 - origin.x, 0.0 - origin.y, -(origin.z - extent.z))
    for actor in actors: actor.set_actor_location(actor.get_actor_location() + offset, False, False)
    origin, extent = leader_actor.get_actor_bounds(False)
    REPORT["pyraxis"] = {"leader_mesh": leader.get_path_name(), "part_count": len(meshes), "animation": animation.get_path_name(), "scale": 0.04, "materials_populated": all(material_paths(c) and all(material_paths(c)) for c in components), "leader_bounds": {"origin": list(origin.to_tuple()), "extent": list(extent.to_tuple())}, "assembly": "RUNTIME_STYLE_LEADER_POSE"}


def add_lighting_and_floor():
    cube = load("/Engine/BasicShapes/Cube.Cube", unreal.StaticMesh)
    floor = ACTORS.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(1300.0, 500.0, -10.0)); floor.get_component_by_class(unreal.StaticMeshComponent).set_static_mesh(cube); floor.set_actor_scale3d(unreal.Vector(38.0, 24.0, 0.2))
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
    spawn_skeletal("Magnarok_Champion", DEST + "/Cast/Rulers/TVS_VoxelKnights_Champion/TVS_VoxelKnights_Champion.TVS_VoxelKnights_Champion", -260.0, 0.0, 0.45, SOURCE / "Cast/TVS_VoxelKnights_Champion_Texture.png")
    spawn_skeletal("Magnarok_Commander", DEST + "/Cast/Rulers/TVS_VoxelKnights_Commander/TVS_VoxelKnights_Commander.TVS_VoxelKnights_Commander", 0.0, 0.0, 0.45, SOURCE / "Cast/TVS_VoxelKnights_Commander_Texture.png")
    spawn_skeletal("Choir_Priest", DEST + "/Cast/Overseer/TVS_VoxelCathedral_Priest/TVS_VoxelCathedral_Priest.TVS_VoxelCathedral_Priest", 260.0, 0.0, 0.45, SOURCE / "Cast/TVS_VoxelCathedral_Priest_Texture.png")
    spawn_pyraxis()
    spawn_static("ArenaSand", DEST + "/Environment/Ground/arena_sand/arena_sand.arena_sand", 1700.0, 250.0, 6.0, SOURCE / "Ground/palette.png")
    spawn_static("ObsidianShore", DEST + "/Environment/Ground/obsidian_shore/obsidian_shore.obsidian_shore", 2150.0, 250.0, 6.0, SOURCE / "Ground/palette.png")
    spawn_static("LavaLake", DEST + "/Environment/Lava/lava_lake/lava_lake.lava_lake", 1700.0, 750.0, 5.0, SOURCE / "Lava/palette.png")
    spawn_static("LavaWaterfall", DEST + "/Environment/Lava/lava_waterfall/lava_waterfall.lava_waterfall", 2300.0, 750.0, 5.0, SOURCE / "Lava/palette.png")
    spawn_static("Dungeon", DEST + "/Environment/Dungeon/Dungeon1/Dungeon1.Dungeon1", 2750.0, 550.0, 1.2)
    spawn_static("Pillar", DEST + "/Environment/Dungeon/Pillar/Pillar1.Pillar1", 3000.0, 350.0, 1.2)
    spawn_static("TortureCage", DEST + "/Environment/Dungeon/TortureCage1/TortureCage1.TortureCage1", 3000.0, 750.0, 1.2)
    capture_actor, capture_component, target = make_capture(); REPORT["capture_status"] = "WAITING_FOR_RENDER"; write_report(); start = time.monotonic(); stage = 0
    views = [("01_cinderreach_cast.png", (0.0, -1050.0, 220.0), (0.0, 0.0, 95.0)), ("02_pyraxis_assembly.png", (1300.0, -1100.0, 500.0), (800.0, 0.0, 160.0)), ("03_cinderreach_environment.png", (2600.0, -1200.0, 700.0), (2250.0, 550.0, 140.0))]
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
