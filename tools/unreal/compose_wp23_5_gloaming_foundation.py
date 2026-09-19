"""Compose the bounded L_GloamingMarches environment/navigation foundation."""

import json
from pathlib import Path

import unreal


ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
MAP = "/Game/WYRMFALL/World/Regions/L_GloamingMarches"
REPORT = ROOT / "Saved/Diagnostics/WP23_5_gloaming_foundation_composition.json"
INTAKE = ROOT / "Saved/Diagnostics/WP23_5_gloaming_environment_intake.json"
GROUND = "/Game/WYRMFALL/World/Regions/Materials/M_GloamingGround"
SURFACE_Z = 900.0


def ensure_ground_material(assets):
    material = assets.load_asset(GROUND) if assets.does_asset_exist(GROUND) else None
    if material:
        return material
    material = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        "M_GloamingGround", "/Game/WYRMFALL/World/Regions/Materials",
        unreal.Material, unreal.MaterialFactoryNew())
    if not material:
        raise RuntimeError("Could not create M_GloamingGround")
    editing = unreal.MaterialEditingLibrary
    color = editing.create_material_expression(material, unreal.MaterialExpressionVectorParameter, -300, 0)
    color.set_editor_property("parameter_name", "GroundColor")
    color.set_editor_property("default_value", unreal.LinearColor(0.055, 0.075, 0.065, 1.0))
    editing.connect_material_property(color, "RGB", unreal.MaterialProperty.MP_BASE_COLOR)
    roughness = editing.create_material_expression(material, unreal.MaterialExpressionConstant, -300, 180)
    roughness.set_editor_property("r", 0.96)
    editing.connect_material_property(roughness, "", unreal.MaterialProperty.MP_ROUGHNESS)
    editing.recompile_material(material)
    if not assets.save_loaded_asset(material):
        raise RuntimeError("Could not save M_GloamingGround")
    return material


def add_mesh(actors, assets, label, mesh_path, material_path, location, rotation, target_height):
    mesh = assets.load_asset(mesh_path)
    material = assets.load_asset(material_path)
    if not isinstance(mesh, unreal.StaticMesh) or not isinstance(material, unreal.MaterialInterface):
        raise RuntimeError(f"Missing supplied mesh/material for {label}: {mesh_path}, {material_path}")
    cemetery_y_up = "/Cemetery/" in mesh_path
    authored_yaw = rotation[2]
    actor_rotation = unreal.Rotator(90.0, 0.0, authored_yaw) if cemetery_y_up else unreal.Rotator(*rotation)
    actor = actors.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(*location), actor_rotation)
    actor.set_actor_label("GLM_ENV_" + label)
    component = actor.static_mesh_component
    component.set_static_mesh(mesh)
    component.set_collision_profile_name("BlockAll")
    for index in range(max(component.get_num_materials(), 1)):
        component.set_material(index, material)
    box = mesh.get_bounding_box()
    initial_height = (box.max.y - box.min.y) if cemetery_y_up else (box.max.z - box.min.z)
    if initial_height <= 0.01:
        raise RuntimeError(
            f"Unusable supplied mesh height for {label}: min={box.min.to_tuple()} max={box.max.to_tuple()}")
    scale = target_height / initial_height
    actor.set_actor_scale3d(unreal.Vector(scale, scale, scale))
    source_bottom = box.min.y if cemetery_y_up else box.min.z
    actor.set_actor_location(
        unreal.Vector(location[0], location[1], SURFACE_Z - source_bottom * scale),
        False, False)
    origin, extent = actor.get_actor_bounds(False)
    current = actor.get_actor_location()
    actor.set_actor_location(
        unreal.Vector(
            current.x + location[0] - origin.x,
            current.y + location[1] - origin.y,
            current.z + SURFACE_Z - (origin.z - extent.z)),
        False, False)
    origin, extent = actor.get_actor_bounds(False)
    source_dimensions = [box.max.x - box.min.x, box.max.y - box.min.y, box.max.z - box.min.z]
    world_dimensions = ([source_dimensions[0], source_dimensions[2], source_dimensions[1]]
                        if cemetery_y_up else source_dimensions)
    return {
        "label": actor.get_actor_label(), "mesh": mesh.get_path_name(),
        "material": material.get_path_name(), "target_height_cm": target_height,
        "uniform_scale": scale, "source_up_axis": "Y" if cemetery_y_up else "Z",
        "calculated_dimensions_cm": [value * scale for value in world_dimensions],
        "observed_bounds_extent_cm": list(extent.to_tuple()),
        "observed_bounds_center_cm": list(origin.to_tuple()),
        "observed_bounds_bottom_z": origin.z - extent.z,
    }


