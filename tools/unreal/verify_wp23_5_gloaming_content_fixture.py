"""Build and capture the bounded WP-23.5 Gloaming content fixture.

This uses an unsaved editor map and ignored Development/Intake assets. It does
not create the production Gloaming region or gameplay actors. Cathedral roles
use their supplied skeletal meshes and compatible supplied animation clips.
The representative horror actor, Michael Mire, deliberately keeps the source
FBX's modular static pieces and proves a capsule-proxy/procedural-root strategy.
"""

import hashlib
import json
from pathlib import Path
import time
import traceback

import unreal


ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
SOURCE_ROOT = ROOT / "Saved/Diagnostics/WP23_5_Source/Cathedral/Voxel Cathedral"
DEST = "/Game/WYRMFALL/Development/Intake/WP23_5/Gloaming"
HORROR_DEST = "/Game/WYRMFALL/Development/Intake/WP23_5/Horror"
ANIM_DEST = DEST + "/RoleAnimations"
TEXTURE_DEST = DEST + "/FixtureTextures"
MATERIAL_DEST = DEST + "/FixtureMaterials"
OUT = ROOT / "Saved/Diagnostics/WP23_5_GloamingContentFixture"
REPORT_PATH = ROOT / "Saved/Diagnostics/WP23_5_gloaming_content_fixture.json"

ACTORS = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
ASSETS = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
TOOLS = unreal.AssetToolsHelpers.get_asset_tools()
WORLD = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
COLLISION_PROXIES = []

ROLE_SPECS = (
    ("Ashgrave", "Roles/Ashgrave/TVS_VoxelCathedral_Crusader", "TVS_VoxelCathedral_Crusader_Texture.png", "Human_Idle_Anim.fbx", "Idle", (-360.0, 40.0, 0.0), 0.0),
    ("CountMalvaine", "Roles/CountMalvaine/TVS_VoxelCathedral_Priest", "TVS_VoxelCathedral_Priest_Texture.png", "Human_Preach_Anim.fbx", "Preach", (-120.0, 40.0, 0.0), 0.2),
    ("HollowTwinMorrow", "Roles/HollowTwins/TVS_VoxelCathedral_Nun", "TVS_VoxelCathedral_Nun_Texture.png", "Human_Praying_Anim.fbx", "Praying", (120.0, 40.0, 0.0), 0.0),
    ("HollowTwinMourn", "Roles/HollowTwins/TVS_VoxelCathedral_Nun", "TVS_VoxelCathedral_Nun_Texture.png", "Human_Praying_Anim.fbx", "Praying", (360.0, 40.0, 0.0), 0.65),
)
TARGET_HEIGHT_CM = 180.0
HORROR_SOURCE_TOKEN = "MicahelMeyers"
HORROR_ALIAS = "Michael Mire"
HORROR_TARGET_HEIGHT_CM = 190.0

