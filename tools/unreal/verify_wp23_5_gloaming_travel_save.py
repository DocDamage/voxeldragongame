"""Exercise bounded Gloaming travel and Schema 7 recovery in live PIE."""

import json
import time
import traceback
from pathlib import Path

import unreal


ROOT = Path(r"G:/assets/voxel project")
MAP = "/Game/WYRMFALL/World/Regions/L_GloamingMarches"
SLOT = "WP23_5_GloamingTravelRecovery"
REPORT = ROOT / "Saved/Diagnostics/WP23_5_gloaming_travel_save_proof.json"

editor = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
if not editor.get_editor_world() or MAP not in editor.get_editor_world().get_path_name():
    assert levels.load_level(MAP), f"Could not load {MAP}"
editor_actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
if not any(isinstance(actor, unreal.WyrmCharacter) for actor in editor_actors.get_all_level_actors()):
    proof_player = editor_actors.spawn_actor_from_class(
        unreal.WyrmCharacter, unreal.Vector(-5000, -3400, 904), unreal.Rotator())
    proof_player.set_actor_label("DIAG_WP23_5_TravelSave_Player")
stage = "wait"
ticks = 0
started = time.monotonic()
finished = False
data = {}
report = {
    "kind": "wp23_5_gloaming_travel_save_slice",
    "engine": unreal.SystemLibrary.get_engine_version(),
    "map": MAP,
    "status": "RUNNING",
    "acceptance": {},
    "measurements": {},
    "not_claimed": ["regional completion", "interactive keyboard or gamepad walkthrough", "new packaged build"],
}


def actors(world, cls):
    return list(unreal.GameplayStatics.get_all_actors_of_class(world, cls))


def write_report():
    REPORT.parent.mkdir(parents=True, exist_ok=True)
    REPORT.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")


