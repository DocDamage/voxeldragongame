"""Render the scoped WP-12 NPC intake in an unsaved developer fixture.

This script applies each supplied one-pixel palette through a simple generated
development material and renders idle/walk poses from the verified intake
assets. It uses a blank, unsaved map and a diagnostic floor only for visual
inspection; it does not create a production Region 01 map or NPC gameplay.

Run via startup ``ExecCmds`` in a full editor session so Slate callbacks can
finish SceneCapture2D exports.
"""

import hashlib
import json
from pathlib import Path
import time
import traceback

import unreal


ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
SOURCE_INTAKE = ROOT / "Saved" / "Diagnostics" / "WP12_region01_npc_intake.json"
NORMALIZED_INTAKE = ROOT / "Saved" / "Diagnostics" / "WP12_region01_normalized_npc_intake.json"
USE_NORMALIZED = bool(globals().get("WP12_USE_NORMALIZED", False))
INTAKE = NORMALIZED_INTAKE if USE_NORMALIZED else SOURCE_INTAKE
OUT = ROOT / "Saved" / "Diagnostics" / (
    "WP12Region01NormalizedNpcPreview" if USE_NORMALIZED else "WP12Region01NpcPreview"
)
OUT.mkdir(parents=True, exist_ok=True)
MATERIAL_ROOT = "/Game/WYRMFALL/Development/Intake/WP12/NPC/Materials"
PALETTE_TEXTURE_ROOT = "/Game/WYRMFALL/Development/Intake/WP12/NPC/PaletteTextures"

ACTORS = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
ASSETS = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
TOOLS = unreal.AssetToolsHelpers.get_asset_tools()
MATERIALS = unreal.MaterialEditingLibrary
COMPONENTS = []
REPORT = {
    "kind": "native_rendered_npc_intake_fixture_not_production_placement",
    "status": "STARTING",
    "variant": "normalized_supplied_fbx" if USE_NORMALIZED else "direct_supplied_fbx",
    "intake_receipt": str(INTAKE),
    "actors": [],
    "captures": [],
}


def write_report():
    (OUT / "report.json").write_text(json.dumps(REPORT, indent=2), encoding="utf-8")


def sha256(path):
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for chunk in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def palette_texture_for(role, source_path):
    palette_path = PALETTE_TEXTURE_ROOT + "/T_PaletteFactory_" + role
    existing = ASSETS.load_asset(palette_path) if ASSETS.does_asset_exist(palette_path) else None
    if existing is not None:
        if not isinstance(existing, unreal.Texture2D):
            raise RuntimeError("Existing fixture palette is not a Texture2D: " + palette_path)
        return existing

    source = Path(source_path)
    if not source.is_file():
        raise RuntimeError("Missing supplied palette source for {}: {}".format(role, source))
    factory = unreal.TextureFactory()
    # The Blender source inspection identifies a 256x1 palette with all mesh
    # UVs on its only row. Import a fresh ignored fixture texture with the
    # correct settings before its first GPU build; do not mutate the normal
    # intake texture after it has already been compressed.
    factory.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_EDITOR_ICON)
    factory.set_editor_property("mip_gen_settings", unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS)
    task = unreal.AssetImportTask()
    task.filename = source.as_posix()
    task.destination_path = PALETTE_TEXTURE_ROOT
    task.destination_name = "T_PaletteFactory_" + role
    task.automated = True
    task.save = True
    task.replace_existing = False
    task.async_ = False
    task.factory = factory
    TOOLS.import_asset_tasks([task])
    texture = ASSETS.load_asset(palette_path)
    if texture is None or not isinstance(texture, unreal.Texture2D):
        raise RuntimeError("Fixture palette import did not produce a Texture2D for " + role)
    return texture