REPORT = {
    "kind": "wp23_5_gloaming_content_integration_fixture_not_production_region",
    "engine": unreal.SystemLibrary.get_engine_version(),
    "status": "STARTING",
    "scope": "unsaved editor fixture; no production map, encounter, Echo, route, travel, or save acceptance",
    "cathedral_roles": [],
    "horror_actor": {},
    "collision_traces": [],
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


def import_animation(role, source_name, clip_name, skeleton):
    object_path = f"{ANIM_DEST}/A_{role}_{clip_name}"
    existing = ASSETS.load_asset(object_path) if ASSETS.does_asset_exist(object_path) else None
    if existing is not None:
        if not isinstance(existing, unreal.AnimSequence):
            raise RuntimeError("Existing fixture animation has wrong class: " + object_path)
        return existing

    source = SOURCE_ROOT / "Animations" / source_name
    if not source.is_file():
        raise RuntimeError("Missing extracted supplied animation: " + source.as_posix())
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
    task.destination_name = f"A_{role}_{clip_name}"
    task.automated = True
    task.save = True
    task.replace_existing = False
    task.async_ = False
    task.factory = unreal.FbxFactory()
    task.options = options
    TOOLS.import_asset_tasks([task])
    return load(object_path, unreal.AnimSequence)


def fixture_material(role, texture_name):
    texture_path = f"{TEXTURE_DEST}/T_{role}"
    texture = ASSETS.load_asset(texture_path) if ASSETS.does_asset_exist(texture_path) else None
    if texture is None:
        source = SOURCE_ROOT / "Textures/Characters" / texture_name
        if not source.is_file():
            raise RuntimeError("Missing supplied Cathedral texture: " + source.as_posix())
        factory = unreal.TextureFactory()
        factory.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_EDITOR_ICON)
        factory.set_editor_property("mip_gen_settings", unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS)
        task = unreal.AssetImportTask()
        task.filename = source.as_posix()
        task.destination_path = TEXTURE_DEST
        task.destination_name = "T_" + role
        task.automated = True
        task.save = True
        task.replace_existing = False
        task.async_ = False
        task.factory = factory
        TOOLS.import_asset_tasks([task])
        texture = load(texture_path, unreal.Texture2D)
    elif not isinstance(texture, unreal.Texture2D):
        raise RuntimeError("Fixture palette has wrong class: " + texture_path)

    material_path = f"{MATERIAL_DEST}/M_{role}_SuppliedPaletteV2"
    material = ASSETS.load_asset(material_path) if ASSETS.does_asset_exist(material_path) else None
    if material is None:
        material = TOOLS.create_asset(
            "M_" + role + "_SuppliedPaletteV2", MATERIAL_DEST,
            unreal.Material, unreal.MaterialFactoryNew())
        if material is None:
            raise RuntimeError("Could not create fixture material for " + role)
        sample = unreal.MaterialEditingLibrary.create_material_expression(
            material, unreal.MaterialExpressionTextureSample, -240, 0)
        sample.set_editor_property("texture", texture)
        unreal.MaterialEditingLibrary.connect_material_property(
            sample, "RGB", unreal.MaterialProperty.MP_BASE_COLOR)
        unreal.MaterialEditingLibrary.connect_material_property(
            sample, "RGB", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
        roughness = unreal.MaterialEditingLibrary.create_material_expression(
            material, unreal.MaterialExpressionConstant, -240, 160)
        roughness.set_editor_property("r", 0.9)
        unreal.MaterialEditingLibrary.connect_material_property(
            roughness, "", unreal.MaterialProperty.MP_ROUGHNESS)
        unreal.MaterialEditingLibrary.recompile_material(material)
        ASSETS.save_loaded_asset(material)
    return material


def set_animation(component, animation, phase):
    component.set_animation_mode(unreal.AnimationMode.ANIMATION_SINGLE_NODE)
    component.set_animation(animation)
    component.set_update_animation_in_editor(True)
    component.set_position(float(phase) * max(animation.get_play_length(), 0.01), False)
    component.play(True)


def bounds_dict(actor):
    origin, extent = actor.get_actor_bounds(False)
    return {
        "origin": list(origin.to_tuple()),
        "extent": list(extent.to_tuple()),
        "height_cm": float(extent.z * 2.0),
    }


def scale_and_ground(actor, target_height, x, y):
    initial = bounds_dict(actor)
    if initial["height_cm"] <= 1.0:
        raise RuntimeError("Actor has unusable bounds: " + actor.get_actor_label())
    scale = target_height / initial["height_cm"]
    actor.set_actor_scale3d(unreal.Vector(scale, scale, scale))
    actor.set_actor_location(unreal.Vector(x, y, 0.0), False, False)
    grounded = bounds_dict(actor)
    actor.set_actor_location(unreal.Vector(x, y, -float(grounded["origin"][2] - grounded["extent"][2])), False, False)
    return scale, bounds_dict(actor)


def create_collision_proxy(label, x, y, height, radius):
    cube = load("/Engine/BasicShapes/Cube.Cube", unreal.StaticMesh)
    proxy = ACTORS.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(x, y, height * 0.5))
    proxy.set_actor_label("COLLISION_" + label)
    component = proxy.get_component_by_class(unreal.StaticMeshComponent)
    component.set_static_mesh(cube)
    component.set_collision_profile_name("BlockAll")
    component.set_visibility(False)
    component.set_hidden_in_game(True)
    proxy.set_actor_scale3d(unreal.Vector(radius * 2.0 / 100.0, radius * 2.0 / 100.0, height / 100.0))
    COLLISION_PROXIES.append(proxy)
    return proxy


def material_paths(component):
    return [
        component.get_material(index).get_path_name() if component.get_material(index) else None
        for index in range(component.get_num_materials())
    ]


