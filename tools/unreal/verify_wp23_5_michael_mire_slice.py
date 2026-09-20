"""Exercise the bounded Michael Mire required-horror encounter in live PIE."""

import json
import time
import traceback
from pathlib import Path

import unreal


ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
MAP = "/Game/WYRMFALL/World/Regions/L_GloamingMarches"
REPORT = ROOT / "Saved/Diagnostics/WP23_5_michael_mire_slice_proof.json"
OUT = ROOT / "Saved/Diagnostics/WP23_5_MichaelMire"

levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
editor = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
if not editor.get_editor_world() or MAP not in editor.get_editor_world().get_path_name():
    assert levels.load_level(MAP), f"Could not load {MAP}"
if not any(isinstance(actor, unreal.WyrmCharacter) for actor in actors.get_all_level_actors()):
    player = actors.spawn_actor_from_class(
        unreal.WyrmCharacter, unreal.Vector(3900, 1700, 904), unreal.Rotator())
    player.set_actor_label("DIAG_WP23_5_MichaelMire_Player")

camera_origin = unreal.Vector(5300, 2700, 1050)
camera_target = unreal.Vector(4950, 3000, 990)
camera = actors.spawn_actor_from_class(
    unreal.CameraActor, camera_origin,
    unreal.MathLibrary.find_look_at_rotation(camera_origin, camera_target))
camera.set_actor_label("DIAG_GLM_MichaelMireEncounter")

OUT.mkdir(parents=True, exist_ok=True)
report = {
    "kind": "wp23_5_michael_mire_encounter_slice",
    "engine": unreal.SystemLibrary.get_engine_version(),
    "map": MAP,
    "status": "RUNNING",
    "acceptance": {},
    "not_claimed": [
        "new Echo reward", "additional required-horror identities", "regional completion",
        "interactive keyboard or gamepad walkthrough", "new packaged build"],
}
REPORT.parent.mkdir(parents=True, exist_ok=True)
REPORT.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")

started = time.monotonic()
ticks = 0
finished = False
stage = "wait"
wait_ticks = 0
nav_attempts = 0
capture_camera = None


def world_actors(world, cls):
    return list(unreal.GameplayStatics.get_all_actors_of_class(world, cls))


def prepare_prerequisites(region):
    return (region.record_arrival() and region.resolve_ashgrave_extraction_seal() and
            region.record_malvaine_resolution(True) and
            region.record_hollow_twins_resolution(True))


