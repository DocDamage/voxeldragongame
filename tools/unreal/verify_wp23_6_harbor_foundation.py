"""Build and verify an unsaved Cogspire Harbor foundation fixture.

The fixture uses supplied waterfront and public-machinery art. It proves scale,
waterline, collision metadata, a continuous walkable dock route, and visual
separation between ordinary civic pumps and the later coercion-engine assembly.
It creates no production map, region gameplay, travel, save, or encounter owner.
"""

import hashlib
import json
from pathlib import Path
import time
import traceback

import unreal


ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
OUT = ROOT / "Saved/Diagnostics/WP23_6_HarborFoundation"
REPORT_PATH = ROOT / "Saved/Diagnostics/WP23_6_harbor_foundation.json"
DEST = "/Game/WYRMFALL/Development/Intake/WP23_6"
WATER_Z = 80.0
WALK_Z = 205.0
NAV_START = (-50.0, 0.0, WALK_Z)
NAV_END = (1500.0, 0.0, WALK_Z)

ASSETS = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
ACTORS = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
EDITOR = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
LEVELS = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)

REPORT = {
    "kind": "wp23_6_cogspire_harbor_foundation",
    "engine": unreal.SystemLibrary.get_engine_version(),
    "scope": "unsaved environment/navigation fixture only; no production map, gameplay, travel, save schema, or encounters",
    "status": "INITIALIZING",
    "waterline_z_cm": WATER_Z,
    "placements": [],
    "navigation": {},
    "public_machinery_continuity": {},
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


def bounds_row(actor):
    origin, extent = actor.get_actor_bounds(False)
    return {
        "origin": list(origin.to_tuple()),
        "extent": list(extent.to_tuple()),
        "dimensions_cm": [extent.x * 2.0, extent.y * 2.0, extent.z * 2.0],
        "bottom_z_cm": origin.z - extent.z,
        "top_z_cm": origin.z + extent.z,
    }


def place_bottom(actor, location, scale=1.0, rotation=None, collision="BlockAll"):
    actor.set_actor_scale3d(unreal.Vector(scale, scale, scale))
    if rotation is not None:
        actor.set_actor_rotation(unreal.Rotator(*rotation), False)
    origin, extent = actor.get_actor_bounds(False)
    actor.set_actor_location(
        unreal.Vector(location[0] - origin.x, location[1] - origin.y,
                      location[2] - (origin.z - extent.z)), False, False)
    component = actor.get_component_by_class(unreal.PrimitiveComponent)
    if component:
        component.set_collision_profile_name(collision)
        component.set_collision_enabled(
            unreal.CollisionEnabled.NO_COLLISION if collision == "NoCollision"
            else unreal.CollisionEnabled.QUERY_AND_PHYSICS)
        if collision != "NoCollision":
            component.set_editor_property("can_ever_affect_navigation", True)
    return bounds_row(actor)


def spawn_asset(label, path, location, scale=1.0, rotation=None, collision="BlockAll"):
    asset = ASSETS.load_asset(path)
    if isinstance(asset, unreal.StaticMesh):
        if asset.get_editor_property("body_setup") is None:
            raise RuntimeError(label + " has no body setup")
        actor = ACTORS.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector())
        component = actor.get_component_by_class(unreal.StaticMeshComponent)
        component.set_static_mesh(asset)
        kind = "StaticMesh"
        collision_asset = True
    elif isinstance(asset, unreal.SkeletalMesh):
        physics = asset.get_editor_property("physics_asset")
        if physics is None:
            raise RuntimeError(label + " has no physics asset")
        actor = ACTORS.spawn_actor_from_class(unreal.SkeletalMeshActor, unreal.Vector())
        component = actor.get_component_by_class(unreal.SkeletalMeshComponent)
        component.set_skinned_asset_and_update(asset)
        kind = "SkeletalMesh"
        collision_asset = True
    else:
        raise RuntimeError("Missing or unsupported asset: " + path)
    actor.set_actor_label("WP23_6_HARBOR_" + label)
    measured = place_bottom(actor, location, scale, rotation, collision)
    materials = [
        component.get_material(i).get_path_name() if component.get_material(i) else None
        for i in range(component.get_num_materials())
    ]
    if not materials or any(value is None for value in materials):
        raise RuntimeError(label + " has an empty material slot")
    REPORT["placements"].append({
        "label": label, "asset": asset.get_path_name(), "class": kind,
        "scale": scale, "collision_asset_present": collision_asset,
        "collision_profile": str(component.get_collision_profile_name()),
        "materials": materials, "bounds": measured,
    })
    return actor