def spawn_role(role, mesh_name, texture_name, source_anim, clip_name, location, phase):
    asset_name = mesh_name.rsplit("/", 1)[-1]
    mesh = load(f"{DEST}/{mesh_name}.{asset_name}", unreal.SkeletalMesh)
    skeleton = mesh.get_editor_property("skeleton")
    physics = mesh.get_editor_property("physics_asset")
    if skeleton is None or physics is None:
        raise RuntimeError(role + " lacks a skeleton or physics asset")
    animation = import_animation(role, source_anim, clip_name, skeleton)
    if animation.get_editor_property("skeleton") != skeleton:
        raise RuntimeError(role + " animation does not target its supplied skeleton")

    actor = ACTORS.spawn_actor_from_class(unreal.SkeletalMeshActor, unreal.Vector(*location))
    actor.set_actor_label("GLM_" + role)
    component = actor.get_component_by_class(unreal.SkeletalMeshComponent)
    component.set_skinned_asset_and_update(mesh)
    material = fixture_material(role, texture_name)
    for index in range(component.get_num_materials()):
        component.set_material(index, material)
    component.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
    set_animation(component, animation, phase)
    scale, measured = scale_and_ground(actor, TARGET_HEIGHT_CM, location[0], location[1])
    materials = material_paths(component)
    if not materials or any(value is None for value in materials):
        raise RuntimeError(role + " has an empty material slot")
    create_collision_proxy(role, location[0], location[1], measured["height_cm"], 34.0)
    row = {
        "role": role,
        "mesh": mesh.get_path_name(),
        "skeleton": skeleton.get_path_name(),
        "physics_asset": physics.get_path_name(),
        "animation": animation.get_path_name(),
        "animation_seconds": animation.get_play_length(),
        "materials": materials,
        "uniform_scale": scale,
        "bounds": measured,
        "collision_strategy": "hidden BlockAll 68cm-wide body proxy; supplied physics asset retained for later gameplay actor",
    }
    REPORT["cathedral_roles"].append(row)
    return actor


def combined_bounds(actors):
    minimum = unreal.Vector(1e12, 1e12, 1e12)
    maximum = unreal.Vector(-1e12, -1e12, -1e12)
    for actor in actors:
        origin, extent = actor.get_actor_bounds(False)
        minimum.x = min(minimum.x, origin.x - extent.x)
        minimum.y = min(minimum.y, origin.y - extent.y)
        minimum.z = min(minimum.z, origin.z - extent.z)
        maximum.x = max(maximum.x, origin.x + extent.x)
        maximum.y = max(maximum.y, origin.y + extent.y)
        maximum.z = max(maximum.z, origin.z + extent.z)
    return minimum, maximum


