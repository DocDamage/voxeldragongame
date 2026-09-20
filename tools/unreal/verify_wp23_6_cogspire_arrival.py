"""Exercise the bounded Cogspire arrival-to-city-engine observation slice in live PIE."""

import json
from pathlib import Path
import time
import traceback

import unreal


ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
MAP = "/Game/WYRMFALL/World/Regions/L_CogspireHarbor"
REPORT_PATH = ROOT / "Saved/Diagnostics/WP23_6_cogspire_arrival_proof.json"
OUT = ROOT / "Saved/Diagnostics/WP23_6_CogspireArrival"

levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
editor = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
editor_actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
if not levels.load_level(MAP):
    raise RuntimeError("Could not load saved Cogspire Harbor map")
if not any(isinstance(actor, unreal.WyrmCharacter) for actor in editor_actors.get_all_level_actors()):
    player = editor_actors.spawn_actor_from_class(
        unreal.WyrmCharacter, unreal.Vector(-1400, 0, 310), unreal.Rotator())
    player.set_actor_label("DIAG_WP23_6_CogspireArrival_Player")
camera_origin = unreal.Vector(-2500, -1800, 1100)
camera_target = unreal.Vector(-350, 350, 250)
camera = editor_actors.spawn_actor_from_class(
    unreal.CameraActor, camera_origin,
    unreal.MathLibrary.find_look_at_rotation(camera_origin, camera_target))
camera.set_actor_label("DIAG_WP23_6_CogspireArrival_Camera")
OUT.mkdir(parents=True, exist_ok=True)

report = {
    "kind": "wp23_6_cogspire_arrival_observation_slice",
    "engine": unreal.SystemLibrary.get_engine_version(),
    "map": MAP,
    "status": "RUNNING",
    "acceptance": {},
    "not_claimed": [
        "Cogfang combat", "city-engine shutdown", "regional completion",
        "optional urban investigations", "save persistence or schema extension",
        "interactive keyboard or gamepad walkthrough", "new packaged build",
    ],
}
REPORT_PATH.parent.mkdir(parents=True, exist_ok=True)
REPORT_PATH.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")

started = time.monotonic()
ticks = 0
nav_attempts = 0
stage = "wait"
wait_ticks = 0
finished = False
capture_camera = None


def actors(world, cls):
    return list(unreal.GameplayStatics.get_all_actors_of_class(world, cls))


def write_report():
    REPORT_PATH.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")


def project(context, location):
    result = unreal.WyrmTerrainDiagnostics.project_navigation_point(
        context, location, unreal.Vector(240, 240, 500))
    if isinstance(result, (tuple, list)):
        point = next((value for value in result[1:] if isinstance(value, unreal.Vector)), unreal.Vector())
        return bool(result[0]), point
    return (True, result) if isinstance(result, unreal.Vector) else (False, unreal.Vector())