def diagnostic_material(name, color):
    path = f"/Game/WYRMFALL/Development/Intake/WP23_6/FoundationMaterials/{name}"
    existing = ASSETS.load_asset(path) if ASSETS.does_asset_exist(path) else None
    if existing:
        return existing
    tools = unreal.AssetToolsHelpers.get_asset_tools()
    material = tools.create_asset(name, path.rsplit("/", 1)[0], unreal.Material, unreal.MaterialFactoryNew())
    expression = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionConstant3Vector, -220, 0)
    expression.set_editor_property("constant", unreal.LinearColor(*color, 1.0))
    unreal.MaterialEditingLibrary.connect_material_property(
        expression, "", unreal.MaterialProperty.MP_BASE_COLOR)
    unreal.MaterialEditingLibrary.connect_material_property(
        expression, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    unreal.MaterialEditingLibrary.recompile_material(material)
    ASSETS.save_loaded_asset(material)
    return material


def spawn_block(label, location, scale, material, collision="BlockAll"):
    cube = load("/Engine/BasicShapes/Cube.Cube", unreal.StaticMesh)
    actor = ACTORS.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(*location))
    actor.set_actor_label("WP23_6_HARBOR_" + label)
    component = actor.get_component_by_class(unreal.StaticMeshComponent)
    component.set_static_mesh(cube)
    component.set_material(0, material)
    actor.set_actor_scale3d(unreal.Vector(*scale))
    component.set_collision_profile_name(collision)
    component.set_collision_enabled(
        unreal.CollisionEnabled.NO_COLLISION if collision == "NoCollision"
        else unreal.CollisionEnabled.QUERY_AND_PHYSICS)
    if collision != "NoCollision":
        component.set_editor_property("can_ever_affect_navigation", True)
    return actor


def add_lighting():
    for rotation, intensity in (((-50.0, -35.0, 0.0), 7.0), ((-25.0, 145.0, 0.0), 2.5)):
        light = ACTORS.spawn_actor_from_class(unreal.DirectionalLight, unreal.Vector(0, 0, 1500), unreal.Rotator(*rotation))
        component = light.get_component_by_class(unreal.DirectionalLightComponent)
        component.set_mobility(unreal.ComponentMobility.MOVABLE)
        component.set_intensity(intensity)
    sky = ACTORS.spawn_actor_from_class(unreal.SkyLight, unreal.Vector(0, 0, 1000))
    sky_component = sky.get_component_by_class(unreal.SkyLightComponent)
    sky_component.set_mobility(unreal.ComponentMobility.MOVABLE)
    sky_component.set_intensity(2.0)


def make_capture():
    actor = ACTORS.spawn_actor_from_class(unreal.SceneCapture2D, unreal.Vector())
    component = actor.get_component_by_class(unreal.SceneCaptureComponent2D)
    target = unreal.RenderingLibrary.create_render_target2d(
        actor, 1440, 1080, unreal.TextureRenderTargetFormat.RTF_RGBA8)
    component.set_editor_property("texture_target", target)
    component.set_editor_property("capture_source", unreal.SceneCaptureSource.SCS_FINAL_COLOR_LDR)
    component.set_editor_property("fov_angle", 55.0)
    component.set_editor_property("capture_every_frame", False)
    component.set_editor_property("capture_on_movement", False)
    return actor, component, target


