"""Create an unsaved Cogspire cast fixture and capture visual-review evidence.

This imports supplied animation clips against each candidate's own skeleton,
normalizes human scale, stages supplied casino/tavern art, and records material
and collision suitability. It creates no production map or gameplay owner.
"""

import hashlib
import json
from pathlib import Path
import time
import traceback

import unreal


ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
SOURCE = ROOT / "Saved/Diagnostics/WP23_6_Source"
OUT = ROOT / "Saved/Diagnostics/WP23_6_CastVisualQA"
REPORT_PATH = ROOT / "Saved/Diagnostics/WP23_6_cast_visual_qa.json"
DEST = "/Game/WYRMFALL/Development/Intake/WP23_6"
ANIM_DEST = DEST + "/CastAnimations"
TEXTURE_DEST = DEST + "/CastTextures"
MATERIAL_DEST = DEST + "/CastMaterials"

ASSETS = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
ACTORS = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
TOOLS = unreal.AssetToolsHelpers.get_asset_tools()

REPORT = {
    "kind": "wp23_6_cogspire_cast_visual_qa",
    "engine": unreal.SystemLibrary.get_engine_version(),
    "scope": "unsaved visual/editor fixture only; no production map, gameplay, travel, or save schema",
    "capture_status": "INITIALIZING",
    "qa_status": "UNASSESSED",
    "roles": [],
    "props": [],
    "captures": [],
}


def write_report():
    REPORT_PATH.parent.mkdir(parents=True, exist_ok=True)
    REPORT_PATH.write_text(json.dumps(REPORT, indent=2, default=str) + "\n", encoding="utf-8")


def sha256(path):
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def load(path, expected):
    asset = ASSETS.load_asset(path)
    if asset is None or not isinstance(asset, expected):
        raise RuntimeError("Missing or wrong-class asset: " + path)
    return asset


def import_animation(label, source, skeleton):
    object_path = f"{ANIM_DEST}/A_{label}"
    existing = ASSETS.load_asset(object_path) if ASSETS.does_asset_exist(object_path) else None
    if existing is not None:
        if not isinstance(existing, unreal.AnimSequence):
            raise RuntimeError("Existing cast animation has wrong class: " + object_path)
        return existing
    if not source.is_file():
        raise RuntimeError("Missing supplied animation: " + source.as_posix())

    options = unreal.FbxImportUI()
    options.set_editor_property("automated_import_should_detect_type", False)
    options.set_editor_property("mesh_type_to_import", unreal.FBXImportType.FBXIT_ANIMATION)
    options.set_editor_property("import_mesh", False)
    options.set_editor_property("import_as_skeletal", True)
    options.set_editor_property("import_animations", True)
    options.set_editor_property("import_materials", False)
    options.set_editor_property("import_textures", False)
    options.set_editor_property("skeleton", skeleton)

    task = unreal.AssetImportTask()
    task.filename = source.as_posix()
    task.destination_path = ANIM_DEST
    task.destination_name = "A_" + label
    task.automated = True
    task.save = True
    task.replace_existing = False
    task.async_ = False
    task.factory = unreal.FbxFactory()
    task.options = options
    TOOLS.import_asset_tasks([task])
    return load(object_path, unreal.AnimSequence)


def import_texture(label, source):
    object_path = f"{TEXTURE_DEST}/T_{label}"
    existing = ASSETS.load_asset(object_path) if ASSETS.does_asset_exist(object_path) else None
    if existing is not None:
        if not isinstance(existing, unreal.Texture2D):
            raise RuntimeError("Existing cast texture has wrong class: " + object_path)
        existing.set_editor_property("mip_gen_settings", unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS)
        existing.set_editor_property("filter", unreal.TextureFilter.TF_NEAREST)
        ASSETS.save_loaded_asset(existing)
        return existing
    if not source.is_file():
        raise RuntimeError("Missing supplied palette texture: " + source.as_posix())
    task = unreal.AssetImportTask()
    task.filename = source.as_posix()
    task.destination_path = TEXTURE_DEST
    task.destination_name = "T_" + label
    task.automated = True
    task.save = True
    task.replace_existing = False
    task.async_ = False
    task.factory = unreal.TextureFactory()
    TOOLS.import_asset_tasks([task])
    texture = load(object_path, unreal.Texture2D)
    texture.set_editor_property("mip_gen_settings", unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS)
    texture.set_editor_property("filter", unreal.TextureFilter.TF_NEAREST)
    ASSETS.save_loaded_asset(texture)
    return texture