def material_for(role, source_texture_path):
    material_path = MATERIAL_ROOT + "/M_PaletteDiagnosticEmissive_" + role
    texture = palette_texture_for(role, source_texture_path)
    material = ASSETS.load_asset(material_path) if ASSETS.does_asset_exist(material_path) else None
    if material is not None:
        return material

    material = TOOLS.create_asset("M_PaletteDiagnosticEmissive_" + role, MATERIAL_ROOT, unreal.Material, unreal.MaterialFactoryNew())
    if material is None:
        raise RuntimeError("Could not create development material for " + role)
    sample = MATERIALS.create_material_expression(material, unreal.MaterialExpressionTextureSample, -300, 0)
    sample.set_editor_property("texture", texture)
    MATERIALS.connect_material_property(sample, "RGB", unreal.MaterialProperty.MP_BASE_COLOR)
    # Keep the actual palette colours legible in the developer fixture even
    # when its diagnostic lights leave a voxel face in shadow. This is not a
    # production lighting/material choice.
    MATERIALS.connect_material_property(sample, "RGB", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    roughness = MATERIALS.create_material_expression(material, unreal.MaterialExpressionConstant, -300, 180)
    roughness.set_editor_property("r", 1.0)
    MATERIALS.connect_material_property(roughness, "", unreal.MaterialProperty.MP_ROUGHNESS)
    MATERIALS.recompile_material(material)
    ASSETS.save_loaded_asset(material)
    return material


def set_animation(component, animation):
    component.set_animation_mode(unreal.AnimationMode.ANIMATION_SINGLE_NODE)
    component.set_animation(animation)
    component.set_position(0.0, False)
    component.set_update_animation_in_editor(True)
    component.play(True)


def spawn_npc(row, source_row, location):
    role = row["role"]
    mesh = ASSETS.load_asset(row["mesh"]["object_path"])
    idle = ASSETS.load_asset(row["animations"]["idle"]["object_path"])
    walk = ASSETS.load_asset(row["animations"]["walk"]["object_path"])
    if not isinstance(mesh, unreal.SkeletalMesh):
        raise RuntimeError("Missing SkeletalMesh for " + role)
    if not isinstance(idle, unreal.AnimSequence) or not isinstance(walk, unreal.AnimSequence):
        raise RuntimeError("Missing AnimSequence intake for " + role)

    source_texture_path = source_row["source_files"]["texture"]["path"]
    material = material_for(role, source_texture_path)
    actor = ACTORS.spawn_actor_from_class(unreal.SkeletalMeshActor, unreal.Vector(*location))
    actor.set_actor_label("WP12_" + role + "_DeveloperFixture")
    actor.set_actor_scale3d(unreal.Vector(0.5, 0.5, 0.5))
    component = actor.get_component_by_class(unreal.SkeletalMeshComponent)
    component.set_skinned_asset_and_update(mesh)
    for material_index in range(component.get_num_materials()):
        component.set_material(material_index, material)
    set_animation(component, idle)
    origin, extent = actor.get_actor_bounds(False)
    COMPONENTS.append({"role": role, "component": component, "idle": idle, "walk": walk})
    REPORT["actors"].append({
        "role": role,
        "mesh": mesh.get_path_name(),
        "texture": source_row["texture"]["object_path"],
        "fixture_palette_texture": palette_texture_for(role, source_texture_path).get_path_name(),
        "development_material": material.get_path_name(),
        "idle": idle.get_path_name(),
        "walk": walk.get_path_name(),
        "world_bounds_origin": origin.to_tuple(),
        "world_bounds_extent": extent.to_tuple(),
        "component_materials": [
            component.get_material(index).get_path_name() if component.get_material(index) else None
            for index in range(component.get_num_materials())
        ],
    })


def capture(capture_component, target, filename):
    # Explicit captures stay reliable after a mesh-animation switch in an
    # automated editor session; do not rely on an idle every-frame target.
    capture_component.capture_scene()
    unreal.RenderingLibrary.export_render_target(capture_component, target, OUT.as_posix(), filename)
    image_path = OUT / filename
    if not image_path.is_file() or image_path.stat().st_size == 0:
        raise RuntimeError("Scene capture did not produce " + image_path.as_posix())
    REPORT["captures"].append({"file": filename, "sha256": sha256(image_path)})


try:
    intake = json.loads(INTAKE.read_text(encoding="utf-8"))
    source_intake = json.loads(SOURCE_INTAKE.read_text(encoding="utf-8"))
    source_rows = {row["role"]: row for row in source_intake.get("roles", [])}
    expected_status = (
        "PASS_NORMALIZED_NPC_MESH_AND_ANIMATION_SKELETON_COMPATIBILITY"
        if USE_NORMALIZED
        else "PASS_SCOPED_NPC_MESH_TEXTURE_ANIMATION_INTAKE_AND_MEASUREMENT"
    )
    if intake.get("status") != expected_status:
        raise RuntimeError("NPC intake receipt is not ready for visual fixture: " + str(intake.get("status")))
    if len(intake.get("roles", [])) != 6:
        raise RuntimeError("Expected six selected roles in the NPC intake receipt")
    if len(source_rows) != 6:
        raise RuntimeError("Expected six supplied source-role rows for palette lookup")

    unreal.EditorLoadingAndSavingUtils.new_blank_map(False)
    locations = ((-360, -180, 0), (0, -180, 0), (360, -180, 0), (-360, 190, 0), (0, 190, 0), (360, 190, 0))
    for row, location in zip(intake["roles"], locations):
        source_row = source_rows.get(row["role"])
        if source_row is None:
            raise RuntimeError("No supplied palette source mapped for " + row["role"])
        spawn_npc(row, source_row, location)

    floor = ACTORS.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(0, 0, -6))
    floor.static_mesh_component.set_static_mesh(ASSETS.load_asset("/Engine/BasicShapes/Cube.Cube"))
    floor.set_actor_scale3d(unreal.Vector(12, 10, 0.1))
    for rotation, intensity in (((-45, -40, 0), 5.0), ((-25, 140, 0), 2.0), ((-70, 40, 0), 1.0)):
        light = ACTORS.spawn_actor_from_class(unreal.DirectionalLight, unreal.Vector(0, 0, 500), unreal.Rotator(*rotation))
        light_component = light.get_component_by_class(unreal.DirectionalLightComponent)
        light_component.set_mobility(unreal.ComponentMobility.MOVABLE)
        light_component.set_intensity(intensity)
    sky = ACTORS.spawn_actor_from_class(unreal.SkyLight, unreal.Vector(0, 0, 500))
    sky_component = sky.get_component_by_class(unreal.SkyLightComponent)
    sky_component.set_mobility(unreal.ComponentMobility.MOVABLE)
    sky_component.set_intensity(1.0)

    capture_actor = ACTORS.spawn_actor_from_class(unreal.SceneCapture2D, unreal.Vector(1180, -1420, 680))
    capture_actor.set_actor_rotation(
        unreal.MathLibrary.find_look_at_rotation(capture_actor.get_actor_location(), unreal.Vector(0, 0, 150)), False
    )
    capture_component = capture_actor.get_component_by_class(unreal.SceneCaptureComponent2D)
    target = unreal.RenderingLibrary.create_render_target2d(
        capture_actor, 1440, 1080, unreal.TextureRenderTargetFormat.RTF_RGBA8
    )
    capture_component.set_editor_property("texture_target", target)
    capture_component.set_editor_property("capture_source", unreal.SceneCaptureSource.SCS_FINAL_COLOR_LDR)
    capture_component.set_editor_property("fov_angle", 48.0)
    capture_component.set_editor_property("capture_every_frame", False)
    capture_component.set_editor_property("capture_on_movement", False)

    start = time.monotonic()
    stage = 0
    REPORT["status"] = "WAITING_FOR_RENDER"
    write_report()

    def tick(delta):
        global stage
        try:
            elapsed = time.monotonic() - start
            if stage == 0 and elapsed > 8:
                capture(capture_component, target, "idle_front.png")
                for entry in COMPONENTS:
                    set_animation(entry["component"], entry["walk"])
                stage = 1
                write_report()
            elif stage == 1 and elapsed > 11:
                # Sample a known non-zero walk frame explicitly. Editor
                # automation does not guarantee that a play-state advances
                # between Slate ticks, while this verifies the imported pose.
                for entry in COMPONENTS:
                    entry["component"].set_position(0.5, False)
                capture(capture_component, target, "walk_front.png")
                REPORT["status"] = "CAPTURED_REQUIRES_VISUAL_REVIEW"
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
