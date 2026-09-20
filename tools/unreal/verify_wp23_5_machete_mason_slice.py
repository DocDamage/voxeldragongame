"""Exercise the bounded Machete Mason required-horror encounter in live PIE."""

import json
import time
import traceback
from pathlib import Path

import unreal


ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
MAP = "/Game/WYRMFALL/World/Regions/L_GloamingMarches"
REPORT = ROOT / "Saved/Diagnostics/WP23_5_machete_mason_slice_proof.json"
OUT = ROOT / "Saved/Diagnostics/WP23_5_MacheteMason"

levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
editor = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
if not editor.get_editor_world() or MAP not in editor.get_editor_world().get_path_name():
    assert levels.load_level(MAP), f"Could not load {MAP}"
if not any(isinstance(actor, unreal.WyrmCharacter) for actor in actors.get_all_level_actors()):
    player = actors.spawn_actor_from_class(
        unreal.WyrmCharacter, unreal.Vector(4300, 2200, 904), unreal.Rotator())
    player.set_actor_label("DIAG_WP23_5_MacheteMason_Player")

camera_origin = unreal.Vector(5125, 1825, 1040)
camera_target = unreal.Vector(4750, 2150, 985)
camera = actors.spawn_actor_from_class(
    unreal.CameraActor, camera_origin,
    unreal.MathLibrary.find_look_at_rotation(camera_origin, camera_target))
camera.set_actor_label("DIAG_GLM_MacheteMasonEncounter")

OUT.mkdir(parents=True, exist_ok=True)
report = {
    "kind": "wp23_5_machete_mason_encounter_slice",
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
            region.record_hollow_twins_resolution(True) and
            region.record_michael_mire_resolution())


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
            screenshot = OUT / "01_machete_mason_encounter.png"
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
        encounters = world_actors(world, unreal.WyrmMacheteMasonCharacter)
        cameras = [actor for actor in world_actors(world, unreal.CameraActor)
                   if actor.get_actor_label() == "DIAG_GLM_MacheteMasonEncounter"]
        anchors = {actor.get_actor_label(): actor for actor in world_actors(world, unreal.TargetPoint)}
        adapters = world_actors(world, unreal.WyrmGeoForgeAdapter)
        if len(players) != 1 or len(encounters) != 1 or len(cameras) != 1 or len(adapters) != 1:
            return
        if "GLM_ROUTE_MICHAEL_MIRE" not in anchors or "GLM_ROUTE_MACHETE_MASON" not in anchors:
            raise RuntimeError("Missing Michael-Mire-to-Machete-Mason route anchors")

        player = players[0]
        mason = encounters[0]
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

        start_ok, start = project(mason, anchors["GLM_ROUTE_MICHAEL_MIRE"].get_actor_location())
        end_ok, end = project(mason, anchors["GLM_ROUTE_MACHETE_MASON"].get_actor_location())
        if not start_ok or not end_ok:
            nav_attempts += 1
            if nav_attempts % 120 == 1:
                unreal.WyrmTerrainDiagnostics.refresh_navigation_data_for_actor(terrain)
            if nav_attempts < 600:
                return
            raise RuntimeError("Machete Mason route anchors did not project")
        route_path_points = int(
            unreal.WyrmTerrainDiagnostics.find_complete_navigation_path_point_count(
                mason, start, end))
        route_ok = route_path_points > 0

        parts = mason.get_components_by_class(unreal.StaticMeshComponent)
        meshes = [part.get_editor_property("static_mesh") for part in parts]
        presentation_ok = bool(
            mason.has_complete_presentation() and mason.get_presentation_part_count() == 17 and
            len(parts) == 17 and all(meshes) and
            all("Jason" in mesh.get_path_name() for mesh in meshes) and
            all(part.get_material(0) for part in parts) and
            any("Sword_Jason" in mesh.get_path_name() for mesh in meshes) and
            187.0 <= mason.get_presentation_height() <= 189.0)

        region.reset_gloaming_state()
        mason.reset_encounter()
        rejected_before_michael = not mason.begin_encounter(player)
        prerequisites = prepare_prerequisites(region)
        started_encounter = mason.begin_encounter(player)
        attrs = mason.get_attributes()
        attrs.set_current_armor(0.0)
        initial_health = attrs.get_current_health()
        strikes = 0
        while attrs.get_current_health() > attrs.get_current_max_health() * 0.25 and strikes < 12:
            unreal.WyrmMeleeAttackAbility.apply_damage_effect(
                player.get_ability_system(), mason.get_ability_system(), 70.0)
            strikes += 1
        threshold_health = attrs.get_current_health()
        gas_threshold = 0.0 < threshold_health <= attrs.get_current_max_health() * 0.25
        resolved = mason.resolve_after_disarmed_submission(player)
        committed = bool(
            region.has_fact(unreal.Name("gloaming.machete_mason_resolved")) and
            region.has_receipt(unreal.Name("gloaming.machete_mason.disarmed_submission")))
        duplicate_rejected = not mason.resolve_after_disarmed_submission(player)

        snapshot = unreal.WyrmSaveSubsystem.create_snapshot_object(
            "WP23_5_MacheteMasonMemory", player, adapters[0], world)
        region.reset_gloaming_state()
        restored = bool(snapshot and unreal.WyrmSaveSubsystem.apply_snapshot_object(
            snapshot, player, adapters[0], world) and
            region.has_fact(unreal.Name("gloaming.machete_mason_resolved")) and
            region.has_receipt(unreal.Name("gloaming.machete_mason.disarmed_submission")))
        excluded_absent = bool(
            not region.has_fact(unreal.Name("gloaming.region_complete")) and
            not region.has_fact(unreal.Name("echo.machete_mason")) and
            not player.is_echo_unlocked("MacheteMason"))

        report["acceptance"] = {
            "michael_mire_anchor_projects_to_navigation": start_ok,
            "machete_mason_anchor_projects_to_navigation": end_ok,
            "michael_to_mason_path_valid_nonpartial": route_ok,
            "supplied_17_part_weapon_188cm_presentation": presentation_ok,
            "encounter_rejects_before_michael_mire": rejected_before_michael,
            "gas_disarmed_submission_commits_once": bool(
                prerequisites and started_encounter and gas_threshold and resolved and
                committed and duplicate_rejected),
            "gloaming_ledger_roundtrip_restores_receipt": restored,
            "echo_and_regional_completion_absent": excluded_absent,
        }
        report["measurements"] = {
            "part_count": len(parts),
            "presentation_height_cm": mason.get_presentation_height(),
            "initial_health": initial_health,
            "submission_health": threshold_health,
            "gas_damage_applications": strikes,
            "route_path_points": route_path_points,
        }
        if not all(report["acceptance"].values()):
            raise RuntimeError(f"Acceptance failure: {report['acceptance']}")

        mason.set_actor_location(unreal.Vector(4750, 2150, 902), False, True)
        mason.set_actor_rotation(unreal.Rotator(0, 0, 225), False)
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
        unreal.log_error(f"WP23_5_MACHETE_MASON_FAIL {stage}: {exc!r}")
        finished = True
        if levels.is_in_play_in_editor():
            levels.editor_request_end_play()


handle = unreal.register_slate_post_tick_callback(tick)
levels.editor_play_simulate()
unreal.log("WP-23.5 Machete Mason PIE proof started")