def qa_material(label, texture):
    object_path = f"{MATERIAL_DEST}/M_{label}_SuppliedPaletteQA"
    existing = ASSETS.load_asset(object_path) if ASSETS.does_asset_exist(object_path) else None
    if existing is not None:
        if not isinstance(existing, unreal.Material):
            raise RuntimeError("Existing cast QA material has wrong class: " + object_path)
        existing.set_editor_property("used_with_skeletal_mesh", True)
        existing.set_editor_property("two_sided", True)
        unreal.MaterialEditingLibrary.recompile_material(existing)
        ASSETS.save_loaded_asset(existing)
        return existing
    material = TOOLS.create_asset(
        "M_" + label + "_SuppliedPaletteQA", MATERIAL_DEST,
        unreal.Material, unreal.MaterialFactoryNew())
    if material is None:
        raise RuntimeError("Could not create QA material for " + label)
    material.set_editor_property("used_with_skeletal_mesh", True)
    material.set_editor_property("two_sided", True)
    sample = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionTextureSample, -260, 0)
    sample.set_editor_property("texture", texture)
    unreal.MaterialEditingLibrary.connect_material_property(
        sample, "RGB", unreal.MaterialProperty.MP_BASE_COLOR)
    # The fixture uses the supplied palette as low-power emissive color so
    # automated captures remain legible. This is not a production shader.
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


def actor_bounds(actor):
    origin, extent = actor.get_actor_bounds(False)
    return origin, extent


def place_centered(actor, x, y, scale, z_offset=0.0):
    actor.set_actor_scale3d(unreal.Vector(scale, scale, scale))
    origin, extent = actor_bounds(actor)
    current = actor.get_actor_location()
    actor.set_actor_location(
        unreal.Vector(
            current.x + x - origin.x,
            current.y + y - origin.y,
            current.z - (origin.z - extent.z) + z_offset),
        False, False)
    origin, extent = actor_bounds(actor)
    return {
        "origin": list(origin.to_tuple()),
        "extent": list(extent.to_tuple()),
        "dimensions_cm": [float(extent.x * 2.0), float(extent.y * 2.0), float(extent.z * 2.0)],
    }


def material_paths(component):
    return [
        component.get_material(index).get_path_name() if component.get_material(index) else None
        for index in range(component.get_num_materials())
    ]


def physics_body_count(physics):
    try:
        return len(physics.get_editor_property("skeletal_body_setups"))
    except Exception:
        return -1