def main():
    if not INTAKE.is_file():
        raise RuntimeError("Run import_wp23_5_gloaming_environment.py first")
    intake = json.loads(INTAKE.read_text(encoding="utf-8"))
    if intake.get("status") != "PASS":
        raise RuntimeError("Gloaming environment intake is not a PASS")
    catalog = {row["name"]: row for group in ("cemetery", "cathedral") for row in intake[group]}

    levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    assets = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
    opened = levels.load_level(MAP) if assets.does_asset_exist(MAP) else levels.new_level(MAP)
    if not opened:
        raise RuntimeError("Could not create or load " + MAP)
    world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
    for actor in list(actors.get_all_level_actors()):
        actors.destroy_actor(actor)

    game_mode = unreal.load_class(None, "/Script/WYRMFALL.WyrmGameMode")
    if game_mode:
        world.get_world_settings().set_editor_property("default_game_mode", game_mode)

    sun = actors.spawn_actor_from_class(unreal.DirectionalLight, unreal.Vector(0, 0, 3000), unreal.Rotator(-24, -52, 0))
    sun.set_actor_label("GLM_DirectionalLight")
    sun_component = sun.get_component_by_class(unreal.DirectionalLightComponent)
    sun_component.set_mobility(unreal.ComponentMobility.MOVABLE)
    sun_component.set_editor_property("intensity", 0.42)
    sun_component.set_editor_property("light_color", unreal.Color(166, 187, 214, 255))
    sky = actors.spawn_actor_from_class(unreal.SkyLight, unreal.Vector(0, 0, 2200))
    sky.set_actor_label("GLM_SkyLight")
    sky_component = sky.get_component_by_class(unreal.SkyLightComponent)
    sky_component.set_mobility(unreal.ComponentMobility.MOVABLE)
    sky_component.set_editor_property("intensity", 0.22)
    sky_component.set_editor_property("real_time_capture", True)
    actors.spawn_actor_from_class(unreal.SkyAtmosphere, unreal.Vector(0, 0, 1000)).set_actor_label("GLM_SkyAtmosphere")
    fog = actors.spawn_actor_from_class(unreal.ExponentialHeightFog, unreal.Vector(0, 0, 700))
    fog.set_actor_label("GLM_HeightFog")
    fog_component = fog.get_component_by_class(unreal.ExponentialHeightFogComponent)
    fog_component.set_editor_property("fog_density", 0.025)
    fog_component.set_editor_property("fog_height_falloff", 0.28)

    terrain = actors.spawn_actor_from_class(unreal.GeoForgeInfiniteTerrainActor, unreal.Vector())
    terrain.set_actor_label("GLM_GeoForgeTerrain")
    settings = {
        "auto_rebuild_in_editor": False, "auto_apply_preset_defaults": False,
        "follow_editor_viewport_camera_for_streaming": False,
        "world_shape": unreal.GeoForgeWorldShape.PLANAR_FINITE,
        "horizontal_bounds_mode": unreal.GeoForgeHorizontalBoundsMode.FINITE_CENTERED,
        "finite_bounds_half_extent_x_in_chunks": 4, "finite_bounds_half_extent_y_in_chunks": 4,
        "chunk_size_in_cells": 16, "chunk_height_in_cells": 16, "cell_size": 100.0,
        "ground_level_in_cells": 8, "max_mountain_height_in_cells": 0,
        "water_level_in_cells": -100, "river_depth_in_cells": 0, "generate_caves": False,
        "enable_built_in_cube_trees": False, "generate_collision": True,
        "persist_edited_cells_across_streaming": True, "use_async_chunk_generation": False,
        "max_queued_chunk_rebuilds_per_tick": 64,
        "chunk_render_mode": unreal.GeoForgeChunkRenderMode.MARCHING_CUBES_SURFACE,
        "world_seed": 2355, "save_compatibility_id": "WYRMFALL.GloamingMarches",
        "streamed_terrain_affects_navigation": True,
    }
    for key, value in settings.items():
        terrain.set_editor_property(key, value)
    ground = ensure_ground_material(assets)
    for key in ("terrain_material", "surface_material", "soil_material", "rock_material",
                "deep_rock_material", "sand_material", "snow_material"):
        terrain.set_editor_property(key, ground)
    terrain.set_editor_property("use_general_biome_terrain_material", True)
    terrain.set_editor_property("general_biome_terrain_material", ground)

    adapter = actors.spawn_actor_from_class(unreal.WyrmGeoForgeAdapter, unreal.Vector())
    adapter.set_actor_label("GLM_GeoForgeAdapter")
    adapter.bind_terrain_actor(terrain)
    nav = actors.spawn_actor_from_class(unreal.NavMeshBoundsVolume, unreal.Vector(0, 0, SURFACE_Z))
    nav.set_actor_label("GLM_NavMeshBounds")
    nav.set_actor_scale3d(unreal.Vector(100, 100, 30))
    _nav_origin, nav_extent = nav.get_actor_bounds(False)
    nav_system = unreal.NavigationSystemV1.get_navigation_system(world)
    if not nav_system or min(nav_extent.x, nav_extent.y, nav_extent.z) <= 0:
        raise RuntimeError("Gloaming navigation bounds are unavailable")
    nav_system.on_navigation_bounds_updated(nav)

    route = [
        ("GLM_ROUTE_ARRIVAL", (-5000, -3400, SURFACE_Z)),
        ("GLM_ROUTE_ASHGRAVE", (-2600, -700, SURFACE_Z)),
        ("GLM_ROUTE_MALVAINE", (650, -450, SURFACE_Z)),
        ("GLM_ROUTE_TWINS", (3900, 1700, SURFACE_Z)),
    ]
    for label, location in route:
        anchor = actors.spawn_actor_from_class(unreal.TargetPoint, unreal.Vector(*location))
        anchor.set_actor_label(label)
    actors.spawn_actor_from_class(unreal.PlayerStart, unreal.Vector(-5000, -3400, SURFACE_Z + 96)).set_actor_label("PlayerStart_GloamingArrival")
    arrival_trigger = actors.spawn_actor_from_class(
        unreal.WyrmGloamingArrivalTrigger, unreal.Vector(-5000, -3400, SURFACE_Z + 100))
    arrival_trigger.set_actor_label("GLM_TRIGGER_Arrival")

    placements = [
        ("ArrivalGate", "Gate", (-3800, -2450, SURFACE_Z), (0, 0, 125), 320.0),
        ("AshgraveCrypt", "Crypt", (-2600, 250, SURFACE_Z), (0, 0, -18), 270.0),
        ("AshgraveTombA", "Tombstone", (-3150, -150, SURFACE_Z), (0, 0, 12), 105.0),
        ("AshgraveTombB", "CrossGrave", (-2150, 350, SURFACE_Z), (0, 0, -22), 130.0),
        ("AshgraveOpenGrave", "DirtGrave", (-1800, -50, SURFACE_Z), (0, 0, 15), 35.0),
        ("AshgraveCoffin", "Coffin", (-1450, 500, SURFACE_Z), (0, 0, 30), 75.0),
        ("FenceWest", "Fence", (-3350, 350, SURFACE_Z), (0, 0, 5), 150.0),
        ("FenceEast", "Fence", (-950, 650, SURFACE_Z), (0, 0, -12), 150.0),
        ("MalvaineCathedral", "Cathedral", (500, 2150, SURFACE_Z), (0, 0, 180), 1450.0),
        ("MalvaineStatue", "Statue", (1550, 650, SURFACE_Z), (0, 0, -145), 310.0),
        ("MalvaineGargoyle", "Gargoyle", (250, 750, SURFACE_Z), (0, 0, -25), 185.0),
        ("TwinsTreeA", "Tree", (3300, 2700, SURFACE_Z), (0, 0, 0), 520.0),
        ("TwinsTreeB", "Tree", (4500, 2750, SURFACE_Z), (0, 0, 0), 470.0),
        ("TwinsGraveA", "Grave", (3400, 2350, SURFACE_Z), (0, 0, 20), 125.0),
        ("TwinsGraveB", "CrossGrave", (4400, 2300, SURFACE_Z), (0, 0, -20), 135.0),
    ]
    placed = []
    for label, asset_name, location, rotation, height in placements:
        row = catalog[asset_name]
        placed.append(add_mesh(actors, assets, label, row["mesh"], row["material"], location, rotation, height))

    ashgrave_seal = actors.spawn_actor_from_class(
        unreal.WyrmGloamingAshgraveSeal, unreal.Vector(-2600, -500, SURFACE_Z + 120))
    ashgrave_seal.set_actor_label("GLM_INTERACT_AshgraveExtractionSeal")
    malvaine = actors.spawn_actor_from_class(
        unreal.WyrmCountMalvaineCharacter, unreal.Vector(650, -250, SURFACE_Z + 90), unreal.Rotator(0, 0, 180))
    malvaine.set_actor_label("GLM_ENCOUNTER_CountMalvaine")

    # Cool pools preserve the horror palette while making the critical route silhouettes readable.
    for label, location, color, intensity, radius in (
        ("ArrivalFill", (-3900, -2500, 1450), unreal.Color(90, 125, 175, 255), 180.0, 2200.0),
        ("CathedralFill", (600, 700, 1700), unreal.Color(75, 105, 155, 255), 240.0, 2800.0),
        ("TwinsFill", (3900, 2050, 1500), unreal.Color(90, 125, 170, 255), 160.0, 2100.0),
    ):
        light = actors.spawn_actor_from_class(unreal.PointLight, unreal.Vector(*location))
        light.set_actor_label("GLM_" + label)
        component = light.get_component_by_class(unreal.PointLightComponent)
        component.set_mobility(unreal.ComponentMobility.MOVABLE)
        component.set_editor_property("intensity", intensity)
        component.set_editor_property("attenuation_radius", radius)
        component.set_editor_property("light_color", color)

    if not levels.save_current_level():
        raise RuntimeError("Failed to save L_GloamingMarches")
    REPORT.parent.mkdir(parents=True, exist_ok=True)
    REPORT.write_text(json.dumps({
        "status": "PASS", "map": MAP, "engine": unreal.SystemLibrary.get_engine_version(),
        "scope": "environment_navigation_foundation_only",
        "terrain_material": ground.get_path_name(), "surface_z": SURFACE_Z,
        "route_anchors": [label for label, _ in route], "supplied_placements": placed,
        "gameplay_actors": [
            arrival_trigger.get_actor_label(), ashgrave_seal.get_actor_label(), malvaine.get_actor_label()],
        "nav_bounds_extent_cm": list(nav_extent.to_tuple()),
        "not_claimed": ["encounters", "region completion", "interactive walkthrough"],
    }, indent=2) + "\n", encoding="utf-8")
    print("WP-23.5 Gloaming foundation composition: PASS")


main()
