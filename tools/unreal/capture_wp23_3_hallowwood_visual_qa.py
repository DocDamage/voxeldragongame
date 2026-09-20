"""Capture an unsaved WP-23.3 Grovemaw, Osk, and environment QA fixture."""

import hashlib
import json
from pathlib import Path
import time
import traceback

import unreal


ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
OUT = ROOT / "Saved/Diagnostics/WP23_3_HallowwoodVisualQA"
REPORT_PATH = ROOT / "Saved/Diagnostics/WP23_3_hallowwood_visual_qa.json"
DEST = "/Game/WYRMFALL/Development/Intake/WP23_3"
DRAGON_BASE = DEST + "/Grovemaw/Wooden_Dragon/SkeletalMeshes"
SOURCE = ROOT / "Saved/Diagnostics/WP23_3_Source"
QA_TEXTURES = DEST + "/VisualQA/Textures"
QA_MATERIALS = DEST + "/VisualQA/Materials"

ASSETS = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
ACTORS = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
TOOLS = unreal.AssetToolsHelpers.get_asset_tools()
REPORT = {
    "kind": "wp23_3_hallowwood_visual_qa",
    "engine": unreal.SystemLibrary.get_engine_version(),
    "scope": "unsaved rendered intake fixture only; no production map, profile, gameplay, travel, fact, Echo, or save acceptance",
    "capture_status": "INITIALIZING",
    "qa_status": "UNASSESSED",
    "roles": [],
    "grovemaw": {},
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
    material.set_editor_property("used_with_skeletal_mesh", True)
    material.set_editor_property("two_sided", True)
    sample = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionTextureSample, -260, 0)
    sample.set_editor_property("texture", texture)
    unreal.MaterialEditingLibrary.connect_material_property(
        sample, "RGB", unreal.MaterialProperty.MP_BASE_COLOR)
    unreal.MaterialEditingLibrary.connect_material_property(
        sample, "RGB", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    roughness = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionConstant, -260, 180)
    roughness.set_editor_property("r", 0.9)
    unreal.MaterialEditingLibrary.connect_material_property(
        roughness, "", unreal.MaterialProperty.MP_ROUGHNESS)
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


def spawn_role(label, mesh_path, animation_path, texture_source, x):
    mesh = load(mesh_path, unreal.SkeletalMesh)
    animation = load(animation_path, unreal.AnimSequence)
    skeleton = mesh.get_editor_property("skeleton")
    if animation.get_editor_property("skeleton") != skeleton:
        raise RuntimeError(label + " animation/skeleton mismatch")
    animation.set_editor_property("enable_root_motion", True)
    animation.set_editor_property("force_root_lock", True)
    actor = ACTORS.spawn_actor_from_class(unreal.SkeletalMeshActor, unreal.Vector())
    actor.set_actor_label("WP23_3_QA_" + label)
    component = actor.get_component_by_class(unreal.SkeletalMeshComponent)
    component.set_skinned_asset_and_update(mesh)
    component.set_animation_mode(unreal.AnimationMode.ANIMATION_SINGLE_NODE)
    component.set_animation(animation)
    component.set_update_animation_in_editor(True)
    component.play(True)
    component.set_position(animation.get_play_length() * 0.35, False)
    texture = import_texture(label, texture_source)
    material = qa_material(label, texture)
    for index in range(component.get_num_materials()):
        component.set_material(index, material)
    raw_origin, raw_extent = actor.get_actor_bounds(False)
    raw_height = float(raw_extent.z * 2.0)
    scale = 180.0 / raw_height
    bounds = place_on_ground(actor, x, 0.0, scale)
    REPORT["roles"].append({
        "label": label,
        "mesh": mesh.get_path_name(),
        "skeleton": skeleton.get_path_name(),
        "bone_count": component.get_num_bones(),
        "animation": animation.get_path_name(),
        "animation_seconds": animation.get_play_length(),
        "materials": material_paths(component),
        "supplied_palette_texture": texture.get_path_name(),
        "scale": scale,
        "bounds": bounds,
        "selection": "NOT_RUN; rendered comparison required",
    })


def spawn_grovemaw():
    paths = ASSETS.list_assets(DRAGON_BASE, recursive=False, include_folder=False)
    meshes = []
    for path in paths:
        asset = ASSETS.load_asset(path)
        if isinstance(asset, unreal.SkeletalMesh):
            meshes.append(asset)
    if len(meshes) != 35:
        raise RuntimeError(f"Expected 35 Grovemaw skeletal meshes, got {len(meshes)}")
    leader_mesh = next((mesh for mesh in meshes if mesh.get_name().lower() == "hip-local"), None)
    if leader_mesh is None:
        raise RuntimeError("Missing Grovemaw Hip-Local leader")
    animation = load(DRAGON_BASE + "/Wooden_DragonIdle_01.Wooden_DragonIdle_01", unreal.AnimSequence)
    actors = []
    components = []
    leader_component = None
    for mesh in meshes:
        actor = ACTORS.spawn_actor_from_class(unreal.SkeletalMeshActor, unreal.Vector())
        actor.set_actor_label("WP23_3_QA_Grovemaw_" + mesh.get_name())
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
            leader_component = component
    if leader_component is None:
        raise RuntimeError("Grovemaw leader component was not created")
    leader_actor = next(actor for actor, component in zip(actors, components) if component == leader_component)
    origin, extent = leader_actor.get_actor_bounds(False)
    offset = unreal.Vector(650.0 - origin.x, 50.0 - origin.y, -(origin.z - extent.z))
    for actor in actors:
        actor.set_actor_location(actor.get_actor_location() + offset, False, False)
    origin, extent = leader_actor.get_actor_bounds(False)
    REPORT["grovemaw"] = {
        "leader_mesh": leader_mesh.get_path_name(),
        "part_count": len(meshes),
        "animation": animation.get_path_name(),
        "animation_seconds": animation.get_play_length(),
        "scale": 0.04,
        "leader_bounds": {
            "origin": list(origin.to_tuple()),
            "extent": list(extent.to_tuple()),
            "dimensions_cm": [float(extent.x * 2), float(extent.y * 2), float(extent.z * 2)],
        },
        "materials_populated": all(material_paths(component) and all(material_paths(component)) for component in components),
        "profile_validation": "NOT_RUN; DRG-15 remains fail-closed",
    }