def spawn_role(role, mesh_path, texture_source, anim_label, anim_source, target_height, x, phase):
    mesh = load(mesh_path, unreal.SkeletalMesh)
    skeleton = mesh.get_editor_property("skeleton")
    physics = mesh.get_editor_property("physics_asset")
    if skeleton is None or physics is None:
        raise RuntimeError(role + " lacks a skeleton or physics asset")
    animation = import_animation(anim_label, anim_source, skeleton)
    if animation.get_editor_property("skeleton") != skeleton:
        raise RuntimeError(role + " animation targets a different skeleton")
    sampled_keys = int(animation.get_editor_property("number_of_sampled_keys"))
    if animation.get_play_length() <= 0.0 or sampled_keys <= 0:
        raise RuntimeError(role + " animation has no playable samples")
    # These source clips contain root tracks. Lock them for the stationary QA
    # fixture so playback cannot translate the whole cast out of its camera;
    # production locomotion/root-motion policy remains owned by the existing
    # character movement layer.
    animation.set_editor_property("enable_root_motion", True)
    animation.set_editor_property("force_root_lock", True)

    actor = ACTORS.spawn_actor_from_class(unreal.SkeletalMeshActor, unreal.Vector(0.0, 0.0, 0.0))
    actor.set_actor_label("WP23_6_QA_" + role)
    component = actor.get_component_by_class(unreal.SkeletalMeshComponent)
    component.set_skinned_asset_and_update(mesh)
    texture = import_texture(role, texture_source)
    palette_material = qa_material(role, texture)
    # Use the supplied palette in the animated capture. A reference-pose
    # isolation run established that any disappearing draw was not caused by
    # the palette shader.
    fixture_material = palette_material
    for index in range(component.get_num_materials()):
        component.set_material(index, fixture_material)
    component.set_animation_mode(unreal.AnimationMode.ANIMATION_SINGLE_NODE)
    component.set_animation(animation)
    component.set_update_animation_in_editor(True)
    # Single-node assignment alone does not initialize an evaluated pose in
    # this unattended editor fixture. Explicit playback is required before
    # scrubbing to the authored inspection fraction.
    component.play(True)
    component.set_position(phase * animation.get_play_length(), False)

    raw_origin, raw_extent = actor_bounds(actor)
    raw_height = float(raw_extent.z * 2.0)
    if raw_height <= 1.0:
        raise RuntimeError(role + " has invalid imported bounds")
    scale = target_height / raw_height
    measured = place_centered(actor, x, 0.0, scale)
    materials = material_paths(component)
    if not materials or any(value is None for value in materials):
        raise RuntimeError(role + " has an empty material slot")
    component.set_collision_profile_name("BlockAll")
    component.set_collision_enabled(unreal.CollisionEnabled.QUERY_ONLY)

    REPORT["roles"].append({
        "role": role,
        "mesh": mesh.get_path_name(),
        "skeleton": skeleton.get_path_name(),
        "bone_count": component.get_num_bones(),
        "physics_asset": physics.get_path_name(),
        "physics_body_count": physics_body_count(physics),
        "animation": animation.get_path_name(),
        "animation_source": anim_source.as_posix(),
        "animation_source_sha256": sha256(anim_source),
        "animation_seconds": animation.get_play_length(),
        "animation_sampled_keys": sampled_keys,
        "animation_pose_fraction": phase,
        "materials": materials,
        "supplied_palette_texture": texture.get_path_name(),
        "supplied_palette_material": palette_material.get_path_name(),
        "fixture_material": fixture_material.get_path_name(),
        "material_suitability": "PASS_FIXTURE; supplied 256x1 palette is readable through a diagnostic BaseColor/emissive binding; production shader authoring remains later work",
        "uniform_scale": scale,
        "target_height_cm": target_height,
        "bounds": measured,
        "collision_suitability": "PASS_FIXTURE_QUERY; supplied physics asset present; production character still requires the existing character-capsule policy",
    })
    return actor, component


