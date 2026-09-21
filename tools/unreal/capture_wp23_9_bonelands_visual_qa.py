"""Capture an unsaved WP-23.9 Ossuroth, Kael, and tomb QA fixture."""

import hashlib
import json
from pathlib import Path
import time
import traceback

import unreal


ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
OUT = ROOT / "Saved/Diagnostics/WP23_9_BonelandsVisualQA"
REPORT_PATH = ROOT / "Saved/Diagnostics/WP23_9_bonelands_visual_qa.json"
DEST = "/Game/WYRMFALL/Development/Intake/WP23_9"
DRAGON_BASE = DEST + "/Ossuroth/Skull_Dragon/SkeletalMeshes"
SOURCE = ROOT / "Saved/Diagnostics/WP23_9_Source"
QA_TEXTURES = DEST + "/VisualQA/Textures"
QA_MATERIALS = DEST + "/VisualQA/Materials"

ASSETS = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
ACTORS = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
TOOLS = unreal.AssetToolsHelpers.get_asset_tools()
REPORT = {
    "kind": "wp23_9_bonelands_visual_qa",
    "engine": unreal.SystemLibrary.get_engine_version(),
    "scope": "unsaved rendered intake fixture only; no production map, profile, gameplay, travel, fact, Echo, or save acceptance",
    "capture_status": "INITIALIZING",
    "qa_status": "UNASSESSED",
    "kael_candidates": [],
    "skinning_man_candidates": [],
    "ossuroth": {},
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
    if not source.is_file():
        raise RuntimeError("Missing supplied Kael palette: " + source.as_posix())
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


def ground_actor(actor, x, y, scale):
    actor.set_actor_scale3d(unreal.Vector(scale, scale, scale))
    origin, extent = actor.get_actor_bounds(False)
    current = actor.get_actor_location()
    actor.set_actor_location(
        unreal.Vector(current.x + x - origin.x, current.y + y - origin.y, current.z - (origin.z - extent.z)),
        False, False)
    origin, extent = actor.get_actor_bounds(False)
    return {
        "origin": list(origin.to_tuple()),
        "extent": list(extent.to_tuple()),
        "dimensions_cm": [float(extent.x * 2), float(extent.y * 2), float(extent.z * 2)],
    }


def spawn_skeletal(label, path, x, y, scale, category, texture_source):
    mesh = load(path, unreal.SkeletalMesh)
    actor = ACTORS.spawn_actor_from_class(unreal.SkeletalMeshActor, unreal.Vector())
    actor.set_actor_label("WP23_9_QA_" + label)
    component = actor.get_component_by_class(unreal.SkeletalMeshComponent)
    component.set_skinned_asset_and_update(mesh)
    material = qa_material(label, import_texture(label, texture_source))
    for index in range(component.get_num_materials()):
        component.set_material(index, material)
    bounds = ground_actor(actor, x, y, scale)
    REPORT[category].append({
        "label": label,
        "mesh": mesh.get_path_name(),
        "materials": material_paths(component),
        "scale": scale,
        "bounds": bounds,
        "palette_binding": "SUPPLIED_TEXTURE_QA_MATERIAL",
        "supplied_palette_texture": texture_source.as_posix(),
    })


def spawn_static(label, path, x, y, scale, category="environment", rotation=None):
    mesh = load(path, unreal.StaticMesh)
    actor = ACTORS.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector())
    actor.set_actor_label("WP23_9_QA_" + label)
    component = actor.get_component_by_class(unreal.StaticMeshComponent)
    component.set_static_mesh(mesh)
    if rotation:
        actor.set_actor_rotation(unreal.Rotator(roll=rotation[0], pitch=rotation[1], yaw=rotation[2]), False)
    bounds = ground_actor(actor, x, y, scale)
    REPORT[category].append({
        "label": label,
        "mesh": mesh.get_path_name(),
        "materials": material_paths(component),
        "body_setup_present": mesh.get_editor_property("body_setup") is not None,
        "scale": scale,
        "bounds": bounds,
    })


def spawn_ossuroth():
    actor = ACTORS.spawn_actor_from_class(unreal.WyrmDragonCharacter, unreal.Vector(700.0, 0.0, 200.0))
    actor.set_actor_label("WP23_9_QA_Ossuroth_RuntimeAssembly")
    actor.set_dragon_id(unreal.Name("Ossuroth"))
    actor.set_dragon_form(unreal.WyrmDragonForm.TRUE_FORM)
    components = [actor.get_component_by_class(unreal.SkeletalMeshComponent)] + list(actor.follower_mesh_components)
    if len(components) != 39:
        raise RuntimeError(f"Expected 39 runtime Ossuroth components, got {len(components)}")
    origin, extent = actor.get_actor_bounds(False)
    REPORT["ossuroth"] = {
        "leader_mesh": components[0].get_skinned_asset().get_path_name(),
        "part_count": len(components),
        "scale": 0.036,
        "leader_bounds": {
            "origin": list(origin.to_tuple()),
            "extent": list(extent.to_tuple()),
            "dimensions_cm": [float(extent.x * 2), float(extent.y * 2), float(extent.z * 2)],
        },
        "materials_populated": all(material_paths(component) and all(material_paths(component)) for component in components),
        "profile_validation": "RUNTIME_PROFILE_ASSEMBLY",
    }


