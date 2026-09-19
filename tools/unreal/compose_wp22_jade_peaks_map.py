"""Compose and save the bounded L_JadePeaks production-region slice."""

import json
import time
from pathlib import Path

import unreal

ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
MAP = "/Game/WYRMFALL/World/Regions/L_JadePeaks"
REPORT = ROOT / "Saved/Diagnostics/WP22_jade_peaks_map_composition.json"
BASE = "/Game/WYRMFALL/Development/Intake/WP22/JadePeaks/Palace"
DIRT = "/Game/WYRMFALL/Development/Intake/WP00/Dirt/dirt-1/Materials/palette"
PRODUCTION_GROUND = "/Game/WYRMFALL/World/Regions/Materials/M_JadePeaksGround"


def ensure_ground_material(assets):
    """Create the simple production-slice ground without intake texture tiling."""
    material = assets.load_asset(PRODUCTION_GROUND) if assets.does_asset_exist(PRODUCTION_GROUND) else None
    if material:
        return material
    tools = unreal.AssetToolsHelpers.get_asset_tools()
    material = tools.create_asset(
        "M_JadePeaksGround", "/Game/WYRMFALL/World/Regions/Materials",
        unreal.Material, unreal.MaterialFactoryNew())
    if not material:
        raise RuntimeError("Could not create the Jade Peaks ground material")
    editing = unreal.MaterialEditingLibrary
    color = editing.create_material_expression(material, unreal.MaterialExpressionVectorParameter, -300, 0)
    color.set_editor_property("parameter_name", "GroundColor")
    color.set_editor_property("default_value", unreal.LinearColor(0.16, 0.25, 0.20, 1.0))
    editing.connect_material_property(color, "RGB", unreal.MaterialProperty.MP_BASE_COLOR)
    roughness = editing.create_material_expression(material, unreal.MaterialExpressionConstant, -300, 180)
    roughness.set_editor_property("r", 0.92)
    editing.connect_material_property(roughness, "", unreal.MaterialProperty.MP_ROUGHNESS)
    editing.recompile_material(material)
    if not assets.save_loaded_asset(material):
        raise RuntimeError("Could not save the Jade Peaks ground material")
    return material