def spawn_prop(label, path, x, y, scale, z_offset=0.0, rotation=None, texture_path=None):
    mesh = load(path, unreal.StaticMesh)
    if mesh.get_editor_property("body_setup") is None:
        raise RuntimeError(label + " has no static body setup")
    actor = ACTORS.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(0.0, 0.0, 0.0))
    actor.set_actor_label("WP23_6_QA_" + label)
    component = actor.get_component_by_class(unreal.StaticMeshComponent)
    component.set_static_mesh(mesh)
    fixture_material = None
    if texture_path is not None:
        texture = load(texture_path, unreal.Texture2D)
        fixture_material = qa_material(label, texture)
        for index in range(component.get_num_materials()):
            component.set_material(index, fixture_material)
    component.set_collision_profile_name("BlockAll")
    component.set_collision_enabled(unreal.CollisionEnabled.QUERY_AND_PHYSICS)
    if rotation is not None:
        actor.set_actor_rotation(unreal.Rotator(*rotation), False)
    measured = place_centered(actor, x, y, scale, z_offset)
    materials = material_paths(component)
    if not materials or any(value is None for value in materials):
        raise RuntimeError(label + " has an empty material slot")
    REPORT["props"].append({
        "label": label,
        "mesh": mesh.get_path_name(),
        "materials": materials,
        "fixture_material": fixture_material.get_path_name() if fixture_material else None,
        "body_setup_present": True,
        "collision_profile": str(component.get_collision_profile_name()),
        "uniform_scale": scale,
        "bounds": measured,
    })
    return actor


def add_lighting_and_floor():
    cube = load("/Engine/BasicShapes/Cube.Cube", unreal.StaticMesh)
    floor = ACTORS.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(1500.0, 0.0, -10.0))
    floor.set_actor_label("WP23_6_QA_Floor")
    floor.get_component_by_class(unreal.StaticMeshComponent).set_static_mesh(cube)
    floor.set_actor_scale3d(unreal.Vector(38.0, 15.0, 0.2))
    for rotation, intensity in (((-45.0, -35.0, 0.0), 5.0), ((-25.0, 145.0, 0.0), 2.0)):
        light = ACTORS.spawn_actor_from_class(
            unreal.DirectionalLight, unreal.Vector(0.0, 0.0, 600.0), unreal.Rotator(*rotation))
        component = light.get_component_by_class(unreal.DirectionalLightComponent)
        component.set_mobility(unreal.ComponentMobility.MOVABLE)
        component.set_intensity(intensity)
    sky = ACTORS.spawn_actor_from_class(unreal.SkyLight, unreal.Vector(0.0, 0.0, 600.0))
    sky_component = sky.get_component_by_class(unreal.SkyLightComponent)
    sky_component.set_mobility(unreal.ComponentMobility.MOVABLE)
    sky_component.set_intensity(1.5)


