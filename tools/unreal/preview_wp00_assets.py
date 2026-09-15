"""Fresh-editor-only native asset preview; unsaved map, no gameplay acceptance.

Run via startup ExecCmds so Slate callbacks can finish render-target captures.
Uses real supplied textures and imported parts. Scales are diagnostic, not design approval.
"""
import json
from pathlib import Path
import time
import traceback
import unreal

ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
OUT = ROOT / "Saved/Diagnostics/WP00Preview"
OUT.mkdir(parents=True, exist_ok=True)
DATA = json.loads((ROOT / "Saved/Diagnostics/WP00_asset_review.json").read_text())
REPORT = {"kind": "native_rendered_asset_preview_not_gameplay", "status": "STARTING", "actors": [], "captures": []}
ACTORS = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
ASSETS = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
MAT = unreal.MaterialEditingLibrary
COMPONENTS = []


def write():
    (OUT / "report.json").write_text(json.dumps(REPORT, indent=2), encoding="utf-8")


def palette(label):
    folder = "/Game/WYRMFALL/Development/Intake/WP00/Review/" + label + "Texture"
    texture = next(ASSETS.load_asset(p) for p in ASSETS.list_assets(folder, True, False)
                   if isinstance(ASSETS.load_asset(p), unreal.Texture2D))
    path = "/Game/WYRMFALL/Development/Intake/WP00/Review/Materials/M_" + label
    material = ASSETS.load_asset(path) if ASSETS.does_asset_exist(path) else None
    if not material:
        material = unreal.AssetToolsHelpers.get_asset_tools().create_asset("M_" + label,
            path.rsplit("/", 1)[0], unreal.Material, unreal.MaterialFactoryNew())
        sample = MAT.create_material_expression(material, unreal.MaterialExpressionTextureSample, -300, 0)
        sample.set_editor_property("texture", texture)
        MAT.connect_material_property(sample, "RGB", unreal.MaterialProperty.MP_BASE_COLOR)
        roughness = MAT.create_material_expression(material, unreal.MaterialExpressionConstant, -300, 180)
        roughness.set_editor_property("r", 1.0)
        MAT.connect_material_property(roughness, "", unreal.MaterialProperty.MP_ROUGHNESS)
        MAT.recompile_material(material)
        ASSETS.save_loaded_asset(material)
    return material


def spawn_mesh(row, label, location, scale, material=None, animation=None):
    mesh = ASSETS.load_asset(row["path"])
    skeletal = isinstance(mesh, unreal.SkeletalMesh)
    actor = ACTORS.spawn_actor_from_class(unreal.SkeletalMeshActor if skeletal else unreal.StaticMeshActor, unreal.Vector(*location))
    actor.set_actor_label(label)
    actor.set_actor_scale3d(unreal.Vector(scale, scale, scale))
    comp = actor.get_component_by_class(unreal.SkeletalMeshComponent if skeletal else unreal.StaticMeshComponent)
    if skeletal:
        comp.set_skinned_asset_and_update(mesh)
        if animation:
            comp.set_animation(animation)
            comp.set_position(0.0, False)
            comp.set_update_animation_in_editor(True)
        COMPONENTS.append(comp)
    else:
        comp.set_static_mesh(mesh)
    if material:
        for index in range(comp.get_num_materials()):
            comp.set_material(index, material)
    origin, extent = actor.get_actor_bounds(False)
    REPORT["actors"].append({"label": label, "mesh": row["path"], "scale": scale,
        "world_origin": origin.to_tuple(), "world_extent": extent.to_tuple(),
        "materials": [comp.get_material(i).get_path_name() if comp.get_material(i) else None for i in range(comp.get_num_materials())]})
    return actor