def main():
    levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    assets = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
    if assets.does_asset_exist(MAP):
        opened = levels.load_level(MAP)
    else:
        opened = levels.new_level(MAP)
    if not opened:
        raise RuntimeError(f"Could not create or load {MAP}")
    world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
    for actor in list(actors.get_all_level_actors()):
        actors.destroy_actor(actor)

    game_mode = unreal.load_class(None, "/Script/WYRMFALL.WyrmGameMode")
    if game_mode:
        world.get_world_settings().set_editor_property("default_game_mode", game_mode)

    sun = actors.spawn_actor_from_class(unreal.DirectionalLight, unreal.Vector(0, 0, 2500), unreal.Rotator(-32, -38, 0))
    sun.set_actor_label("JADE_DirectionalLight")
    sun_component = sun.get_component_by_class(unreal.DirectionalLightComponent)
    sun_component.set_mobility(unreal.ComponentMobility.MOVABLE)
    sun_component.set_editor_property("intensity", 1.5)
    sun_component.set_editor_property("light_color", unreal.Color(255, 232, 205, 255))
    sky = actors.spawn_actor_from_class(unreal.SkyLight, unreal.Vector(0, 0, 1800))
    sky.set_actor_label("JADE_SkyLight")
    sky_component = sky.get_component_by_class(unreal.SkyLightComponent)
    sky_component.set_mobility(unreal.ComponentMobility.MOVABLE)
    sky_component.set_editor_property("intensity", 0.8)
    sky_component.set_editor_property("real_time_capture", True)
    actors.spawn_actor_from_class(unreal.SkyAtmosphere, unreal.Vector(0, 0, 1000)).set_actor_label("JADE_SkyAtmosphere")
    fog = actors.spawn_actor_from_class(unreal.ExponentialHeightFog, unreal.Vector(0, 0, 650))
    fog.set_actor_label("JADE_HeightFog")
    fog_component = fog.get_component_by_class(unreal.ExponentialHeightFogComponent)
    fog_component.set_editor_property("fog_density", 0.018)
    fog_component.set_editor_property("fog_height_falloff", 0.22)

    terrain = actors.spawn_actor_from_class(unreal.GeoForgeInfiniteTerrainActor, unreal.Vector(0, 0, 0))
    terrain.set_actor_label("JADE_GeoForgeTerrain")
    settings = {
        "auto_rebuild_in_editor": False, "auto_apply_preset_defaults": False,
        "follow_editor_viewport_camera_for_streaming": False,
        "world_shape": unreal.GeoForgeWorldShape.PLANAR_FINITE,
        "horizontal_bounds_mode": unreal.GeoForgeHorizontalBoundsMode.FINITE_CENTERED,
        "finite_bounds_half_extent_x_in_chunks": 4, "finite_bounds_half_extent_y_in_chunks": 4,
        "chunk_size_in_cells": 16, "chunk_height_in_cells": 16, "cell_size": 100.0,
        # The bounded palace/temple route must be traversable and safe for the
        # 400 cm Mirror Step. Jade Peaks height comes from the authored palace,
        # aerie, and flight composition rather than collision-noisy spawn ground.
        "ground_level_in_cells": 8, "max_mountain_height_in_cells": 0,
        "water_level_in_cells": -100, "river_depth_in_cells": 0, "generate_caves": False,
        "enable_built_in_cube_trees": False, "generate_collision": True,
        "persist_edited_cells_across_streaming": True, "use_async_chunk_generation": False,
        "max_queued_chunk_rebuilds_per_tick": 64,
        "chunk_render_mode": unreal.GeoForgeChunkRenderMode.MARCHING_CUBES_SURFACE,
        "world_seed": 2202, "save_compatibility_id": "WYRMFALL.JadePeaks",
        "streamed_terrain_affects_navigation": True,
    }
    for key, value in settings.items():
        terrain.set_editor_property(key, value)
    palette = ensure_ground_material(assets)
    if palette:
        for key in ("terrain_material", "surface_material", "soil_material", "rock_material",
                    "deep_rock_material", "sand_material", "snow_material"):
            terrain.set_editor_property(key, palette)
        terrain.set_editor_property("use_general_biome_terrain_material", True)
        terrain.set_editor_property("general_biome_terrain_material", palette)

    adapter = actors.spawn_actor_from_class(unreal.WyrmGeoForgeAdapter, unreal.Vector(0, 0, 0))
    adapter.set_actor_label("JADE_GeoForgeAdapter")
    adapter.bind_terrain_actor(terrain)
    nav = actors.spawn_actor_from_class(unreal.NavMeshBoundsVolume, unreal.Vector(0, 0, 900))
    nav.set_actor_label("JADE_NavMeshBounds")
    nav.set_actor_scale3d(unreal.Vector(100, 100, 30))
    nav_origin, nav_extent = nav.get_actor_bounds(False)
    if min(nav_extent.x, nav_extent.y, nav_extent.z) <= 0:
        raise RuntimeError("Jade Peaks NavMeshBoundsVolume has no usable brush bounds")
    nav_system = unreal.NavigationSystemV1.get_navigation_system(world)
    if not nav_system:
        raise RuntimeError("Jade Peaks world has no navigation system")
    nav_system.on_navigation_bounds_updated(nav)

    # Every walkable anchor sits outside the palace/prop collision envelopes.
    # The route proceeds west-to-east around the palace rather than through it.
    landmarks = [
        ("LM-JADE-ARRIVAL", unreal.Vector(-4800, -3000, 900)),
        ("LM-JADE-PALACE", unreal.Vector(-1800, -1500, 900)),
        ("LM-JADE-STORMCOURT", unreal.Vector(1600, -1300, 900)),
        ("LM-JADE-MIRRORWELL", unreal.Vector(1600, 300, 900)),
        ("LM-JADE-AERIE", unreal.Vector(4000, 1700, 900)),
        ("LM-JADE-RETURN", unreal.Vector(-5200, -3700, 900)),
    ]
    for landmark_id, location in landmarks:
        landmark = actors.spawn_actor_from_class(unreal.WyrmJadePeaksLandmark, location)
        landmark.set_actor_label(f"Landmark_{landmark_id}")
        landmark.set_editor_property("LandmarkId", unreal.Name(landmark_id))

    actors.spawn_actor_from_class(unreal.PlayerStart, unreal.Vector(-4800, -3000, 996)).set_actor_label("PlayerStart_JadeArrival")

    placements = [
        ("Palace", "Environment/TVS_VoxelPalace_Palace", unreal.Vector(0, 600, 900), unreal.Vector(0.85, 0.85, 0.85)),
        ("MirrorFountain", "Environment/TVS_VoxelPalace_Fountain", unreal.Vector(2050, 300, 900), unreal.Vector(1, 1, 1)),
        ("PalaceTreeWest", "Environment/TVS_VoxelPalace_PottedTree", unreal.Vector(-1150, -850, 1042), unreal.Vector(1, 1, 1)),
        ("PalaceTreeEast", "Environment/TVS_VoxelPalace_PottedTree", unreal.Vector(-650, -850, 1042), unreal.Vector(1, 1, 1)),
        ("StormTreeNorth", "Environment/TVS_VoxelPalace_Tree", unreal.Vector(2200, -850, 906), unreal.Vector(1.25, 1.25, 1.25)),
        ("StormTreeSouth", "Environment/TVS_VoxelPalace_Tree", unreal.Vector(2300, -1750, 906), unreal.Vector(1.0, 1.0, 1.0)),
        ("AerieTree", "Environment/TVS_VoxelPalace_Tree", unreal.Vector(4550, 2350, 906), unreal.Vector(1.5, 1.5, 1.5)),
        ("AerieFountain", "Environment/TVS_VoxelPalace_Fountain", unreal.Vector(4300, 2050, 900), unreal.Vector(0.8, 0.8, 0.8)),
        ("AerieTreeWest", "Environment/TVS_VoxelPalace_PottedTree", unreal.Vector(3750, 2050, 1042), unreal.Vector(1, 1, 1)),
        ("AerieTreeEast", "Environment/TVS_VoxelPalace_PottedTree", unreal.Vector(4750, 1950, 1042), unreal.Vector(1, 1, 1)),
        ("ApproachFenceWest", "Environment/TVS_VoxelPalace_Fence", unreal.Vector(-2600, -2050, 978), unreal.Vector(2, 1, 1)),
        ("ApproachFenceEast", "Environment/TVS_VoxelPalace_Fence", unreal.Vector(-2200, -1750, 978), unreal.Vector(2, 1, 1)),
        ("StormFenceSouth", "Environment/TVS_VoxelPalace_Fence", unreal.Vector(900, -1750, 978), unreal.Vector(2, 1, 1)),
        ("StormFenceNorth", "Environment/TVS_VoxelPalace_Fence", unreal.Vector(1200, -900, 978), unreal.Vector(2, 1, 1)),
        ("MirrorFence", "Environment/TVS_VoxelPalace_Fence", unreal.Vector(1500, -250, 978), unreal.Vector(2, 1, 1)),
        ("AerieFenceWest", "Environment/TVS_VoxelPalace_Fence", unreal.Vector(3300, 1000, 978), unreal.Vector(2, 1, 1)),
        ("AerieFenceEast", "Environment/TVS_VoxelPalace_Fence", unreal.Vector(3700, 1350, 978), unreal.Vector(2, 1, 1)),
        ("DiscipleSpear", "Props/TVS_VoxelPalace_Spear", unreal.Vector(2100, 800, 915), unreal.Vector(1, 1, 1)),
    ]
    placed_assets = []
    for label, relative, location, scale in placements:
        mesh = assets.load_asset(f"{BASE}/{relative}")
        if not mesh:
            raise RuntimeError(f"Missing imported supplied asset: {BASE}/{relative}")
        actor = actors.spawn_actor_from_class(unreal.StaticMeshActor, location)
        actor.static_mesh_component.set_static_mesh(mesh)
        actor.set_actor_scale3d(scale)
        actor.set_actor_label("JADE_" + label)
        placed_assets.append(mesh.get_path_name())

    jadefang = actors.spawn_actor_from_class(unreal.WyrmDragonCharacter, unreal.Vector(4750, 2250, 950))
    jadefang.set_actor_label("JADE_Jadefang")
    jadefang.set_dragon_id(unreal.Name("Jadefang"))
    jadefang.set_dragon_role(unreal.WyrmDragonRole.ALLIED_COMPANION)
    jadefang.set_dragon_form(unreal.WyrmDragonForm.TRUE_FORM)

    disciple = actors.spawn_actor_from_class(unreal.WyrmJadeDiscipleCharacter, unreal.Vector(2100, 750, 988))
    disciple.set_actor_label("JADE_TempleDisciple")

    if not levels.save_current_level():
        raise RuntimeError("Failed to save L_JadePeaks")
    report = {
        "status": "PASS", "map": MAP, "engine": unreal.SystemLibrary.get_engine_version(),
        "landmarks": [item[0] for item in landmarks], "real_assets": sorted(set(placed_assets)),
        "dragon": "Jadefang", "encounter": "TempleDisciple",
        "terrain_material": palette.get_path_name() if palette else None,
        "nav_bounds_extent_cm": [nav_extent.x, nav_extent.y, nav_extent.z],
        "nav_data_notified_after_scale": True,
    }
    REPORT.parent.mkdir(parents=True, exist_ok=True)
    REPORT.write_text(json.dumps(report, indent=2), encoding="utf-8")
    print(f"WP-22 map composition complete at {time.strftime('%H:%M:%S')}")


if __name__ == "__main__":
    main()