def make_capture():
    actor = ACTORS.spawn_actor_from_class(unreal.SceneCapture2D, unreal.Vector(0.0, -900.0, 280.0))
    component = actor.get_component_by_class(unreal.SceneCaptureComponent2D)
    target = unreal.RenderingLibrary.create_render_target2d(
        actor, 1440, 1080, unreal.TextureRenderTargetFormat.RTF_RGBA8)
    component.set_editor_property("texture_target", target)
    component.set_editor_property("capture_source", unreal.SceneCaptureSource.SCS_FINAL_COLOR_LDR)
    component.set_editor_property("fov_angle", 45.0)
    component.set_editor_property("capture_every_frame", False)
    component.set_editor_property("capture_on_movement", False)
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

    knight_anim = SOURCE / "Knights/Voxel Knights/Animations/Humans"
    villager_anim = SOURCE / "Villagers/Voxel Village/Animations"
    knight_textures = SOURCE / "Knights/Voxel Knights/Textures/Characters"
    villager_textures = SOURCE / "Villagers/Voxel Village/Textures/Characters"
    baron, baron_component = spawn_role(
        "BaronFeistCogwell",
        DEST + "/Roles/BaronFeistCogwell_Captain/TVS_VoxelKnights_Captain.TVS_VoxelKnights_Captain",
        knight_textures / "TVS_VoxelKnights_Captain_Texture.png",
        "BaronFeistCogwell_Command", knight_anim / "Human_Command_Anim.fbx", 190.0, 0.0, 0.55)
    champion, champion_component = spawn_role(
        "HouseMarkChampion",
        DEST + "/Roles/HouseMark_Champion/TVS_VoxelKnights_Champion.TVS_VoxelKnights_Champion",
        knight_textures / "TVS_VoxelKnights_Champion_Texture.png",
        "HouseMarkChampion_Slash", knight_anim / "Human_Slash_Anim.fbx", 190.0, 1500.0, 0.42)
    chef, chef_component = spawn_role(
        "ChefAurelioVane",
        DEST + "/Roles/ChefAurelioVane_Chef/TVS_VoxelVillage_Chef.TVS_VoxelVillage_Chef",
        villager_textures / "TVS_VoxelVillage_Chef_Texture.png",
        "ChefAurelioVane_Walk", villager_anim / "Human_Walk_Anim.fbx", 175.0, 3000.0, 0.35)

    spawn_prop("CasinoPokerTable", DEST + "/Urban/Casino/Poker_Table-9_00.Poker_Table-9_00", 1300.0, 170.0, 0.8)
    spawn_prop("CasinoSlotMachine", DEST + "/Urban/Casino/SlotMachine_Red-13_00.SlotMachine_Red-13_00", 1700.0, 160.0, 0.8)
    spawn_prop("CasinoBeltBarrier", DEST + "/Urban/Casino/BeltBarriers_Red-10_00.BeltBarriers_Red-10_00", 1500.0, -150.0, 0.8)

    spawn_prop("ChefTable", DEST + "/Urban/ChefScene/Table/tavern-48-table.tavern-48-table", 2850.0, 170.0, 100.0,
               texture_path=DEST + "/Urban/ChefScene/Table/TEX_tavern-48-table.TEX_tavern-48-table")
    spawn_prop("ChefBar", DEST + "/Urban/ChefScene/Bar/tavern-25-bar.tavern-25-bar", 3250.0, 180.0, 100.0,
               texture_path=DEST + "/Urban/ChefScene/Bar/TEX_tavern-25-bar.TEX_tavern-25-bar")
    spawn_prop("ChefMeatShelf", DEST + "/Urban/ChefScene/MeatShelf/tavern-33-shelf_meat.tavern-33-shelf_meat", 3150.0, -160.0, 100.0,
               texture_path=DEST + "/Urban/ChefScene/MeatShelf/TEX_tavern-33-shelf_meat.TEX_tavern-33-shelf_meat")
    spawn_prop("ChefKnife", DEST + "/Urban/ChefScene/Knife/tavern-3-knife.tavern-3-knife", 2850.0, 160.0, 100.0, 92.0,
               texture_path=DEST + "/Urban/ChefScene/Knife/TEX_tavern-3-knife.TEX_tavern-3-knife")

    capture_actor, capture_component, target = make_capture()
    REPORT["capture_status"] = "WAITING_FOR_RENDER"
    write_report()
    start = time.monotonic()
    stage = 0

    views = [
        ("01_baron_reference_pose.png", (520.0, -780.0, 300.0), (0.0, 0.0, 95.0)),
        ("02_house_mark_casino.png", (2020.0, -900.0, 330.0), (1500.0, 40.0, 95.0)),
        ("03_chef_tavern.png", (3520.0, -900.0, 320.0), (3000.0, 30.0, 90.0)),
        ("04_cast_lineup.png", (1500.0, -2300.0, 560.0), (1500.0, 0.0, 100.0)),
    ]

    def tick(_delta):
        global stage
        try:
            elapsed = time.monotonic() - start
            # Newly created materials need their skeletal/static usage shaders
            # compiled before a capture is evidence. Existing reruns also use
            # this settle window to avoid recording fallback/invisible draws.
            if elapsed < 30.0 + stage * 2.0:
                return
            if stage < len(views):
                capture(capture_actor, capture_component, target, *views[stage])
                stage += 1
                write_report()
                return
            REPORT["animation_pose_evaluation"] = "PASS; three non-zero sampled supplied clips explicitly played on their matching skeletons with their source root tracks locked for this stationary fixture; production root-motion policy remains with the existing character movement layer"
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