def spawn_horror_assembly(anchor):
    paths = [
        path for path in ASSETS.list_assets(HORROR_DEST, recursive=False, include_folder=False)
        if HORROR_SOURCE_TOKEN.lower() in path.lower()
    ]
    if len(paths) < 12:
        raise RuntimeError(f"Expected a complete {HORROR_SOURCE_TOKEN} modular set, found {len(paths)} parts")
    parts = []
    all_materials = []
    source_minimum = unreal.Vector(1e12, 1e12, 1e12)
    source_maximum = unreal.Vector(-1e12, -1e12, -1e12)
    for index, path in enumerate(sorted(paths)):
        mesh = load(path, unreal.StaticMesh)
        box = mesh.get_bounding_box()
        source_minimum.x = min(source_minimum.x, box.min.x)
        source_minimum.y = min(source_minimum.y, box.min.y)
        source_minimum.z = min(source_minimum.z, box.min.z)
        source_maximum.x = max(source_maximum.x, box.max.x)
        source_maximum.y = max(source_maximum.y, box.max.y)
        source_maximum.z = max(source_maximum.z, box.max.z)
        actor = ACTORS.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(*anchor))
        actor.set_actor_label(f"GLM_MichaelMire_Part_{index:02d}")
        component = actor.get_component_by_class(unreal.StaticMeshComponent)
        component.set_static_mesh(mesh)
        component.set_collision_profile_name("NoCollision")
        component.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
        materials = material_paths(component)
        if not materials or any(value is None for value in materials):
            raise RuntimeError("Michael Mire part has an empty material slot: " + path)
        all_materials.extend(materials)
        parts.append(actor)

    initial_height = source_maximum.z - source_minimum.z
    if initial_height <= 0.001:
        REPORT["horror_actor"] = {
            "alias": HORROR_ALIAS,
            "part_count": len(parts),
            "diagnostic_source_minimum": list(source_minimum.to_tuple()),
            "diagnostic_source_maximum": list(source_maximum.to_tuple()),
        }
        raise RuntimeError("Michael Mire modular assembly has unusable source bounds")
    scale = HORROR_TARGET_HEIGHT_CM / initial_height
    source_center_x = (source_minimum.x + source_maximum.x) * 0.5
    source_center_y = (source_minimum.y + source_maximum.y) * 0.5
    assembled_location = unreal.Vector(
        anchor[0] - source_center_x * scale,
        anchor[1] - source_center_y * scale,
        anchor[2] - source_minimum.z * scale)
    for actor in parts:
        actor.set_actor_scale3d(unreal.Vector(scale, scale, scale))
        actor.set_actor_location(assembled_location, False, False)
    minimum = unreal.Vector(
        anchor[0] + (source_minimum.x - source_center_x) * scale,
        anchor[1] + (source_minimum.y - source_center_y) * scale,
        0.0)
    maximum = unreal.Vector(
        anchor[0] + (source_maximum.x - source_center_x) * scale,
        anchor[1] + (source_maximum.y - source_center_y) * scale,
        HORROR_TARGET_HEIGHT_CM)
    create_collision_proxy("MichaelMire", anchor[0], anchor[1], HORROR_TARGET_HEIGHT_CM, 38.0)
    REPORT["horror_actor"] = {
        "alias": HORROR_ALIAS,
        "source_identity_token": HORROR_SOURCE_TOKEN,
        "part_count": len(parts),
        "mesh_paths": sorted(paths),
        "material_paths": sorted(set(all_materials)),
        "uniform_scale": scale,
        "bounds": {
            "minimum": list(minimum.to_tuple()),
            "maximum": list(maximum.to_tuple()),
            "height_cm": maximum.z - minimum.z,
        },
        "animation_strategy": "authored procedural root idle for rigid modular parts; skeletal locomotion is not claimed",
        "collision_strategy": "hidden BlockAll 76cm-wide body proxy; all visual body parts use NoCollision",
    }
    return parts


def trace_proxy(label, x, y):
    world = WORLD.get_editor_world()
    result = unreal.SystemLibrary.line_trace_single(
        world, unreal.Vector(x, y, 300.0), unreal.Vector(x, y, -50.0),
        unreal.TraceTypeQuery.TRACE_TYPE_QUERY1, False, [],
        unreal.DrawDebugTrace.NONE, True)
    parts = result.to_tuple()
    hit = bool(parts[0])
    hit_actor = parts[9].get_actor_label() if hit and parts[9] else None
    REPORT["collision_traces"].append({"subject": label, "hit": hit, "actor": hit_actor})
    if not hit or hit_actor != "COLLISION_" + label:
        raise RuntimeError(f"Collision proxy trace failed for {label}: {hit_actor}")


def capture(component, target, name):
    component.capture_scene()
    unreal.RenderingLibrary.export_render_target(component, target, OUT.as_posix(), name)
    path = OUT / name
    if not path.is_file() or path.stat().st_size == 0:
        raise RuntimeError("Capture missing: " + path.as_posix())
    REPORT["captures"].append({"path": str(path), "sha256": sha256(path)})


