"""Compose the bounded L_CogspireHarbor environment/navigation foundation."""

import json
from pathlib import Path

import unreal


ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
MAP = "/Game/WYRMFALL/World/Regions/L_CogspireHarbor"
DEST = "/Game/WYRMFALL/Development/Intake/WP23_6"
REPORT = ROOT / "Saved/Diagnostics/WP23_6_cogspire_map_composition.json"
WATER_MATERIAL = "/Game/WYRMFALL/World/Regions/Materials/M_CogspireHarborWater"
WATER_Z = 80.0
WALK_Z = 205.0


def load(assets, path, expected):
    asset = assets.load_asset(path)
    if asset is None or not isinstance(asset, expected):
        raise RuntimeError("Missing or wrong-class asset: " + path)
    return asset


def water_material(assets):
    existing = assets.load_asset(WATER_MATERIAL) if assets.does_asset_exist(WATER_MATERIAL) else None
    if existing:
        return existing
    tools = unreal.AssetToolsHelpers.get_asset_tools()
    material = tools.create_asset(
        "M_CogspireHarborWater", "/Game/WYRMFALL/World/Regions/Materials",
        unreal.Material, unreal.MaterialFactoryNew())
    if material is None:
        raise RuntimeError("Could not create Cogspire Harbor water material")
    editing = unreal.MaterialEditingLibrary
    color = editing.create_material_expression(material, unreal.MaterialExpressionConstant3Vector, -240, 0)
    color.set_editor_property("constant", unreal.LinearColor(0.015, 0.12, 0.19, 1.0))
    editing.connect_material_property(color, "", unreal.MaterialProperty.MP_BASE_COLOR)
    roughness = editing.create_material_expression(material, unreal.MaterialExpressionConstant, -240, 160)
    roughness.set_editor_property("r", 0.28)
    editing.connect_material_property(roughness, "", unreal.MaterialProperty.MP_ROUGHNESS)
    metallic = editing.create_material_expression(material, unreal.MaterialExpressionConstant, -240, 300)
    metallic.set_editor_property("r", 0.12)
    editing.connect_material_property(metallic, "", unreal.MaterialProperty.MP_METALLIC)
    editing.recompile_material(material)
    if not assets.save_loaded_asset(material):
        raise RuntimeError("Could not save Cogspire Harbor water material")
    return material


def place_bottom(actor, location, scale=1.0, rotation=None):
    actor.set_actor_scale3d(unreal.Vector(scale, scale, scale))
    if rotation is not None:
        actor.set_actor_rotation(unreal.Rotator(*rotation), False)
    origin, extent = actor.get_actor_bounds(False)
    actor.set_actor_location(
        unreal.Vector(location[0] - origin.x, location[1] - origin.y,
                      location[2] - (origin.z - extent.z)), False, False)


def spawn_asset(actors, assets, label, path, location, scale=1.0, rotation=None):
    asset = assets.load_asset(path)
    if isinstance(asset, unreal.StaticMesh):
        actor = actors.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector())
        component = actor.get_component_by_class(unreal.StaticMeshComponent)
        component.set_static_mesh(asset)
    elif isinstance(asset, unreal.SkeletalMesh):
        actor = actors.spawn_actor_from_class(unreal.SkeletalMeshActor, unreal.Vector())
        component = actor.get_component_by_class(unreal.SkeletalMeshComponent)
        component.set_skinned_asset_and_update(asset)
    else:
        raise RuntimeError("Missing or unsupported supplied asset: " + path)
    actor.set_actor_label("COG_ENV_" + label)
    component.set_collision_profile_name("BlockAll")
    component.set_collision_enabled(unreal.CollisionEnabled.QUERY_AND_PHYSICS)
    component.set_editor_property("can_ever_affect_navigation", True)
    place_bottom(actor, location, scale, rotation)
    return actor


def spawn_hidden_substrate(actors, assets):
    cube = load(assets, "/Engine/BasicShapes/Cube.Cube", unreal.StaticMesh)
    actor = actors.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(-50, 0, 190))
    actor.set_actor_label("COG_NAV_HiddenDockSubstrate")
    actor.set_actor_scale3d(unreal.Vector(35.5, 8.0, 0.1))
    component = actor.get_component_by_class(unreal.StaticMeshComponent)
    component.set_static_mesh(cube)
    component.set_material(0, load(
        assets, DEST + "/Harbor/Jetty/Material_0.Material_0", unreal.MaterialInterface))
    component.set_collision_profile_name("BlockAll")
    component.set_collision_enabled(unreal.CollisionEnabled.QUERY_AND_PHYSICS)
    component.set_editor_property("can_ever_affect_navigation", True)
    return actor


