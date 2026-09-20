"""Verify the saved L_CogspireHarbor foundation in live PIE and capture evidence."""

import hashlib
import json
from pathlib import Path
import time
import traceback

import unreal


ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
MAP = "/Game/WYRMFALL/World/Regions/L_CogspireHarbor"
OUT = ROOT / "Saved/Diagnostics/WP23_6_CogspireMap"
REPORT_PATH = ROOT / "Saved/Diagnostics/WP23_6_cogspire_map_proof.json"
ARRIVAL = (-1400.0, 0.0, 205.0)
JETTY_END = (1650.0, 0.0, 205.0)

LEVELS = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
ACTORS = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
EDITOR = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
REPORT = {
    "kind": "wp23_6_saved_cogspire_harbor_foundation",
    "engine": unreal.SystemLibrary.get_engine_version(),
    "map": MAP,
    "status": "INITIALIZING",
    "scope": "saved environment/navigation foundation only",
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
    REPORT["captures"].append({"path": path.as_posix(), "sha256": sha256(path)})


try:
    OUT.mkdir(parents=True, exist_ok=True)
    if not LEVELS.load_level(MAP):
        raise RuntimeError("Could not load saved Cogspire Harbor map")
    labels = {actor.get_actor_label(): actor for actor in ACTORS.get_all_level_actors()}
    required = {
        "LM-COGSPIRE-ARRIVAL", "LM-COGSPIRE-RETURN", "COG_ROUTE_ARRIVAL",
        "COG_ROUTE_JETTY_END", "PlayerStart_CogspireArrival", "COG_NavMeshBounds",
        "COG_WATER_HarborAuthority", "COG_NAV_HiddenDockSubstrate",
        "COG_GeoForgeTerrain", "COG_GeoForgeAdapter",
        "COG_ENV_CivicWaterPumpA", "COG_ENV_CivicWaterPumpB", "COG_ENV_CoercionEnginePump",
    }
    missing = sorted(required.difference(labels))
    supplied = [label for label in labels if label.startswith("COG_ENV_")]
    if missing or len(supplied) < 50:
        raise RuntimeError(f"Saved map inventory failed: missing={missing}, supplied={len(supplied)}")
    water_count = sum(isinstance(actor, unreal.WyrmWaterVolume) for actor in labels.values())
    terrain_count = sum(isinstance(actor, unreal.GeoForgeInfiniteTerrainActor) for actor in labels.values())
    adapter_count = sum(isinstance(actor, unreal.WyrmGeoForgeAdapter) for actor in labels.values())
    if water_count != 1 or terrain_count != 1 or adapter_count != 1:
        raise RuntimeError(
            f"Owner inventory mismatch: water={water_count}, terrain={terrain_count}, adapter={adapter_count}")
    REPORT["saved_map_inventory"] = {
        "required_labels_present": True,
        "supplied_environment_actor_count": len(supplied),
        "water_authority_count": water_count,
        "terrain_authority_count": terrain_count,
        "terrain_adapter_count": adapter_count,
        "arrival_anchor": "LM-COGSPIRE-ARRIVAL",
        "return_anchor": "LM-COGSPIRE-RETURN",
    }

    capture_actor, capture_component, target = make_capture()
    views = [
        ("01_saved_harbor_overview.png", (-3300, -3600, 2400), (250, 0, 220)),
        ("02_saved_arrival_to_jetty.png", (-2200, -1800, 900), (500, 0, 190)),
        ("03_saved_public_machinery.png", (-2700, -1800, 900), (-650, 350, 250)),
    ]
    REPORT["status"] = "WAITING_FOR_PIE_NAVIGATION"
    write_report()
    started = time.monotonic()
    capture_started = [None]
    last_nav_refresh = [-1]
    stage = 0

    def tick(_delta):
        global stage
        try:
            if REPORT["status"] == "WAITING_FOR_PIE_NAVIGATION":
                game_world = EDITOR.get_game_world()
                if not game_world or not LEVELS.is_in_play_in_editor():
                    return
                runtime_labels = {
                    actor.get_actor_label(): actor for actor in
                    unreal.GameplayStatics.get_all_actors_of_class(game_world, unreal.Actor)
                }
                start_actor = runtime_labels.get("COG_ROUTE_ARRIVAL")
                end_actor = runtime_labels.get("COG_ROUTE_JETTY_END")
                if not start_actor or not end_actor:
                    return
                start_ok, start_point = nav_projection(start_actor, ARRIVAL)
                end_ok, end_point = nav_projection(end_actor, JETTY_END)
                path = unreal.NavigationSystemV1.find_path_to_location_synchronously(
                    game_world, start_point if start_ok else unreal.Vector(*ARRIVAL),
                    end_point if end_ok else unreal.Vector(*JETTY_END))
                complete = bool(path and path.is_valid() and not path.is_partial() and len(path.path_points) >= 2)
                recast_count = len(unreal.GameplayStatics.get_all_actors_of_class(game_world, unreal.RecastNavMesh))
                if not (start_ok and end_ok and complete and recast_count == 1):
                    refresh_slot = int(time.monotonic() - started) // 3
                    if refresh_slot > last_nav_refresh[0]:
                        last_nav_refresh[0] = refresh_slot
                        runtime_substrate = runtime_labels.get("COG_NAV_HiddenDockSubstrate")
                        if runtime_substrate:
                            unreal.WyrmTerrainDiagnostics.refresh_navigation_data_for_actor(runtime_substrate)
                        runtime_nav = unreal.NavigationSystemV1.get_navigation_system(game_world)
                        if runtime_nav:
                            for bound in unreal.GameplayStatics.get_all_actors_of_class(
                                    game_world, unreal.NavMeshBoundsVolume):
                                runtime_nav.on_navigation_bounds_updated(bound)
                    if time.monotonic() - started > 40.0:
                        raise RuntimeError(
                            f"Saved-map navigation failed: start={start_ok} end={end_ok} "
                            f"complete={complete} recast={recast_count}")
                    return
                runtime_water = unreal.GameplayStatics.get_all_actors_of_class(game_world, unreal.WyrmWaterVolume)
                runtime_terrain = unreal.GameplayStatics.get_all_actors_of_class(
                    game_world, unreal.GeoForgeInfiniteTerrainActor)
                runtime_adapters = unreal.GameplayStatics.get_all_actors_of_class(
                    game_world, unreal.WyrmGeoForgeAdapter)
                REPORT["live_pie"] = {
                    "arrival_projects": start_ok,
                    "jetty_end_projects": end_ok,
                    "complete_non_partial_path": complete,
                    "path_point_count": len(path.path_points),
                    "path_points": [list(point.to_tuple()) for point in path.path_points],
                    "recast_count": recast_count,
                    "water_authority_count": len(runtime_water),
                    "water_surface_z_cm": runtime_water[0].get_editor_property("surface_elevation"),
                    "terrain_authority_count": len(runtime_terrain),
                    "terrain_adapter_count": len(runtime_adapters),
                }
                if (len(runtime_water) != 1 or len(runtime_terrain) != 1 or len(runtime_adapters) != 1 or
                        abs(REPORT["live_pie"]["water_surface_z_cm"] - 80.0) > 0.1):
                    raise RuntimeError("Saved-map owner inventory mismatch")
                REPORT["status"] = "WAITING_FOR_PIE_END"
                write_report()
                LEVELS.editor_request_end_play()
                return
            if REPORT["status"] == "WAITING_FOR_PIE_END":
                if LEVELS.is_in_play_in_editor():
                    return
                substrate = next((actor for actor in ACTORS.get_all_level_actors()
                                  if actor.get_actor_label() == "COG_NAV_HiddenDockSubstrate"), None)
                if substrate:
                    component = substrate.get_component_by_class(unreal.StaticMeshComponent)
                    if component:
                        component.set_visibility(False, True)
                REPORT["status"] = "CAPTURING"
                capture_started[0] = time.monotonic()
                return
            if time.monotonic() - capture_started[0] < 2.5 + stage * 1.8:
                return
            if stage < len(views):
                capture(capture_actor, capture_component, target, *views[stage])
                stage += 1
                write_report()
                return
            REPORT["status"] = "PASS"
            REPORT["acceptance"] = {
                "saved_map_exists": True,
                "supplied_harbor_art": "PASS",
                "existing_water_authority": "PASS",
                "existing_terrain_authority": "PASS",
                "arrival_return_anchors": "PASS_LABELS_ONLY",
                "live_pie_navigation": "PASS",
                "travel_allowlist_verified_by_this_packet": False,
                "encounters_added": False,
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