try:
    OUT.mkdir(parents=True, exist_ok=True)
    unreal.EditorLoadingAndSavingUtils.new_blank_map(False)

    for spec in ROLE_SPECS:
        spawn_role(*spec)
    horror_parts = spawn_horror_assembly((0.0, 390.0, 0.0))

    floor = ACTORS.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(0.0, 180.0, -8.0))
    floor.set_actor_label("GLM_FixtureFloor")
    floor.get_component_by_class(unreal.StaticMeshComponent).set_static_mesh(
        load("/Engine/BasicShapes/Cube.Cube", unreal.StaticMesh))
    floor.set_actor_scale3d(unreal.Vector(12.0, 9.0, 0.12))

    # Directional lights use physical lux in this UE configuration. The first
    # capture deliberately failed review when fixture-era values of 5/2 lux
    # rendered the supplied palettes nearly black.
    for rotation, intensity in (((-42.0, -35.0, 0.0), 50.0), ((-20.0, 145.0, 0.0), 10.0)):
        light = ACTORS.spawn_actor_from_class(
            unreal.DirectionalLight, unreal.Vector(0.0, 0.0, 600.0), unreal.Rotator(*rotation))
        component = light.get_component_by_class(unreal.DirectionalLightComponent)
        component.set_mobility(unreal.ComponentMobility.MOVABLE)
        component.set_intensity(intensity)
    sky = ACTORS.spawn_actor_from_class(unreal.SkyLight, unreal.Vector(0.0, 0.0, 600.0))
    sky.get_component_by_class(unreal.SkyLightComponent).set_intensity(1.2)
    front_fill = ACTORS.spawn_actor_from_class(
        unreal.PointLight, unreal.Vector(600.0, -700.0, 480.0))
    front_fill_component = front_fill.get_component_by_class(unreal.PointLightComponent)
    front_fill_component.set_mobility(unreal.ComponentMobility.MOVABLE)
    front_fill_component.set_intensity(3000.0)
    front_fill_component.set_attenuation_radius(3000.0)
    front_fill_component.set_cast_shadows(False)

    for role, _, _, _, _, location, _ in ROLE_SPECS:
        trace_proxy(role, location[0], location[1])
    trace_proxy("MichaelMire", 0.0, 390.0)
    for proxy in COLLISION_PROXIES:
        ACTORS.destroy_actor(proxy)

    camera = ACTORS.spawn_actor_from_class(unreal.SceneCapture2D, unreal.Vector(950.0, -1250.0, 520.0))
    camera_component = camera.get_component_by_class(unreal.SceneCaptureComponent2D)
    target = unreal.RenderingLibrary.create_render_target2d(camera, 1600, 900, unreal.TextureRenderTargetFormat.RTF_RGBA8)
    camera_component.set_editor_property("texture_target", target)
    camera_component.set_editor_property("capture_source", unreal.SceneCaptureSource.SCS_FINAL_COLOR_LDR)
    camera_component.set_editor_property("fov_angle", 48.0)
    camera_component.set_editor_property("capture_every_frame", False)
    camera_component.set_editor_property("capture_on_movement", False)

    start = time.monotonic()
    stage = 0
    REPORT["status"] = "WAITING_FOR_CAPTURE"
    write_report()

    def tick(_delta):
        global stage
        try:
            elapsed = time.monotonic() - start
            # The rigid modular source has no skeleton. Move the complete set
            # together to prove the selected procedural-root idle strategy.
            yaw = 2.5 if int(elapsed * 2.0) % 2 == 0 else -2.5
            for part in horror_parts:
                part.set_actor_rotation(unreal.Rotator(0.0, yaw, 0.0), False)
            if stage == 0 and elapsed > 7.0:
                camera.set_actor_location(unreal.Vector(950.0, -1250.0, 520.0), False, False)
                camera.set_actor_rotation(unreal.MathLibrary.find_look_at_rotation(
                    camera.get_actor_location(), unreal.Vector(0.0, 170.0, 100.0)), False)
                capture(camera_component, target, "01_cathedral_roles_and_horror.png")
                stage = 1
            elif stage == 1 and elapsed > 9.0:
                camera.set_actor_location(unreal.Vector(520.0, 20.0, 250.0), False, False)
                camera.set_actor_rotation(unreal.MathLibrary.find_look_at_rotation(
                    camera.get_actor_location(), unreal.Vector(0.0, 390.0, 95.0)), False)
                capture(camera_component, target, "02_michael_mire_assembly.png")
                REPORT["horror_actor"]["observed_root_idle_yaw_degrees"] = yaw
                REPORT["status"] = "PASS_AUTOMATED_REQUIRES_MANUAL_VISUAL_REVIEW"
                write_report()
                unreal.unregister_slate_post_tick_callback(handle)
                unreal.SystemLibrary.quit_editor()
        except Exception:
            REPORT["status"] = "ERROR"
            REPORT["error"] = traceback.format_exc()
            write_report()
            unreal.unregister_slate_post_tick_callback(handle)
            unreal.SystemLibrary.quit_editor()

    handle = unreal.register_slate_post_tick_callback(tick)
except Exception:
    REPORT["status"] = "ERROR"
    REPORT["error"] = traceback.format_exc()
    write_report()
    unreal.SystemLibrary.quit_editor()
