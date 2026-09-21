"""Capture an unsaved WP-23.8 Rotwing, Rotking, and environment QA fixture."""

import hashlib
import json
from pathlib import Path
import time
import traceback

import unreal


ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
OUT = ROOT / "Saved/Diagnostics/WP23_8_AshenWastesVisualQA"
REPORT_PATH = ROOT / "Saved/Diagnostics/WP23_8_ashen_wastes_visual_qa.json"
DEST = "/Game/WYRMFALL/Development/Intake/WP23_8"
DRAGON_BASE = DEST + "/Rotwing/Zombie_Dragon/SkeletalMeshes"
SOURCE = ROOT / "Saved/Diagnostics/WP23_8_Source"
QA_TEXTURES = DEST + "/VisualQA/Textures"
QA_MATERIALS = DEST + "/VisualQA/Materials"

ASSETS = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
ACTORS = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
TOOLS = unreal.AssetToolsHelpers.get_asset_tools()
REPORT = {
    "kind": "wp23_8_ashen_wastes_visual_qa",
    "engine": unreal.SystemLibrary.get_engine_version(),
    "scope": "unsaved rendered intake fixture only; no production map, profile, gameplay, travel, fact, Echo, or save acceptance",
    "capture_status": "INITIALIZING",
    "qa_status": "UNASSESSED",
    "rotking_candidates": [],
    "rotwing": {},
    "environment": [],
    "captures": [],
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
    return [
        component.get_material(index).get_path_name() if component.get_material(index) else None
        for index in range(component.get_num_materials())
    ]


def import_texture(label, source):
    object_path = f"{QA_TEXTURES}/T_{label}"
    existing = ASSETS.load_asset(object_path) if ASSETS.does_asset_exist(object_path) else None
    if existing is not None:
        return existing
    task = unreal.AssetImportTask()
    task.filename = source.as_posix()
    task.destination_path = QA_TEXTURES
    task.destination_name = "T_" + label
    task.automated = True
    task.save = True
    task.replace_existing = True
    task.async_ = False
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
    material = TOOLS.create_asset(
        "M_" + label + "_SuppliedPaletteQA", QA_MATERIALS,
        unreal.Material, unreal.MaterialFactoryNew())
    material.set_editor_property("two_sided", True)
    sample = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionTextureSample, -260, 0)
    sample.set_editor_property("texture", texture)
    unreal.MaterialEditingLibrary.connect_material_property(sample, "RGB", unreal.MaterialProperty.MP_BASE_COLOR)
    unreal.MaterialEditingLibrary.connect_material_property(sample, "RGB", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    roughness = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionConstant, -260, 180)
    roughness.set_editor_property("r", 0.9)
    unreal.MaterialEditingLibrary.connect_material_property(roughness, "", unreal.MaterialProperty.MP_ROUGHNESS)
    unreal.MaterialEditingLibrary.recompile_material(material)
    ASSETS.save_loaded_asset(material)
    return material


def place_on_ground(actor, x, y, scale):
    actor.set_actor_scale3d(unreal.Vector(scale, scale, scale))
    origin, extent = actor.get_actor_bounds(False)
    current = actor.get_actor_location()
    actor.set_actor_location(
        unreal.Vector(current.x + x - origin.x, current.y + y - origin.y,
                      current.z - (origin.z - extent.z)), False, False)
    origin, extent = actor.get_actor_bounds(False)
    return {
        "origin": list(origin.to_tuple()),
        "extent": list(extent.to_tuple()),
        "dimensions_cm": [float(extent.x * 2), float(extent.y * 2), float(extent.z * 2)],
    }


def spawn_static(label, path, x, y, scale, category, texture_source=None):
    mesh = load(path, unreal.StaticMesh)
    actor = ACTORS.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector())
    actor.set_actor_label("WP23_8_QA_" + label)
    component = actor.get_component_by_class(unreal.StaticMeshComponent)
    component.set_static_mesh(mesh)
    if texture_source:
        material = qa_material(label, import_texture(label, texture_source))
        for index in range(component.get_num_materials()):
            component.set_material(index, material)
    bounds = place_on_ground(actor, x, y, scale)
    row = {
        "label": label,
        "mesh": mesh.get_path_name(),
        "materials": material_paths(component),
        "body_setup_present": mesh.get_editor_property("body_setup") is not None,
        "scale": scale,
        "bounds": bounds,
    }
    REPORT[category].append(row)