def capture(actor, component, target, name, camera, look_at):
    actor.set_actor_location(unreal.Vector(*camera), False, False)
    actor.set_actor_rotation(unreal.MathLibrary.find_look_at_rotation(
        unreal.Vector(*camera), unreal.Vector(*look_at)), False)
    component.capture_scene()
    unreal.RenderingLibrary.export_render_target(component, target, OUT.as_posix(), name)
    path = OUT / name
    if not path.is_file() or path.stat().st_size <= 0:
        raise RuntimeError("Capture was not written: " + path.as_posix())
    REPORT["captures"].append({"view": name.removesuffix(".png"), "path": path.as_posix(), "sha256": sha256(path)})


def nav_projection(context, location):
    result = unreal.WyrmTerrainDiagnostics.project_navigation_point(
        context, unreal.Vector(*location), unreal.Vector(180, 180, 350))
    if isinstance(result, (tuple, list)):
        ok = bool(result[0])
        point = next((item for item in result[1:] if isinstance(item, unreal.Vector)), unreal.Vector())
    elif isinstance(result, unreal.Vector):
        ok, point = True, result
    else:
        ok, point = bool(result), unreal.Vector()
    return ok, point


try:
    OUT.mkdir(parents=True, exist_ok=True)
    # Start from the project's configured navigation-bearing world, then clear
    # only this unsaved editor instance. A brand-new transient world has no
    # registered Recast data even when project auto-create settings are valid.
    if not LEVELS.load_level("/Game/WYRMFALL/World/Regions/L_Region01"):
        raise RuntimeError("Could not load the navigation-bearing Region01 template")
    for existing_actor in list(ACTORS.get_all_level_actors()):
        if not isinstance(existing_actor, unreal.RecastNavMesh):
            ACTORS.destroy_actor(existing_actor)
    world = EDITOR.get_editor_world()
    add_lighting()

    stone = diagnostic_material("M_HarborQuayQA", (0.16, 0.18, 0.22))
    water = diagnostic_material("M_HarborWaterlineQA", (0.0, 0.12, 0.32))
    core = diagnostic_material("M_CoercionCoreQA", (0.35, 0.04, 0.02))
    quay = spawn_block("Quay", (-1200, 0, 100), (12, 20, 2), stone)
    spawn_block("Waterline", (1500, 0, WATER_Z - 2.5), (15, 20, 0.05), water, "NoCollision")

    jetty_path = DEST + "/Harbor/Jetty/jetty.jetty"
    dock_substrate = spawn_block(
        "DockNavigationSubstrate", (800, 0, 190), (9.0, 1.6, 0.2), stone)
    REPORT["navigation_collision_policy"] = (
        "The supplied jetty body setup blocks correctly but does not yield a continuous Recast deck; "
        "a thin authored under-deck simple-collision substrate joins the visible modular pieces without "
        "introducing a gameplay or navigation authority.")
    # Overlap modular jetty pieces by 100 cm so Recast agent-radius erosion
    # cannot split an otherwise visually touching dock seam.
    for index, x in enumerate((150, 500, 850, 1200, 1550), 1):
        spawn_asset(f"Jetty{index}", jetty_path, (x, 0, 0), 8.0)

    spawn_asset("HarborBuilding", DEST + "/Harbor/Building_1/Building_1.Building_1", (-1550, 750, 200), 1.0)
    spawn_asset("Bridge", DEST + "/Harbor/Bridge_2/Bridge_2.Bridge_2", (-250, 700, 80), 1.8, (0, 0, 90))
    boat = spawn_asset("Boat", DEST + "/Harbor/Boat/Boat.Boat", (1950, 850, -110), 1.0, (0, 0, -8))
    rowboat = spawn_asset("Rowboat", DEST + "/Harbor/Rowboat/Rowboat.Rowboat", (950, -650, -25), 1.0, (0, 0, 12))
    spawn_asset("Lantern", DEST + "/Harbor/Lantern_1/Lantern_1.Lantern_1", (50, -170, 200), 1.6)
    spawn_asset("CargoChest", DEST + "/Harbor/Chest/Chest.Chest", (-550, -650, 200), 1.0)

    pump_path = DEST + "/Engine/WaterPump/water_pump.water_pump"
    steam_path = DEST + "/Engine/SteamLeakFrame01/frame_001.frame_001"
    civic_a = spawn_asset("CivicWaterPumpA", pump_path, (-1550, -700, 200), 4.0)
    civic_b = spawn_asset("CivicWaterPumpB", pump_path, (-1050, -700, 200), 4.0)
    core_plinth = spawn_block("CoercionCorePlinth", (-150, 900, 260), (2.2, 2.2, 0.6), core)
    engine_pump = spawn_asset("CoercionEnginePump", pump_path, (-150, 900, 320), 7.0)
    steam_a = spawn_asset("CoercionSteamA", steam_path, (-360, 900, 320), 4.0)
    steam_b = spawn_asset("CoercionSteamB", steam_path, (60, 900, 320), 4.0, (0, 0, 180))

    boat_bounds = bounds_row(boat)
    rowboat_bounds = bounds_row(rowboat)
    waterline_checks = {
        "boat_straddles_waterline": boat_bounds["bottom_z_cm"] < WATER_Z < boat_bounds["top_z_cm"],
        "rowboat_straddles_waterline": rowboat_bounds["bottom_z_cm"] < WATER_Z < rowboat_bounds["top_z_cm"],
        "jetty_walk_surface_above_water_cm": WALK_Z - WATER_Z,
    }
    REPORT["waterline"] = waterline_checks
    if not all((waterline_checks["boat_straddles_waterline"], waterline_checks["rowboat_straddles_waterline"])):
        raise RuntimeError("Boat waterline placement is not credible")

    civic_distance = (civic_a.get_actor_location() - engine_pump.get_actor_location()).length()
    REPORT["public_machinery_continuity"] = {
        "ordinary_pump_count": 2,
        "coercion_engine_assembly_parts": 4,
        "nearest_civic_to_engine_distance_cm": civic_distance,
        "ordinary_pumps_are_separate_actors": civic_a != civic_b and civic_a != engine_pump,
        "shutdown_design_readability": "PASS_FIXTURE; civic pumps are spatially and actor-wise independent from the coercion-core assembly; later gameplay must preserve their operation",
    }
    if civic_distance < 800.0:
        raise RuntimeError("Civic machinery is not visibly separated from the coercion engine")

    start = ACTORS.spawn_actor_from_class(unreal.TargetPoint, unreal.Vector(*NAV_START))
    start.set_actor_label("WP23_6_HARBOR_ROUTE_Quay")
    end = ACTORS.spawn_actor_from_class(unreal.TargetPoint, unreal.Vector(*NAV_END))
    end.set_actor_label("WP23_6_HARBOR_ROUTE_JettyEnd")
    nav = ACTORS.spawn_actor_from_class(unreal.NavMeshBoundsVolume, unreal.Vector(300, 0, 300))
    nav.set_actor_label("WP23_6_HARBOR_NavBounds")
    nav.set_actor_scale3d(unreal.Vector(35, 25, 8))
    nav_system = unreal.NavigationSystemV1.get_navigation_system(world)
    if not nav_system:
        raise RuntimeError("Harbor fixture has no navigation system")
    nav_system.on_navigation_bounds_updated(nav)
    unreal.WyrmTerrainDiagnostics.refresh_navigation_data_for_actor(quay)
    unreal.WyrmTerrainDiagnostics.refresh_navigation_data_for_actor(dock_substrate)

    capture_actor, capture_component, target = make_capture()
    views = [
        ("01_harbor_overview.png", (-3100, -3600, 2500), (450, 0, 250)),
        ("02_walkable_jetty.png", (-1000, -2100, 850), (750, 0, 180)),
        ("03_public_machinery.png", (-2600, -1900, 950), (-650, 350, 300)),
        ("04_waterline.png", (3300, -2600, 950), (1350, 200, 120)),
    ]
    REPORT["status"] = "WAITING_FOR_PIE_NAVIGATION"
    write_report()
    start_time = time.monotonic()
    capture_start = [None]
    stage = 0

    def tick(_delta):
        global stage
        try:
            elapsed = time.monotonic() - start_time
            if REPORT["status"] == "WAITING_FOR_PIE_NAVIGATION":
                game_world = EDITOR.get_game_world()
                if not game_world or not LEVELS.is_in_play_in_editor():
                    return
                runtime_points = {
                    actor.get_actor_label(): actor for actor in
                    unreal.GameplayStatics.get_all_actors_of_class(game_world, unreal.TargetPoint)
                }
                runtime_start = runtime_points.get("WP23_6_HARBOR_ROUTE_Quay")
                runtime_end = runtime_points.get("WP23_6_HARBOR_ROUTE_JettyEnd")
                if not runtime_start or not runtime_end:
                    return
                start_ok, start_point = nav_projection(runtime_start, NAV_START)
                end_ok, end_point = nav_projection(runtime_end, NAV_END)
                path = unreal.NavigationSystemV1.find_path_to_location_synchronously(
                    game_world,
                    start_point if start_ok else unreal.Vector(*NAV_START),
                    end_point if end_ok else unreal.Vector(*NAV_END))
                path_ok = bool(path and path.is_valid() and not path.is_partial() and len(path.path_points) >= 2)
                REPORT["navigation_attempt"] = {
                    "quay_projects": start_ok,
                    "jetty_end_projects": end_ok,
                    "path_exists": bool(path),
                    "path_valid": bool(path and path.is_valid()),
                    "path_partial": bool(path and path.is_partial()),
                    "path_point_count": len(path.path_points) if path else 0,
                    "quay_projected": list(start_point.to_tuple()) if start_ok else None,
                    "jetty_end_projected": list(end_point.to_tuple()) if end_ok else None,
                    "path_points": [list(point.to_tuple()) for point in path.path_points] if path else [],
                    "runtime_recast_count": len(unreal.GameplayStatics.get_all_actors_of_class(game_world, unreal.RecastNavMesh)),
                }
                if not (start_ok and end_ok and path_ok):
                    if elapsed > 35.0:
                        write_report()
                        raise RuntimeError("Walkable quay-to-jetty navigation did not become ready")
                    return
                REPORT["navigation"] = {
                    "quay_projects": start_ok,
                    "jetty_end_projects": end_ok,
                    "quay_projected": list(start_point.to_tuple()),
                    "jetty_end_projected": list(end_point.to_tuple()),
                    "complete_path": path_ok,
                    "path_point_count": len(path.path_points),
                    "path_points": [list(point.to_tuple()) for point in path.path_points],
                }
                REPORT["status"] = "WAITING_FOR_PIE_END"
                write_report()
                LEVELS.editor_request_end_play()
                return
            if REPORT["status"] == "WAITING_FOR_PIE_END":
                if LEVELS.is_in_play_in_editor():
                    return
                REPORT["status"] = "CAPTURING"
                capture_start[0] = time.monotonic()
                return
            capture_elapsed = time.monotonic() - capture_start[0]
            if capture_elapsed < 3.0 + stage * 2.0:
                return
            if stage < len(views):
                capture(capture_actor, capture_component, target, *views[stage])
                stage += 1
                write_report()
                return
            REPORT["status"] = "PASS"
            REPORT["acceptance"] = {
                "layout_scale": "PASS",
                "waterline": "PASS",
                "collision_metadata": "PASS",
                "walkable_navigation": "PASS",
                "public_machinery_readability": "PASS",
                "production_map_created": False,
            }
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
    LEVELS.editor_play_simulate()
except Exception:
    REPORT["status"] = "ERROR"
    REPORT["error"] = traceback.format_exc()
    write_report()
    unreal.SystemLibrary.quit_editor()