try:
    unreal.EditorLoadingAndSavingUtils.new_blank_map(False)
    meshes = DATA["meshes"]
    knight = next(r for r in meshes if "/KnightCurrentImporter/" in r["path"])
    wolf = next(r for r in meshes if "/Review/Wolf/" in r["path"])
    sword = next(r for r in meshes if "/Review/Sword/" in r["path"])
    spawn_mesh(knight, "Knight_180cm_preview", (-230, 0, 0), 0.5, palette("Knight"))
    spawn_mesh(wolf, "Wolf_preview", (-200, 270, 0), 0.5, palette("Wolf"))
    spawn_mesh(sword, "Sword_100cm_preview", (-340, 0, 85), 1.0 / 3.0, palette("Sword"))
    dragon = [r for r in meshes if "/GreenDragon/" in r["path"]]
    for row in dragon:
        spawn_mesh(row, "Dragon_" + row["path"].rsplit("/", 1)[-1], (120, 130, 2), 0.02)
    for label, position in [("Dirt", (-160, -220, 0)), ("Stone", (-150, -220, 12))]:
        row = next(r for r in meshes if "/" + label + "/" in r["path"])
        spawn_mesh(row, label + "_source_material", position, 0.5 if label == "Dirt" else 1.0)
    REPORT["dragon_parts"] = len(dragon)
    REPORT["dragon_shared_skeletons"] = sorted(set(r["skeleton"] for r in dragon))
    floor = ACTORS.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(0, 100, -6))
    floor.static_mesh_component.set_static_mesh(ASSETS.load_asset("/Engine/BasicShapes/Cube.Cube"))
    floor.set_actor_scale3d(unreal.Vector(15, 15, 0.1))
    for rotation, intensity in [((-45,-40,0), 5.0), ((-25,140,0), 2.0), ((-70,40,0), 1.0)]:
        light = ACTORS.spawn_actor_from_class(unreal.DirectionalLight, unreal.Vector(0,0,500), unreal.Rotator(*rotation))
        light_comp = light.get_component_by_class(unreal.DirectionalLightComponent)
        light_comp.set_mobility(unreal.ComponentMobility.MOVABLE)
        light_comp.set_intensity(intensity)
    sky = ACTORS.spawn_actor_from_class(unreal.SkyLight, unreal.Vector(0,0,500))
    sky_comp = sky.get_component_by_class(unreal.SkyLightComponent)
    sky_comp.set_mobility(unreal.ComponentMobility.MOVABLE)
    sky_comp.set_editor_property("source_type", unreal.SkyLightSourceType.SLS_SPECIFIED_CUBEMAP)
    sky_comp.set_cubemap(ASSETS.load_asset("/Engine/MapTemplates/Sky/DaylightAmbientCubemap.DaylightAmbientCubemap"))
    sky_comp.set_intensity(1.0)
    capture_actor = ACTORS.spawn_actor_from_class(unreal.SceneCapture2D, unreal.Vector(750,-1150,650))
    capture_actor.set_actor_rotation(unreal.MathLibrary.find_look_at_rotation(capture_actor.get_actor_location(), unreal.Vector(-30,120,100)), False)
    CAPTURE = capture_actor.get_component_by_class(unreal.SceneCaptureComponent2D)
    TARGET = unreal.RenderingLibrary.create_render_target2d(capture_actor, 1440, 1080, unreal.TextureRenderTargetFormat.RTF_RGBA8)
    CAPTURE.set_editor_property("texture_target", TARGET)
    CAPTURE.set_editor_property("capture_source", unreal.SceneCaptureSource.SCS_FINAL_COLOR_LDR)
    CAPTURE.set_editor_property("fov_angle", 50.0)
    CAPTURE.set_editor_property("capture_every_frame", True)
    START = time.monotonic()
    STAGE = 0
    REPORT["status"] = "WAITING_FOR_RENDER"
    write()

    def tick(delta):
        global STAGE
        try:
            elapsed = time.monotonic() - START
            if STAGE == 0 and elapsed > 12:
                unreal.RenderingLibrary.export_render_target(capture_actor, TARGET, OUT.as_posix(), "assembled_front.png")
                REPORT["captures"].append("assembled_front.png")
                capture_actor.set_actor_location(unreal.Vector(-800,1100,650), False, False)
                capture_actor.set_actor_rotation(unreal.MathLibrary.find_look_at_rotation(capture_actor.get_actor_location(), unreal.Vector(-30,120,100)), False)
                STAGE = 1
                write()
            elif STAGE == 1 and elapsed > 18:
                unreal.RenderingLibrary.export_render_target(capture_actor, TARGET, OUT.as_posix(), "assembled_rear.png")
                REPORT["captures"].append("assembled_rear.png")
                REPORT["status"] = "CAPTURED_REQUIRES_VISUAL_REVIEW"
                write()
                unreal.unregister_slate_post_tick_callback(HANDLE)
                unreal.SystemLibrary.quit_editor()
        except Exception:
            REPORT["status"] = "ERROR"
            REPORT["error"] = traceback.format_exc()
            write()
            unreal.unregister_slate_post_tick_callback(HANDLE)
            unreal.SystemLibrary.quit_editor()
    HANDLE = unreal.register_slate_post_tick_callback(tick)
except Exception:
    REPORT["status"] = "ERROR"
    REPORT["error"] = traceback.format_exc()
    write()
    unreal.SystemLibrary.quit_editor()