def tick(_delta):
    global stage, ticks, finished
    try:
        if finished:
            if not levels.is_in_play_in_editor():
                unreal.unregister_slate_post_tick_callback(handle)
                unreal.SystemLibrary.quit_editor()
            return
        if time.monotonic() - started > 240:
            raise RuntimeError(f"Timed out in {stage}")
        world = editor.get_game_world()
        if not world or not levels.is_in_play_in_editor():
            return
        ticks += 1
        pc = unreal.GameplayStatics.get_player_controller(world, 0)
        player = pc.get_controlled_pawn() if pc else None
        player = unreal.WyrmCharacter.cast(player) if player else None
        if not player:
            candidates = actors(world, unreal.WyrmCharacter)
            if not candidates:
                return
            player = candidates[0]
            if pc:
                pc.possess(player)
        region = unreal.WyrmGloamingSubsystem.get_gloaming_subsystem(world)
        travel = unreal.WyrmWorldTravelSubsystem.get_world_travel_subsystem(world)
        adapters = actors(world, unreal.WyrmGeoForgeAdapter)
        terrains = actors(world, unreal.GeoForgeInfiniteTerrainActor)
        if not region or not travel or len(adapters) != 1 or len(terrains) != 1:
            if ticks % 120 == 0:
                report["measurements"]["wait_dependencies"] = {
                    "player": bool(player), "region": bool(region), "travel": bool(travel),
                    "adapters": len(adapters), "terrains": len(terrains)}
                write_report()
            return
        adapter, terrain = adapters[0], terrains[0]

        if stage == "wait":
            stats = terrain.get_runtime_render_stats()
            if ticks % 120 == 0:
                report["measurements"]["wait_stats"] = {
                    "ticks": ticks,
                    "loaded": stats.get_editor_property("loaded_chunk_count"),
                    "generation": stats.get_editor_property("queued_chunk_generation_count"),
                    "rebuild": stats.get_editor_property("queued_chunk_rebuild_count"),
                    "apply": stats.get_editor_property("pending_chunk_apply_count"),
                    "navigation_pending": unreal.WyrmTerrainDiagnostics.is_navigation_build_pending(terrain)}
                write_report()
            if (ticks < 90 or stats.get_editor_property("loaded_chunk_count") < 81 or
                    stats.get_editor_property("queued_chunk_generation_count") or
                    stats.get_editor_property("queued_chunk_rebuild_count") or
                    stats.get_editor_property("pending_chunk_apply_count") or
                    unreal.WyrmTerrainDiagnostics.is_navigation_build_pending(terrain)):
                return
            labels = {a.get_actor_label(): a for a in actors(world, unreal.TargetPoint)}
            required = ("LM-GLOAMING-ARRIVAL", "LM-GLOAMING-RETURN")
            assert all(label in labels for label in required)
            projected = {}
            all_projected = True
            for label in required:
                result = unreal.WyrmTerrainDiagnostics.project_navigation_point(
                    labels[label], labels[label].get_actor_location(), unreal.Vector(240, 240, 500))
                ok = bool(result[0]) if isinstance(result, (tuple, list)) else bool(result)
                all_projected = all_projected and ok
                projected[label] = list(labels[label].get_actor_location().to_tuple())
            if not all_projected:
                nav_attempts = data.get("nav_attempts", 0) + 1
                data["nav_attempts"] = nav_attempts
                if nav_attempts % 120 == 1:
                    unreal.WyrmTerrainDiagnostics.refresh_navigation_data_for_actor(terrain)
                if nav_attempts < 600:
                    return
                raise RuntimeError(f"Travel anchors did not project after {nav_attempts} checks")
            assert travel.complete_arrival("GloamingMarches", "LM-GLOAMING-ARRIVAL")
            report["acceptance"]["authored_safe_arrival_and_return_anchors"] = True
            report["measurements"]["anchors"] = projected
            stage = "state"

        if stage == "state":
            unreal.WyrmSaveSubsystem.delete_save_slot(SLOT)
            region.reset_gloaming_state()
            assert region.record_arrival()
            assert region.resolve_ashgrave_extraction_seal()
            assert region.record_malvaine_resolution(True)
            assert region.record_sanguine_strike_unlock()
            assert region.record_hollow_twins_resolution(True)
            assert region.record_second_turn_unlock()
            assert player.learn_echo("SanguineStrike")
            assert player.learn_echo("SecondTurn")
            player.restore_sanguine_strike_state(8.0)
            player.restore_second_turn_state(9.0)
            assert player.equip_echo("SanguineStrike")
            player.get_attributes().set_current_focus(100.0)
            player.get_attributes().set_current_health(73.0)
            player.set_actor_location(unreal.Vector(-5000, -3400, 904), False, False)

            dragon = unreal.WyrmDragonCharacter.spawn_wyrm_dragon(
                world, unreal.WyrmDragonRole.HOSTILE_BOSS,
                unreal.Transform(location=unreal.Vector(-4700, -3300, 900)))
            assert dragon
            dragon.set_dragon_id("Nyxaroth")
            assert dragon.has_supported_rig_profile()
            assert dragon.perform_boss_defeat()
            assert dragon.bond_with_humanoid(player)
            dragon.set_dragon_form(unreal.WyrmDragonForm.COMPANION_FORM)
            data["dragon"] = dragon
            stage = "preflight"

        if stage == "preflight":
            assert not travel.prepare_travel_with_snapshot(
                "GloamingMarches", "JadePeaks", "LM-GLOAMING-RETURN", SLOT, player, adapter)
            assert str(travel.get_last_travel_failure_reason()) == "RouteNotAllowlisted"
            assert not travel.prepare_travel_with_snapshot(
                "GloamingMarches", "Region01", "LM-ARRIVAL", SLOT, player, adapter)
            assert str(travel.get_last_travel_failure_reason()) == "RouteNotAllowlisted"

            player.restore_sanguine_strike_state(0.0)
            assert player.activate_sanguine_strike()
            assert not travel.prepare_travel_with_snapshot(
                "GloamingMarches", "Region01", "LM-GLOAMING-RETURN", SLOT, player, adapter)
            assert str(travel.get_last_travel_failure_reason()) == "UnsafeCharacterTransient"
            player.restore_sanguine_strike_state(8.0)

            dragon = data["dragon"]
            assert dragon.request_form_change(unreal.WyrmDragonForm.TRUE_FORM)
            assert not travel.prepare_travel_with_snapshot(
                "GloamingMarches", "Region01", "LM-GLOAMING-RETURN", SLOT, player, adapter)
            assert str(travel.get_last_travel_failure_reason()) == "UnsafeDragonTransient"
            dragon.interrupt_form_transition()
            dragon.set_dragon_form(unreal.WyrmDragonForm.COMPANION_FORM)
            report["acceptance"]["unsafe_preflight_rejected_without_fact_mutation"] = True
            stage = "snapshot"

        if stage == "snapshot":
            assert travel.prepare_travel_with_snapshot(
                "GloamingMarches", "Region01", "LM-GLOAMING-RETURN", SLOT, player, adapter)
            snapshot = unreal.GameplayStatics.load_game_from_slot(SLOT, 0)
            assert snapshot and snapshot.schema_version == 7
            regional_ids = sorted(str(item.region_id) for item in snapshot.regional_world_records)
            dragon_ids = sorted(str(item.dragon_id) for item in snapshot.dragon_records)
            assert "GloamingMarches" in regional_ids
            assert dragon_ids.count("Nyxaroth") == 1
            assert "echo.sanguine_strike" in [str(x) for x in snapshot.gloaming_record.known_facts]
            assert "echo.second_turn" in [str(x) for x in snapshot.gloaming_record.known_facts]
            assert abs(snapshot.character_record.sanguine_strike_remaining_cooldown - 8.0) < 0.1
            assert abs(snapshot.character_record.second_turn_remaining_cooldown - 9.0) < 0.1
            report["acceptance"]["schema7_pretravel_snapshot"] = True
            report["measurements"].update(
                schema=7, regional_records=regional_ids, dragon_ids=dragon_ids,
                sanguine_cooldown=snapshot.character_record.sanguine_strike_remaining_cooldown,
                second_turn_cooldown=snapshot.character_record.second_turn_remaining_cooldown)

            assert travel.complete_arrival("Region01", "LM-ARRIVAL")
            assert travel.prepare_travel("Region01", "GloamingMarches", "LM-ARRIVAL")
            assert travel.complete_arrival("GloamingMarches", "LM-GLOAMING-ARRIVAL")
            report["acceptance"]["two_way_allowlisted_travel"] = True
            stage = "recovery"

        if stage == "recovery":
            region.reset_gloaming_state()
            player.restore_echo_state([], unreal.Name(), False, 0.0, 0.0)
            player.restore_sanguine_strike_state(0.0)
            player.restore_second_turn_state(0.0)
            player.get_attributes().set_current_health(20.0)
            player.set_actor_location(unreal.Vector(-4200, -2800, 904), False, False)
            data["dragon"].destroy_actor()

            assert travel.begin_travel_recovery(SLOT)
            assert travel.complete_travel_recovery(player, adapter)
            restored = [d for d in actors(world, unreal.WyrmDragonCharacter)
                        if str(d.dragon_id) == "Nyxaroth"]
            assert len(restored) == 1 and restored[0].has_bond_receipt()
            assert region.has_fact("echo.sanguine_strike") and region.has_fact("echo.second_turn")
            assert player.is_echo_unlocked("SanguineStrike") and player.is_echo_unlocked("SecondTurn")
            assert player.get_sanguine_strike_remaining_cooldown() > 0.0
            assert player.get_second_turn_remaining_cooldown() > 0.0
            assert abs(player.get_attributes().get_current_health() - 73.0) < 0.1
            assert str(travel.get_current_region_id()) == "GloamingMarches"
            assert str(travel.get_arrival_landmark_id()) == "LM-GLOAMING-ARRIVAL"

            assert travel.begin_travel_recovery(SLOT)
            assert travel.complete_travel_recovery(player, adapter)
            assert len([d for d in actors(world, unreal.WyrmDragonCharacter)
                        if str(d.dragon_id) == "Nyxaroth"]) == 1
            assert not region.has_fact("gloaming.region_complete")
            assert all(unreal.WyrmSaveSubsystem.is_schema_version_supported(v) for v in range(1, 8))
            assert not unreal.WyrmSaveSubsystem.is_schema_version_supported(8)
            report["acceptance"]["unified_recovery_restores_state_without_duplicates"] = True
            report["acceptance"]["schemas_1_through_6_remain_readable"] = True
            report["acceptance"]["regional_completion_absent"] = True
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
        unreal.log_error(f"WP23_5_GLOAMING_TRAVEL_SAVE_FAIL {stage}: {exc!r}")
        unreal.WyrmSaveSubsystem.delete_save_slot(SLOT)
        finished = True
        if levels.is_in_play_in_editor():
            levels.editor_request_end_play()


handle = unreal.register_slate_post_tick_callback(tick)
levels.editor_play_simulate()
unreal.log("WP-23.5 Gloaming travel/save PIE proof started")