def add_lighting_and_floor():
    cube = load("/Engine/BasicShapes/Cube.Cube", unreal.StaticMesh)
    floor = ACTORS.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(1200.0, 500.0, -10.0))
    floor.get_component_by_class(unreal.StaticMeshComponent).set_static_mesh(cube)
    floor.set_actor_scale3d(unreal.Vector(38.0, 24.0, 0.2))
    for rotation, intensity in (((-45.0, -35.0, 0.0), 5.0), ((-25.0, 145.0, 0.0), 2.0)):
        light = ACTORS.spawn_actor_from_class(unreal.DirectionalLight, unreal.Vector(0.0, 0.0, 700.0), unreal.Rotator(*rotation))
        component = light.get_component_by_class(unreal.DirectionalLightComponent)
        component.set_mobility(unreal.ComponentMobility.MOVABLE)
        component.set_intensity(intensity)
    sky = ACTORS.spawn_actor_from_class(unreal.SkyLight, unreal.Vector(0.0, 0.0, 700.0))
    component = sky.get_component_by_class(unreal.SkyLightComponent)
    component.set_mobility(unreal.ComponentMobility.MOVABLE)
    component.set_editor_property("source_type", unreal.SkyLightSourceType.SLS_CAPTURED_SCENE)
    component.set_intensity(1.5)


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
    spawn_skeletal("Kael_Commander", DEST + "/Kael/TVS_VoxelKnights_Commander/TVS_VoxelKnights_Commander.TVS_VoxelKnights_Commander", -250.0, 0.0, 0.45, "kael_candidates", SOURCE / "Kael/TVS_VoxelKnights_Commander_Texture.png")
    spawn_skeletal("Kael_Champion", DEST + "/Kael/TVS_VoxelKnights_Champion/TVS_VoxelKnights_Champion.TVS_VoxelKnights_Champion", 0.0, 0.0, 0.45, "kael_candidates", SOURCE / "Kael/TVS_VoxelKnights_Champion_Texture.png")
    spawn_skeletal("Kael_Crusader", DEST + "/Kael/TVS_VoxelCathedral_Crusader/TVS_VoxelCathedral_Crusader.TVS_VoxelCathedral_Crusader", 250.0, 0.0, 0.50, "kael_candidates", SOURCE / "Cathedral/TVS_VoxelCathedral_Crusader_Texture.png")
    spawn_skeletal("SkinningMan_Butcher", DEST + "/SkinningMan/TVS_VoxelVillage_Butcher/TVS_VoxelVillage_Butcher.TVS_VoxelVillage_Butcher", 600.0, -650.0, 0.50, "skinning_man_candidates", SOURCE / "SkinningMan/TVS_VoxelVillage_Butcher_Texture.png")
    spawn_ossuroth()
    spawn_static("Cathedral", DEST + "/Environment/Cathedral/TVS_VoxelCathedral_Cathedral/TVS_VoxelCathedral_Cathedral.TVS_VoxelCathedral_Cathedral", 2300.0, 800.0, 0.35)
    spawn_static("CrossGrave", DEST + "/Environment/Cathedral/TVS_VoxelCathedral_CrossGrave/TVS_VoxelCathedral_CrossGrave.TVS_VoxelCathedral_CrossGrave", 1600.0, 450.0, 1.5)
    spawn_static("Statue", DEST + "/Environment/Cathedral/TVS_VoxelCathedral_Statue/TVS_VoxelCathedral_Statue.TVS_VoxelCathedral_Statue", 1900.0, 550.0, 0.7)
    cemetery = (
        ("WallSkull", "church-12-cata_wallskull", 1550.0, 1100.0),
        ("Skeleton", "church-4-Skele_mob", 1850.0, 1050.0),
        ("Coffin", "church-55-coffin", 2100.0, 1150.0),
        ("Crypt", "church-67-crypt", 2400.0, 1120.0),
        ("Gargoyle", "church-79-gargoyle", 2700.0, 1050.0),
    )
    for label, stem, x, y in cemetery:
        spawn_static(label, f"{DEST}/Environment/Cemetery/{stem}/{stem}.{stem}", x, y, 100.0)

    capture_actor, capture_component, target = make_capture()
    REPORT["capture_status"] = "WAITING_FOR_RENDER"
    write_report()
    start = time.monotonic()
    stage = 0
    views = [
        ("01_kael_candidates.png", (0.0, -1050.0, 220.0), (0.0, 0.0, 95.0)),
        ("02_ossuroth_assembly.png", (2600.0, -3200.0, 1700.0), (700.0, 0.0, 650.0)),
        ("03_bonelands_tomb_trace.png", (2600.0, -1300.0, 850.0), (2200.0, 850.0, 250.0)),
        ("04_skinning_man_butcher_candidate.png", (600.0, -1200.0, 210.0), (600.0, -650.0, 85.0)),
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