def spawn_rotwing():
    meshes = []
    for path in ASSETS.list_assets(DRAGON_BASE, recursive=False, include_folder=False):
        asset = ASSETS.load_asset(path)
        if isinstance(asset, unreal.SkeletalMesh):
            meshes.append(asset)
    if len(meshes) != 35:
        raise RuntimeError(f"Expected 35 Rotwing skeletal meshes, got {len(meshes)}")
    leader_mesh = next((mesh for mesh in meshes if mesh.get_name().casefold() == "hip-local"), None)
    if leader_mesh is None:
        raise RuntimeError("Missing Rotwing Hip-Local leader")
    animation = load(DRAGON_BASE + "/Zombie_DragonIdle_01.Zombie_DragonIdle_01", unreal.AnimSequence)
    actors = []
    components = []
    leader_actor = None
    for mesh in meshes:
        actor = ACTORS.spawn_actor_from_class(unreal.SkeletalMeshActor, unreal.Vector())
        actor.set_actor_label("WP23_8_QA_Rotwing_" + mesh.get_name())
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
        if mesh == leader_mesh:
            leader_actor = actor
    origin, extent = leader_actor.get_actor_bounds(False)
    offset = unreal.Vector(700.0 - origin.x, 0.0 - origin.y, -(origin.z - extent.z))
    for actor in actors:
        actor.set_actor_location(actor.get_actor_location() + offset, False, False)
    origin, extent = leader_actor.get_actor_bounds(False)
    REPORT["rotwing"] = {
        "leader_mesh": leader_mesh.get_path_name(),
        "part_count": len(meshes),
        "animation": animation.get_path_name(),
        "scale": 0.04,
        "leader_bounds": {
            "origin": list(origin.to_tuple()),
            "extent": list(extent.to_tuple()),
            "dimensions_cm": [float(extent.x * 2), float(extent.y * 2), float(extent.z * 2)],
        },
        "materials_populated": all(material_paths(component) and all(material_paths(component)) for component in components),
        "profile_validation": "NOT_RUN; DRG-15 remains fail-closed",
    }


def add_lighting_and_floor():
    cube = load("/Engine/BasicShapes/Cube.Cube", unreal.StaticMesh)
    floor = ACTORS.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(900.0, 350.0, -10.0))
    floor.get_component_by_class(unreal.StaticMeshComponent).set_static_mesh(cube)
    floor.set_actor_scale3d(unreal.Vector(30.0, 20.0, 0.2))
    for rotation, intensity in (((-45.0, -35.0, 0.0), 5.0), ((-25.0, 145.0, 0.0), 2.0)):
        light = ACTORS.spawn_actor_from_class(
            unreal.DirectionalLight, unreal.Vector(0.0, 0.0, 700.0), unreal.Rotator(*rotation))
        component = light.get_component_by_class(unreal.DirectionalLightComponent)
        component.set_mobility(unreal.ComponentMobility.MOVABLE)
        component.set_intensity(intensity)
    sky = ACTORS.spawn_actor_from_class(unreal.SkyLight, unreal.Vector(0.0, 0.0, 700.0))
    component = sky.get_component_by_class(unreal.SkyLightComponent)
    component.set_mobility(unreal.ComponentMobility.MOVABLE)
    component.set_editor_property("source_type", unreal.SkyLightSourceType.SLS_SPECIFIED_CUBEMAP)
    component.set_cubemap(load("/Engine/MapTemplates/Sky/DaylightAmbientCubemap.DaylightAmbientCubemap", unreal.TextureCube))
    component.set_intensity(2.0)


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
    spawn_static("Rotking_Hero", DEST + "/Rotking/Hero/Character_Hero.Character_Hero", -220.0, 0.0, 1.0, "rotking_candidates")
    spawn_static("Rotking_Zombie", DEST + "/Rotking/Zombie/Character_Zombie.Character_Zombie", 80.0, 0.0, 1.0, "rotking_candidates")
    spawn_rotwing()
    spawn_static("DesertRuin", DEST + "/Environment/DesertRuin/desert_ruin.desert_ruin", 1550.0, 450.0, 8.0, "environment")
    spawn_static("ToxicPuddle", DEST + "/Environment/ToxicPuddle/toxic_puddle.toxic_puddle", 2050.0, 500.0, 8.0, "environment")
    bunker = (
        ("Wall", "0-Wall_Container_Blue-0", 1600.0, 1000.0),
        ("Chair", "30-Chair_Red-2", 1850.0, 850.0),
        ("Crate", "70-Crate-0", 2050.0, 900.0),
        ("Shelf", "81-Shelf-3", 2250.0, 1050.0),
        ("Lantern", "88-Lantern-1", 2450.0, 850.0),
        ("StorageBin", "98-StorageBin", 2650.0, 1000.0),
    )
    for label, suffix, x, y in bunker:
        stem = "Bunker - Free Sample-" + suffix
        asset = stem.replace(" ", "_")
        path = f"{DEST}/Environment/Bunker/{asset}/{asset}.{asset}"
        spawn_static(
            "Bunker_" + label, path, x, y, 100.0, "environment",
            SOURCE / "Bunker" / (stem + ".png"))

    capture_actor, capture_component, target = make_capture()
    REPORT["capture_status"] = "WAITING_FOR_RENDER"
    write_report()
    start = time.monotonic()
    stage = 0
    views = [
        ("01_rotking_candidates.png", (-100.0, -520.0, 190.0), (-80.0, 0.0, 90.0)),
        ("02_rotwing_assembly.png", (1170.0, -760.0, 350.0), (700.0, 0.0, 120.0)),
        ("03_ashen_bunker_trace.png", (2450.0, -950.0, 720.0), (2050.0, 700.0, 180.0)),
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
