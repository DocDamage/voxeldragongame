"""Exercise bounded Cogspire arrival/return and Schema 7 recovery in live PIE."""

import json
from pathlib import Path
import time
import traceback

import unreal


ROOT = Path(r"G:/assets/voxel project")
MAP = "/Game/WYRMFALL/World/Regions/L_CogspireHarbor"
SLOT = "WP23_6_CogspireTravelRecovery"
REPORT_PATH = ROOT / "Saved/Diagnostics/WP23_6_cogspire_travel_proof.json"

editor = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
actor_sub = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
if not levels.load_level(MAP):
    raise RuntimeError("Could not load saved Cogspire Harbor map")
if not any(isinstance(actor, unreal.WyrmCharacter) for actor in actor_sub.get_all_level_actors()):
    player = actor_sub.spawn_actor_from_class(
        unreal.WyrmCharacter, unreal.Vector(-1400, 0, 310), unreal.Rotator())
    player.set_actor_label("DIAG_WP23_6_CogspireTravel_Player")

report = {
    "kind": "wp23_6_cogspire_travel_slice",
    "engine": unreal.SystemLibrary.get_engine_version(),
    "map": MAP,
    "status": "RUNNING",
    "acceptance": {},
    "measurements": {},
    "not_claimed": ["actual cross-map loading", "regional gameplay", "encounters", "save schema extension"],
}
stage = "wait"
ticks = 0
started = time.monotonic()
finished = False
data = {}


def actors(world, cls):
    return list(unreal.GameplayStatics.get_all_actors_of_class(world, cls))


def write_report():
    REPORT_PATH.parent.mkdir(parents=True, exist_ok=True)
    REPORT_PATH.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")


def project(anchor):
    result = unreal.WyrmTerrainDiagnostics.project_navigation_point(
        anchor, anchor.get_actor_location(), unreal.Vector(240, 240, 500))
    return bool(result[0]) if isinstance(result, (tuple, list)) else bool(result)