def tick(_delta):
    global ticks, finished, stage, wait_ticks, nav_attempts, capture_camera
    try:
        if finished:
            if not levels.is_in_play_in_editor():
                unreal.unregister_slate_post_tick_callback(handle)
                actors.destroy_actor(camera)
                unreal.SystemLibrary.quit_editor()
            return
        if time.monotonic() - started > 240:
            raise RuntimeError(f"Timed out in {stage}")
        if wait_ticks:
            wait_ticks -= 1
            return
        if stage == "capture":
            screenshot = OUT / "01_michael_mire_encounter.png"
            requested = bool(unreal.AutomationLibrary.take_high_res_screenshot(
                1280, 720, str(screenshot), capture_camera))
            report["screenshot"] = {"path": str(screenshot), "requested": requested}
            report["status"] = (
                "PASS_AUTOMATED_REQUIRES_MANUAL_VISUAL_REVIEW"
                if requested and all(report["acceptance"].values()) else "FAIL")
            REPORT.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
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
        terrains = world_actors(world, unreal.GeoForgeInfiniteTerrainActor)
        if len(terrains) != 1:
            return
        terrain = terrains[0]
        stats = terrain.get_runtime_render_stats()
        if (ticks < 90 or stats.get_editor_property("loaded_chunk_count") < 81 or
                stats.get_editor_property("queued_chunk_generation_count") or
                stats.get_editor_property("queued_chunk_rebuild_count") or
                stats.get_editor_property("pending_chunk_apply_count") or
                unreal.WyrmTerrainDiagnostics.is_navigation_build_pending(terrain)):
            return

        players = world_actors(world, unreal.WyrmCharacter)
        encounters = world_actors(world, unreal.WyrmMichaelMireCharacter)
        cameras = [actor for actor in world_actors(world, unreal.CameraActor)
                   if actor.get_actor_label() == "DIAG_GLM_MichaelMireEncounter"]
        anchors = {actor.get_actor_label(): actor for actor in world_actors(world, unreal.TargetPoint)}
        adapters = world_actors(world, unreal.WyrmGeoForgeAdapter)
        if len(players) != 1 or len(encounters) != 1 or len(cameras) != 1 or len(adapters) != 1:
            return
        if "GLM_ROUTE_TWINS" not in anchors or "GLM_ROUTE_MICHAEL_MIRE" not in anchors:
            raise RuntimeError("Missing Twins-to-Michael-Mire route anchors")

        player = players[0]
        michael = encounters[0]
        region = unreal.WyrmGloamingSubsystem.get_gloaming_subsystem(world)
        if not region:
            raise RuntimeError("Gloaming fact owner unavailable")

        def project(context, location):
            result = unreal.WyrmTerrainDiagnostics.project_navigation_point(
                context, location, unreal.Vector(240, 240, 500))
            if isinstance(result, (tuple, list)):
                point = next((item for item in result[1:] if isinstance(item, unreal.Vector)), unreal.Vector())
                return bool(result[0]), point
            return (True, result) if isinstance(result, unreal.Vector) else (False, unreal.Vector())

        start_ok, start = project(michael, anchors["GLM_ROUTE_TWINS"].get_actor_location())
        end_ok, end = project(michael, anchors["GLM_ROUTE_MICHAEL_MIRE"].get_actor_location())
        if not start_ok or not end_ok:
            nav_attempts += 1
            if nav_attempts % 120 == 1:
                unreal.WyrmTerrainDiagnostics.refresh_navigation_data_for_actor(terrain)
            if nav_attempts < 600:
                return
            raise RuntimeError("Michael Mire route anchors did not project")
        route_path_points = int(
            unreal.WyrmTerrainDiagnostics.find_complete_navigation_path_point_count(
                michael, start, end))
        route_ok = route_path_points > 0

        parts = michael.get_components_by_class(unreal.StaticMeshComponent)
        meshes = [part.get_editor_property("static_mesh") for part in parts]
        presentation_ok = bool(
            michael.has_complete_presentation() and michael.get_presentation_part_count() == 17 and
            len(parts) == 17 and all(meshes) and
            all("MicahelMeyers" in mesh.get_path_name() for mesh in meshes) and
            all(part.get_material(0) for part in parts) and
            189.0 <= michael.get_presentation_height() <= 191.0)

        region.reset_gloaming_state()
        michael.reset_encounter()
        rejected_before_twins = not michael.begin_encounter(player)
        prerequisites = prepare_prerequisites(region)
        started_encounter = michael.begin_encounter(player)
        attrs = michael.get_attributes()
        attrs.set_current_armor(0.0)
        initial_health = attrs.get_current_health()
        strikes = 0
        while attrs.get_current_health() > attrs.get_current_max_health() * 0.25 and strikes < 12:
            unreal.WyrmMeleeAttackAbility.apply_damage_effect(
                player.get_ability_system(), michael.get_ability_system(), 65.0)
            strikes += 1
        threshold_health = attrs.get_current_health()
        gas_threshold = 0.0 < threshold_health <= attrs.get_current_max_health() * 0.25
        resolved = michael.resolve_after_living_submission(player)
        committed = bool(
            region.has_fact(unreal.Name("gloaming.michael_mire_resolved")) and
            region.has_receipt(unreal.Name("gloaming.michael_mire.living_submission")))
        duplicate_rejected = not michael.resolve_after_living_submission(player)

        snapshot = unreal.WyrmSaveSubsystem.create_snapshot_object(
            "WP23_5_MichaelMireMemory", player, adapters[0], world)
        region.reset_gloaming_state()
        restored = bool(snapshot and unreal.WyrmSaveSubsystem.apply_snapshot_object(
            snapshot, player, adapters[0], world) and
            region.has_fact(unreal.Name("gloaming.michael_mire_resolved")) and
            region.has_receipt(unreal.Name("gloaming.michael_mire.living_submission")))
        excluded_absent = bool(
            not region.has_fact(unreal.Name("gloaming.region_complete")) and
            not region.has_fact(unreal.Name("echo.michael_mire")) and
            not player.is_echo_unlocked("MichaelMire"))

        report["acceptance"] = {
            "twins_anchor_projects_to_navigation": start_ok,
            "michael_mire_anchor_projects_to_navigation": end_ok,
            "twins_to_michael_mire_path_valid_nonpartial": route_ok,
            "supplied_17_part_190cm_presentation": presentation_ok,
            "encounter_rejects_before_hollow_twins": rejected_before_twins,
            "gas_living_submission_commits_once": bool(
                prerequisites and started_encounter and gas_threshold and resolved and
                committed and duplicate_rejected),
            "gloaming_ledger_roundtrip_restores_receipt": restored,
            "echo_and_regional_completion_absent": excluded_absent,
        }
        report["measurements"] = {
            "part_count": len(parts),
            "presentation_height_cm": michael.get_presentation_height(),
            "initial_health": initial_health,
            "submission_health": threshold_health,
            "gas_damage_applications": strikes,
            "route_path_points": route_path_points,
        }
        if not all(report["acceptance"].values()):
            raise RuntimeError(f"Acceptance failure: {report['acceptance']}")

        michael.set_actor_location(unreal.Vector(4950, 3000, 903), False, True)
        michael.set_actor_rotation(unreal.Rotator(0, 0, 180), False)
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
        REPORT.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
        unreal.log_error(f"WP23_5_MICHAEL_MIRE_FAIL {stage}: {exc!r}")
        finished = True
        if levels.is_in_play_in_editor():
            levels.editor_request_end_play()


handle = unreal.register_slate_post_tick_callback(tick)
levels.editor_play_simulate()
unreal.log("WP-23.5 Michael Mire PIE proof started")