def tick(_delta):
    global ticks, nav_attempts, stage, wait_ticks, finished, capture_camera
    try:
        if finished:
            if not levels.is_in_play_in_editor():
                unreal.unregister_slate_post_tick_callback(handle)
                editor_actors.destroy_actor(camera)
                unreal.SystemLibrary.quit_editor()
            return
        if time.monotonic() - started > 240:
            raise RuntimeError("Timed out in " + stage)
        if wait_ticks:
            wait_ticks -= 1
            return
        if stage == "capture":
            screenshot = OUT / "01_arrival_to_city_engine.png"
            requested = bool(unreal.AutomationLibrary.take_high_res_screenshot(
                1280, 720, str(screenshot), capture_camera))
            report["screenshot"] = {"path": str(screenshot), "requested": requested}
            report["status"] = "PASS" if requested and all(report["acceptance"].values()) else "FAIL"
            write_report()
            stage = "capture_wait"
            wait_ticks = 75
            return
        if stage == "capture_wait":
            finished = True
            levels.editor_request_end_play()
            return

        world = editor.get_game_world()
        if not world or not levels.is_in_play_in_editor():
            return
        ticks += 1
        terrains = actors(world, unreal.GeoForgeInfiniteTerrainActor)
        if len(terrains) != 1:
            return
        terrain = terrains[0]
        stats = terrain.get_runtime_render_stats()
        if (ticks < 90 or stats.get_editor_property("loaded_chunk_count") < 49 or
                stats.get_editor_property("queued_chunk_generation_count") or
                stats.get_editor_property("queued_chunk_rebuild_count") or
                stats.get_editor_property("pending_chunk_apply_count") or
                unreal.WyrmTerrainDiagnostics.is_navigation_build_pending(terrain)):
            return

        players = actors(world, unreal.WyrmCharacter)
        triggers = actors(world, unreal.WyrmCogspireArrivalTrigger)
        sites = {actor.get_actor_label(): actor for actor in actors(world, unreal.WyrmCogspireObservationSite)}
        targets = {actor.get_actor_label(): actor for actor in actors(world, unreal.TargetPoint)}
        cameras = [actor for actor in actors(world, unreal.CameraActor)
                   if actor.get_actor_label() == "DIAG_WP23_6_CogspireArrival_Camera"]
        required_sites = {
            "COG_OBSERVE_PublicMachinery", "COG_OBSERVE_CoercionDiversion",
            "COG_OBSERVE_BaronAcknowledgment",
        }
        if len(players) != 1 or len(triggers) != 1 or not required_sites.issubset(sites) or len(cameras) != 1:
            return
        if "COG_ROUTE_ARRIVAL" not in targets or "COG_ROUTE_CITY_ENGINE" not in targets:
            raise RuntimeError("Missing authored arrival or city-engine route anchor")

        start_ok, start = project(targets["COG_ROUTE_ARRIVAL"], targets["COG_ROUTE_ARRIVAL"].get_actor_location())
        end_ok, end = project(targets["COG_ROUTE_CITY_ENGINE"], targets["COG_ROUTE_CITY_ENGINE"].get_actor_location())
        path_points = int(unreal.WyrmTerrainDiagnostics.find_complete_navigation_path_point_count(
            terrain, start, end)) if start_ok and end_ok else 0
        if not start_ok or not end_ok or path_points <= 0:
            nav_attempts += 1
            if nav_attempts % 120 == 1:
                substrate = next((actor for actor in actors(world, unreal.StaticMeshActor)
                                  if actor.get_actor_label() == "COG_NAV_HiddenDockSubstrate"), None)
                if substrate:
                    unreal.WyrmTerrainDiagnostics.refresh_navigation_data_for_actor(substrate)
            if nav_attempts < 600:
                return
            raise RuntimeError("Arrival-to-city-engine route did not produce a complete path")

        player = players[0]
        trigger = triggers[0]
        travel = unreal.WyrmWorldTravelSubsystem.get_world_travel_subsystem(world)
        region = unreal.WyrmCogspireSubsystem.get_cogspire_subsystem(world)
        if not travel or not region:
            raise RuntimeError("Cogspire travel or fact owner unavailable")
        region.reset_cogspire_state()

        public_site = sites["COG_OBSERVE_PublicMachinery"]
        diversion_site = sites["COG_OBSERVE_CoercionDiversion"]
        baron_site = sites["COG_OBSERVE_BaronAcknowledgment"]
        player.set_actor_location(baron_site.get_actor_location(), False, False)
        out_of_order_rejected = not baron_site.interact(player)
        player.set_actor_location(diversion_site.get_actor_location(), False, False)
        out_of_order_rejected = out_of_order_rejected and not diversion_site.interact(player)

        wrong_context_rejected = not trigger.trigger_arrival(player)
        arrived = travel.complete_arrival("CogspireHarbor", "LM-COGSPIRE-ARRIVAL")
        player.set_actor_location(trigger.get_actor_location(), False, False)
        arrival_committed = region.has_fact(unreal.Name("cogspire.arrival"))
        if not arrival_committed:
            arrival_committed = trigger.trigger_arrival(player)
        arrival_duplicate_rejected = not trigger.trigger_arrival(player)

        player.set_actor_location(public_site.get_actor_location(), False, False)
        public_committed = public_site.can_interact(player) and public_site.interact(player)
        public_duplicate_rejected = not public_site.interact(player)
        player.set_actor_location(diversion_site.get_actor_location(), False, False)
        diversion_committed = diversion_site.can_interact(player) and diversion_site.interact(player)
        diversion_duplicate_rejected = not diversion_site.interact(player)
        player.set_actor_location(baron_site.get_actor_location(), False, False)
        baron_committed = baron_site.can_interact(player) and baron_site.interact(player)
        baron_duplicate_rejected = not baron_site.interact(player)

        all_runtime = actors(world, unreal.Actor)
        labels = {actor.get_actor_label(): actor for actor in all_runtime}
        civic_labels = ("COG_ENV_CivicWaterPumpA", "COG_ENV_CivicWaterPumpB")
        civic_intact = all(label in labels and not labels[label].is_actor_being_destroyed() for label in civic_labels)
        baron_actor = labels.get("COG_ENV_BaronFeistCogwell")
        baron_mesh = baron_actor.get_component_by_class(unreal.SkeletalMeshComponent) if baron_actor else None
        supplied_baron = bool(baron_mesh and baron_mesh.get_editor_property("skeletal_mesh") and
                              "TVS_VoxelKnights_Captain" in
                              baron_mesh.get_editor_property("skeletal_mesh").get_path_name() and
                              baron_mesh.get_material(0))
        bounded = (not region.has_fact(unreal.Name("cogspire.engine_shutdown")) and
                   not region.has_fact(unreal.Name("cogspire.region_complete")))
        ordered_facts = all(region.has_fact(unreal.Name(fact)) for fact in (
            "cogspire.arrival", "cogspire.public_machinery_observed",
            "cogspire.coercion_diversion_observed", "cogspire.baron_acknowledged_diversion"))

        report["acceptance"] = {
            "arrival_requires_completed_authored_travel_context": bool(wrong_context_rejected and arrived and arrival_committed and arrival_duplicate_rejected),
            "out_of_order_observations_reject": out_of_order_rejected,
            "public_machinery_observation_commits_once": bool(public_committed and public_duplicate_rejected),
            "coercion_diversion_observation_commits_once": bool(diversion_committed and diversion_duplicate_rejected),
            "baron_acknowledgment_commits_once": bool(baron_committed and baron_duplicate_rejected),
            "ordered_runtime_facts_present": ordered_facts,
            "supplied_baron_mesh_and_material_present": supplied_baron,
            "ordinary_public_pumps_remain_present": civic_intact,
            "arrival_to_city_engine_navigation_complete": bool(start_ok and end_ok and path_points > 0),
            "shutdown_completion_and_optional_scope_absent": bounded,
        }
        report["measurements"] = {
            "route_path_points": path_points,
            "observation_site_count": len(sites),
            "public_pump_count": sum(label in labels for label in civic_labels),
            "travel_region": str(travel.get_current_region_id()),
            "travel_arrival_landmark": str(travel.get_arrival_landmark_id()),
        }
        if not all(report["acceptance"].values()):
            raise RuntimeError("Acceptance failure: " + repr(report["acceptance"]))

        capture_camera = cameras[0]
        capture_camera.set_actor_location(camera_origin, False, False)
        capture_camera.set_actor_rotation(
            unreal.MathLibrary.find_look_at_rotation(camera_origin, camera_target), False)
        controller = unreal.GameplayStatics.get_player_controller(world, 0)
        if controller:
            controller.set_view_target_with_blend(capture_camera, 0.0)
        stage = "capture"
        wait_ticks = 30
    except Exception as exc:
        report["status"] = "FAIL"
        report["failed_stage"] = stage
        report["error"] = repr(exc)
        report["traceback"] = traceback.format_exc()
        write_report()
        unreal.log_error(f"WP23_6_COGSPIRE_ARRIVAL_FAIL {stage}: {exc!r}")
        finished = True
        if levels.is_in_play_in_editor():
            levels.editor_request_end_play()


handle = unreal.register_slate_post_tick_callback(tick)
levels.editor_play_simulate()
unreal.log("WP-23.6 Cogspire arrival observation PIE proof started")