def tick(_delta):
    global stage, ticks, finished
    try:
        if finished:
            if not levels.is_in_play_in_editor():
                unreal.unregister_slate_post_tick_callback(handle)
                unreal.SystemLibrary.quit_editor()
            return
        if time.monotonic() - started > 240:
            raise RuntimeError("Timed out in " + stage)
        world = editor.get_game_world()
        if not world or not levels.is_in_play_in_editor():
            return
        ticks += 1
        pc = unreal.GameplayStatics.get_player_controller(world, 0)
        player = unreal.WyrmCharacter.cast(pc.get_controlled_pawn()) if pc and pc.get_controlled_pawn() else None
        if not player:
            candidates = actors(world, unreal.WyrmCharacter)
            if not candidates:
                return
            player = candidates[0]
            if pc:
                pc.possess(player)
        travel = unreal.WyrmWorldTravelSubsystem.get_world_travel_subsystem(world)
        adapters = actors(world, unreal.WyrmGeoForgeAdapter)
        terrains = actors(world, unreal.GeoForgeInfiniteTerrainActor)
        if not travel or len(adapters) != 1 or len(terrains) != 1:
            return
        adapter, terrain = adapters[0], terrains[0]

        if stage == "wait":
            stats = terrain.get_runtime_render_stats()
            if (ticks < 90 or stats.get_editor_property("loaded_chunk_count") < 49 or
                    stats.get_editor_property("queued_chunk_generation_count") or
                    stats.get_editor_property("queued_chunk_rebuild_count") or
                    stats.get_editor_property("pending_chunk_apply_count") or
                    unreal.WyrmTerrainDiagnostics.is_navigation_build_pending(terrain)):
                if ticks % 120 == 0:
                    report["measurements"]["terrain_wait"] = {
                        "loaded": stats.get_editor_property("loaded_chunk_count"),
                        "generation": stats.get_editor_property("queued_chunk_generation_count"),
                        "rebuild": stats.get_editor_property("queued_chunk_rebuild_count"),
                        "apply": stats.get_editor_property("pending_chunk_apply_count"),
                    }
                    write_report()
                return
            labels = {actor.get_actor_label(): actor for actor in actors(world, unreal.TargetPoint)}
            required = ("LM-COGSPIRE-ARRIVAL", "LM-COGSPIRE-RETURN")
            assert all(label in labels for label in required)
            if not all(project(labels[label]) for label in required):
                attempts = data.get("nav_attempts", 0) + 1
                data["nav_attempts"] = attempts
                if attempts % 120 == 1:
                    substrate = next(
                        actor for actor in actors(world, unreal.StaticMeshActor)
                        if actor.get_actor_label() == "COG_NAV_HiddenDockSubstrate")
                    unreal.WyrmTerrainDiagnostics.refresh_navigation_data_for_actor(substrate)
                if attempts < 600:
                    return
                raise RuntimeError("Cogspire travel anchors did not project")
            assert str(travel.get_current_region_id()) == "CogspireHarbor"
            assert travel.complete_arrival("CogspireHarbor", "LM-COGSPIRE-ARRIVAL")
            report["acceptance"]["saved_arrival_and_return_anchors"] = True
            report["measurements"]["anchors"] = {
                label: list(labels[label].get_actor_location().to_tuple()) for label in required
            }
            stage = "preflight"

        if stage == "preflight":
            unreal.WyrmSaveSubsystem.delete_save_slot(SLOT)
            assert not travel.prepare_travel_with_snapshot(
                "CogspireHarbor", "JadePeaks", "LM-COGSPIRE-RETURN", SLOT, player, adapter)
            assert str(travel.get_last_travel_failure_reason()) == "RouteNotAllowlisted"
            assert not travel.prepare_travel_with_snapshot(
                "CogspireHarbor", "Region01", "LM-ARRIVAL", SLOT, player, adapter)
            assert str(travel.get_last_travel_failure_reason()) == "RouteNotAllowlisted"
            assert not unreal.WyrmSaveSubsystem.does_save_exist(SLOT)
            report["acceptance"]["wrong_routes_and_anchors_fail_without_snapshot"] = True
            stage = "snapshot"

        if stage == "snapshot":
            player.get_attributes().set_current_health(81.0)
            player.set_actor_location(unreal.Vector(-1400, 0, 310), False, False)
            assert travel.prepare_travel_with_snapshot(
                "CogspireHarbor", "Region01", "LM-COGSPIRE-RETURN", SLOT, player, adapter)
            snapshot = unreal.GameplayStatics.load_game_from_slot(SLOT, 0)
            assert snapshot and snapshot.schema_version == 7
            region_ids = sorted(str(record.region_id) for record in snapshot.regional_world_records)
            assert "CogspireHarbor" in region_ids
            assert str(snapshot.world_travel_record.current_region_id) == "CogspireHarbor"
            assert len(snapshot.terrain_record.terrain_delta_payload) > 0
            report["acceptance"]["schema7_pretravel_snapshot"] = True
            report["measurements"].update(
                schema=snapshot.schema_version,
                regional_records=region_ids,
                terrain_payload_bytes=len(snapshot.terrain_record.terrain_delta_payload),
            )

            assert travel.complete_arrival("Region01", "LM-ARRIVAL")
            assert travel.prepare_travel("Region01", "CogspireHarbor", "LM-ARRIVAL")
            assert travel.complete_arrival("CogspireHarbor", "LM-COGSPIRE-ARRIVAL")
            assert str(travel.get_current_region_id()) == "CogspireHarbor"
            assert str(travel.get_arrival_landmark_id()) == "LM-COGSPIRE-ARRIVAL"
            report["acceptance"]["two_way_allowlisted_arrival_return"] = True
            stage = "recovery"

        if stage == "recovery":
            player.get_attributes().set_current_health(12.0)
            player.set_actor_location(unreal.Vector(-900, 500, 310), False, False)
            assert travel.begin_travel_recovery(SLOT)
            assert travel.complete_travel_recovery(player, adapter)
            assert abs(player.get_attributes().get_current_health() - 81.0) < 0.1
            restored_location = player.get_actor_location()
            assert (restored_location - unreal.Vector(-1400, 0, 310)).length() < 1.0
            assert str(travel.get_current_region_id()) == "CogspireHarbor"
            assert str(travel.get_arrival_landmark_id()) == "LM-COGSPIRE-ARRIVAL"

            assert travel.begin_travel_recovery(SLOT)
            assert travel.complete_travel_recovery(player, adapter)
            assert snapshot.schema_version == 7
            report["acceptance"]["failure_safe_recovery_is_repeat_stable"] = True
            report["acceptance"]["schema_unchanged"] = True
            report["status"] = "PASS"
            write_report()
            unreal.WyrmSaveSubsystem.delete_save_slot(SLOT)
            finished = True
            levels.editor_request_end_play()
    except Exception as exc:
        report["status"] = "FAIL"
        report["failed_stage"] = stage
        report["error"] = repr(exc)
        report["traceback"] = traceback.format_exc()
        write_report()
        unreal.log_error(f"WP23_6_COGSPIRE_TRAVEL_FAIL {stage}: {exc!r}")
        unreal.WyrmSaveSubsystem.delete_save_slot(SLOT)
        finished = True
        if levels.is_in_play_in_editor():
            levels.editor_request_end_play()


write_report()
handle = unreal.register_slate_post_tick_callback(tick)
levels.editor_play_simulate()
unreal.log("WP-23.6 Cogspire travel PIE proof started")