def main():
    levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    assets = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
    opened = levels.load_level(MAP) if assets.does_asset_exist(MAP) else levels.new_level(MAP)
    if not opened:
        raise RuntimeError("Could not create or load " + MAP)
    for actor in list(actors.get_all_level_actors()):
        actors.destroy_actor(actor)
    world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
    game_mode = unreal.load_class(None, "/Script/WYRMFALL.WyrmGameMode")
    if game_mode:
        world.get_world_settings().set_editor_property("default_game_mode", game_mode)

    sun = actors.spawn_actor_from_class(
        unreal.DirectionalLight, unreal.Vector(0, 0, 2800), unreal.Rotator(-38, -55, 0))
    sun.set_actor_label("COG_DirectionalLight")
    sun_component = sun.get_component_by_class(unreal.DirectionalLightComponent)
    sun_component.set_mobility(unreal.ComponentMobility.MOVABLE)
    sun_component.set_editor_property("intensity", 4.0)
    sky = actors.spawn_actor_from_class(unreal.SkyLight, unreal.Vector(0, 0, 1800))
    sky.set_actor_label("COG_SkyLight")
    sky_component = sky.get_component_by_class(unreal.SkyLightComponent)
    sky_component.set_mobility(unreal.ComponentMobility.MOVABLE)
    sky_component.set_editor_property("intensity", 1.8)
    sky_component.set_editor_property("real_time_capture", True)
    actors.spawn_actor_from_class(unreal.SkyAtmosphere, unreal.Vector()).set_actor_label("COG_SkyAtmosphere")
    for label, location, color, intensity, radius in (
        ("ArrivalFill", (-1200, -350, 900), unreal.Color(255, 190, 120, 255), 2200.0, 2600.0),
        ("JettyFill", (850, 0, 850), unreal.Color(160, 205, 255, 255), 1800.0, 2600.0),
        ("MachineryFill", (-250, 900, 850), unreal.Color(255, 155, 90, 255), 1800.0, 2200.0),
    ):
        light = actors.spawn_actor_from_class(unreal.PointLight, unreal.Vector(*location))
        light.set_actor_label("COG_" + label)
        component = light.get_component_by_class(unreal.PointLightComponent)
        component.set_mobility(unreal.ComponentMobility.MOVABLE)
        component.set_editor_property("intensity", intensity)
        component.set_editor_property("attenuation_radius", radius)
        component.set_editor_property("light_color", color)

    water_mesh = load(assets, "/Engine/BasicShapes/Plane.Plane", unreal.StaticMesh)
    water = actors.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(1750, 0, WATER_Z))
    water.set_actor_label("COG_VISUAL_HarborWaterSurface")
    water.set_actor_scale3d(unreal.Vector(45, 35, 1))
    water_component = water.get_component_by_class(unreal.StaticMeshComponent)
    water_component.set_static_mesh(water_mesh)
    water_component.set_material(0, water_material(assets))
    water_component.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
    water_component.set_editor_property("can_ever_affect_navigation", False)

    water_owner = actors.spawn_actor_from_class(unreal.WyrmWaterVolume, unreal.Vector(2300, 0, -220))
    water_owner.set_actor_label("COG_WATER_HarborAuthority")
    water_owner.set_actor_scale3d(unreal.Vector(2.0, 3.0, 1.0))
    water_owner.set_editor_property("surface_elevation", WATER_Z)

    terrain = actors.spawn_actor_from_class(
        unreal.GeoForgeInfiniteTerrainActor, unreal.Vector(0, 0, -1500))
    terrain.set_actor_label("COG_GeoForgeTerrain")
    terrain_settings = {
        "auto_rebuild_in_editor": False,
        "auto_apply_preset_defaults": False,
        "follow_editor_viewport_camera_for_streaming": False,
        "world_shape": unreal.GeoForgeWorldShape.PLANAR_FINITE,
        "horizontal_bounds_mode": unreal.GeoForgeHorizontalBoundsMode.FINITE_CENTERED,
        "finite_bounds_half_extent_x_in_chunks": 3,
        "finite_bounds_half_extent_y_in_chunks": 3,
        "chunk_size_in_cells": 16,
        "chunk_height_in_cells": 16,
        "cell_size": 100.0,
        "ground_level_in_cells": 8,
        "max_mountain_height_in_cells": 0,
        "water_level_in_cells": -100,
        "river_depth_in_cells": 0,
        "generate_caves": False,
        "enable_built_in_cube_trees": False,
        "generate_collision": True,
        "persist_edited_cells_across_streaming": True,
        "use_async_chunk_generation": False,
        "max_queued_chunk_rebuilds_per_tick": 64,
        "chunk_render_mode": unreal.GeoForgeChunkRenderMode.MARCHING_CUBES_SURFACE,
        "world_seed": 2366,
        "save_compatibility_id": "WYRMFALL.CogspireHarbor",
        "streamed_terrain_affects_navigation": False,
    }
    for key, value in terrain_settings.items():
        terrain.set_editor_property(key, value)
    ground = load(
        assets, "/Game/WYRMFALL/World/Regions/Materials/M_GloamingGround.M_GloamingGround",
        unreal.MaterialInterface)
    for key in ("terrain_material", "surface_material", "soil_material", "rock_material",
                "deep_rock_material", "sand_material", "snow_material"):
        terrain.set_editor_property(key, ground)
    terrain.set_editor_property("use_general_biome_terrain_material", True)
    terrain.set_editor_property("general_biome_terrain_material", ground)
    adapter = actors.spawn_actor_from_class(unreal.WyrmGeoForgeAdapter, unreal.Vector())
    adapter.set_actor_label("COG_GeoForgeAdapter")
    adapter.bind_terrain_actor(terrain)

    jetty = DEST + "/Harbor/Jetty/jetty.jetty"
    for row, y in enumerate((-675, -450, -225, 0, 225, 450, 675), 1):
        for column, x in enumerate((-1700, -1350, -1000, -650, -300), 1):
            spawn_asset(actors, assets, f"Quay_R{row}_C{column}", jetty, (x, y, 0), 8.0)
    for index, x in enumerate((50, 400, 750, 1100, 1450, 1800), 1):
        spawn_asset(actors, assets, f"Jetty{index}", jetty, (x, 0, 0), 8.0)

    substrate = spawn_hidden_substrate(actors, assets)
    spawn_asset(actors, assets, "HarborBuilding", DEST + "/Harbor/Building_1/Building_1.Building_1", (-1550, 750, 200))
    spawn_asset(actors, assets, "Bridge", DEST + "/Harbor/Bridge_2/Bridge_2.Bridge_2", (-250, 700, 80), 1.8, (0, 0, 90))
    spawn_asset(actors, assets, "Boat", DEST + "/Harbor/Boat/Boat.Boat", (1950, 850, -110), 1.0, (0, 0, -8))
    spawn_asset(actors, assets, "Rowboat", DEST + "/Harbor/Rowboat/Rowboat.Rowboat", (950, -650, -25), 1.0, (0, 0, 12))
    spawn_asset(actors, assets, "Lantern", DEST + "/Harbor/Lantern_1/Lantern_1.Lantern_1", (50, -170, 200), 1.6)
    spawn_asset(actors, assets, "CargoChest", DEST + "/Harbor/Chest/Chest.Chest", (-550, -650, 200))

    pump = DEST + "/Engine/WaterPump/water_pump.water_pump"
    steam = DEST + "/Engine/SteamLeakFrame01/frame_001.frame_001"
    spawn_asset(actors, assets, "CivicWaterPumpA", pump, (-1550, -700, 200), 4.0)
    spawn_asset(actors, assets, "CivicWaterPumpB", pump, (-1050, -700, 200), 4.0)
    spawn_asset(actors, assets, "CoercionEnginePump", pump, (-150, 900, 200), 7.0)
    spawn_asset(actors, assets, "CoercionSteamA", steam, (-360, 900, 200), 4.0)
    spawn_asset(actors, assets, "CoercionSteamB", steam, (60, 900, 200), 4.0, (0, 0, 180))

    for label, location in (
        ("COG_ROUTE_ARRIVAL", (-1400, 0, WALK_Z)),
        ("COG_ROUTE_JETTY_END", (1650, 0, WALK_Z)),
        ("LM-COGSPIRE-ARRIVAL", (-1400, 0, WALK_Z)),
        ("LM-COGSPIRE-RETURN", (-1100, -400, WALK_Z)),
    ):
        anchor = actors.spawn_actor_from_class(unreal.TargetPoint, unreal.Vector(*location))
        anchor.set_actor_label(label)
    actors.spawn_actor_from_class(
        unreal.PlayerStart, unreal.Vector(-1400, 0, WALK_Z + 96)).set_actor_label("PlayerStart_CogspireArrival")

    nav = actors.spawn_actor_from_class(unreal.NavMeshBoundsVolume, unreal.Vector(0, 0, 300))
    nav.set_actor_label("COG_NavMeshBounds")
    nav.set_actor_scale3d(unreal.Vector(45, 25, 8))
    nav_system = unreal.NavigationSystemV1.get_navigation_system(world)
    if not nav_system:
        raise RuntimeError("Cogspire Harbor has no navigation system")
    nav_system.on_navigation_bounds_updated(nav)
    unreal.WyrmTerrainDiagnostics.refresh_navigation_data_for_actor(substrate)

    if not levels.save_current_level():
        raise RuntimeError("Failed to save L_CogspireHarbor")
    REPORT.parent.mkdir(parents=True, exist_ok=True)
    REPORT.write_text(json.dumps({
        "status": "PASS", "engine": unreal.SystemLibrary.get_engine_version(), "map": MAP,
        "scope": "saved environment/navigation foundation only",
        "arrival_anchor": "LM-COGSPIRE-ARRIVAL", "return_anchor": "LM-COGSPIRE-RETURN",
        "water_authority": water_owner.get_class().get_name(), "waterline_z_cm": WATER_Z,
        "terrain_authority": adapter.get_class().get_name(),
        "terrain_save_compatibility_id": "WYRMFALL.CogspireHarbor",
        "supplied_quay_and_jetty_piece_count": 41,
        "hidden_navigation_substrate": substrate.get_actor_label(),
        "not_claimed": ["travel allowlist", "regional subsystem", "encounters", "city-engine behavior", "save schema"],
    }, indent=2) + "\n", encoding="utf-8")
    print("WP-23.6 Cogspire Harbor map composition: PASS")


main()