def spawn_static(label, path, x, y, scale):
    mesh = load(path, unreal.StaticMesh)
    actor = ACTORS.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector())
    actor.set_actor_label("WP23_3_QA_" + label)
    component = actor.get_component_by_class(unreal.StaticMeshComponent)
    component.set_static_mesh(mesh)
    bounds = place_on_ground(actor, x, y, scale)
    REPORT["environment"].append({
        "label": label,
        "mesh": mesh.get_path_name(),
        "materials": material_paths(component),
        "body_setup_present": mesh.get_editor_property("body_setup") is not None,
        "scale": scale,
        "bounds": bounds,
    })


def add_lighting_and_floor():
    cube = load("/Engine/BasicShapes/Cube.Cube", unreal.StaticMesh)
    floor = ACTORS.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(250.0, 180.0, -10.0))
    floor.get_component_by_class(unreal.StaticMeshComponent).set_static_mesh(cube)
    floor.set_actor_scale3d(unreal.Vector(20.0, 15.0, 0.2))
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
    component.set_cubemap(load(
        "/Engine/MapTemplates/Sky/DaylightAmbientCubemap.DaylightAmbientCubemap", unreal.TextureCube))
    component.set_intensity(2.0)


def make_capture():
    actor = ACTORS.spawn_actor_from_class(unreal.SceneCapture2D, unreal.Vector())
    component = actor.get_component_by_class(unreal.SceneCaptureComponent2D)
    target = unreal.RenderingLibrary.create_render_target2d(
        actor, 1440, 1080, unreal.TextureRenderTargetFormat.RTF_RGBA8)
    component.set_editor_property("texture_target", target)
    component.set_editor_property("capture_source", unreal.SceneCaptureSource.SCS_FINAL_COLOR_LDR)
    component.set_editor_property("fov_angle", 40.0)
    component.set_editor_property("capture_every_frame", True)
    return actor, component, target


def capture(actor, component, target, name, camera, look_at):
    actor.set_actor_location(unreal.Vector(*camera), False, False)
    actor.set_actor_rotation(
        unreal.MathLibrary.find_look_at_rotation(unreal.Vector(*camera), unreal.Vector(*look_at)), False)
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
    spawn_role(
        "Osk_OldMan",
        DEST + "/Osk/OldMan/TVS_VoxelVillage_OldMan.TVS_VoxelVillage_OldMan",
        DEST + "/Osk/OldMan/Animations/A_OldMan_Idle.A_OldMan_Idle",
        SOURCE / "Villagers/Voxel Village/Textures/Characters/TVS_VoxelVillage_OldMan_Texture.png",
        -220.0)
    spawn_role(
        "Osk_MasterWizard",
        DEST + "/Osk/MasterWizard/TVS_VoxelWizards_MasterWizard.TVS_VoxelWizards_MasterWizard",
        DEST + "/Osk/MasterWizard/Animations/A_MasterWizard_Idle.A_MasterWizard_Idle",
        SOURCE / "Wizards/Voxel Wizards/Textures/Characters/TVS_VoxelWizards_MasterWizard_Texture.png",
        20.0)
    spawn_grovemaw()
    spawn_static("Forest", DEST + "/Environment/Forest/Forest.Forest", 1650.0, 900.0, 0.75)
    spawn_static("RockingPenguin", DEST + "/Environment/Park/Playground_Rocking_Penguin_01/Playground_Rocking_Penguin_01.Playground_Rocking_Penguin_01", 1250.0, 350.0, 100.0)
    spawn_static("Slide", DEST + "/Environment/Park/Playground_Slide/Playground_Slide.Playground_Slide", 1500.0, 450.0, 100.0)
    spawn_static("Swing", DEST + "/Environment/Park/Playground_Swing_01/Playground_Swing_01.Playground_Swing_01", 1800.0, 400.0, 100.0)

    capture_actor, capture_component, target = make_capture()
    REPORT["capture_status"] = "WAITING_FOR_RENDER"
    write_report()
    start = time.monotonic()
    stage = 0
    views = [
        ("01_osk_candidates.png", (-100.0, -480.0, 180.0), (-100.0, 0.0, 90.0)),
        ("02_grovemaw_assembly.png", (1120.0, -720.0, 320.0), (650.0, 50.0, 110.0)),
        ("03_hallowwood_trace.png", (2150.0, -1100.0, 620.0), (1550.0, 600.0, 220.0)),
    ]

    def tick(_delta):
        global stage
        try:
            elapsed = time.monotonic() - start
            if elapsed < 25.0 + stage * 2.0:
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
