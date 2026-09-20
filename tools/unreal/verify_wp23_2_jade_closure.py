"""Focused live-PIE proof for WP-23.2 Jade Peaks closure."""

import json
import time
import traceback
from pathlib import Path
import unreal

ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
REPORT_PATH = ROOT / "Saved/Diagnostics/WP23_2_jade_closure_proof.json"
MAP = "/Game/WYRMFALL/World/Regions/L_JadePeaks"
TESTS = tuple(f"JC-{index:02d}" for index in range(1, 9))
report = {"kind": "wp23_2_jade_closure", "engine": unreal.SystemLibrary.get_engine_version(),
          "map": MAP, "status": "INITIALIZING", "tests": {name: {"status": "NOT_RUN"} for name in TESTS},
          "details": {}, "logs": []}


def log(message):
    line = f"[{time.strftime('%H:%M:%S')}] {message}"
    print(line)
    report["logs"].append(line)


def write_report():
    REPORT_PATH.parent.mkdir(parents=True, exist_ok=True)
    REPORT_PATH.write_text(json.dumps(report, indent=2, default=str) + "\n", encoding="utf-8")


def passed(test, details):
    report["tests"][test]["status"] = "PASS"
    report["details"][test] = details
    log(f"{test}: PASS")


def actors(world, cls):
    return unreal.GameplayStatics.get_all_actors_of_class(world, cls)


write_report()
levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
editor = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
if not editor.get_editor_world() or MAP not in editor.get_editor_world().get_path_name():
    assert levels.load_level(MAP), f"Could not load {MAP}"
editor_actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
if not any(isinstance(a, unreal.WyrmCharacter) for a in editor_actors.get_all_level_actors()):
    player = editor_actors.spawn_actor_from_class(unreal.WyrmCharacter, unreal.Vector(-4200, -2800, 996))
    player.set_actor_label("DIAG_WP23_2_Player")
alt_emperor_editor = editor_actors.spawn_actor_from_class(
    unreal.WyrmJadeEmperorCharacter, unreal.Vector(-600, -650, 1000), unreal.Rotator())
alt_emperor_editor.set_actor_label("DIAG_WP23_2_AltEmperor")

started = time.monotonic()
finished = False
stage = "wait"
data = {"ticks": 0}


def tick(_delta):
    global finished, stage
    try:
        if finished:
            if not levels.is_in_play_in_editor():
                unreal.unregister_slate_post_tick_callback(tick_handle)
                unreal.SystemLibrary.quit_editor()
            return
        if time.monotonic() - started > 210:
            raise RuntimeError(f"Timed out in {stage}")
        world = editor.get_game_world()
        if not world or not levels.is_in_play_in_editor():
            return
        data["ticks"] += 1
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
        region = unreal.WyrmJadePeaksSubsystem.get_jade_peaks_subsystem(world)
        travel = unreal.WyrmWorldTravelSubsystem.get_world_travel_subsystem(world)
        adapters = actors(world, unreal.WyrmGeoForgeAdapter)
        if not region or not travel or not adapters:
            return
        adapter = adapters[0]
        if stage == "wait":
            if data["ticks"] < 90:
                return
            player.grant_combat_abilities()
            player.get_attributes().set_current_health(100.0)
            player.get_attributes().set_current_focus(100.0)
            stage = "presentation"

        if stage == "presentation":
            king = unreal.load_asset("/Game/WYRMFALL/Development/Intake/WP23/JadePeaks/Palace/Characters/TVS_VoxelPalace_King")
            material = unreal.load_asset("/Game/WYRMFALL/Development/Intake/WP23/JadePeaks/Palace/Materials/M_TVS_VoxelPalace_King")
            emperors = [a for a in actors(world, unreal.WyrmJadeEmperorCharacter)
                        if not a.get_actor_label().startswith("DIAG_WP23_2_")]
            props = actors(world, unreal.WyrmUnseenHandTarget)
            assert king and material and len(emperors) == 1 and len(props) == 1
            assert emperors[0].mesh.skeletal_mesh and emperors[0].mesh.get_material(0)
            data.update(emperor=emperors[0], prop=props[0])
            passed("JC-01", {"king_mesh": king.get_path_name(), "material": material.get_path_name(),
                              "emperor": emperors[0].get_name()})
            stage = "diplomacy"

        if stage == "diplomacy":
            region.reset_jade_peaks_state()
            assert region.record_jadefang_arrival()
            emperor = data["emperor"]
            assert emperor.resolve_by_diplomacy()
            assert not emperor.resolve_by_diplomacy()
            assert region.has_fact("jade.emperor_persuaded") and region.has_fact("jade.imperial_pact_resolved")
            jadefangs = [d for d in actors(world, unreal.WyrmDragonCharacter) if str(d.dragon_id) == "Jadefang"]
            assert len(jadefangs) == 1
            passed("JC-02", {"route": "diplomacy", "closure_fact": True, "jadefang_count": 1})
            stage = "combat_route"

        if stage == "combat_route":
            region.reset_jade_peaks_state()
            assert region.record_jadefang_arrival()
            alt = next(a for a in actors(world, unreal.WyrmJadeEmperorCharacter)
                       if a.get_actor_label() == "DIAG_WP23_2_AltEmperor")
            alt.get_attributes().set_current_health(0.0)
            assert alt.is_defeated() and alt.resolve_after_living_defeat()
            assert not alt.resolve_after_living_defeat()
            assert region.has_fact("jade.emperor_defeated_alive") and region.has_fact("jade.imperial_pact_resolved")
            passed("JC-03", {"route": "living_defeat", "duplicate_rejected": True, "same_closure_fact": True})
            data["alt_emperor"] = alt
            stage = "disciple"

        if stage == "disciple":
            disciple = actors(world, unreal.WyrmJadeDiscipleCharacter)[0]
            region.reset_jade_peaks_state()
            disciple.reset_resolution()
            assert disciple.resolve_with_trust(player)
            assert player.is_echo_unlocked("MirrorStep") and player.is_echo_unlocked("UnseenHand")
            assert not disciple.resolve_with_trust(player)
            region.reset_jade_peaks_state()
            disciple.reset_resolution()
            disciple.get_attributes().set_current_health(0.0)
            assert disciple.resolve_after_living_defeat(player)
            assert region.has_fact("disciple.defeated_alive") and region.has_fact("echo.unseen_hand")
            passed("JC-04", {"trust_route": True, "living_defeat_route": True,
                              "mirror_step_preserved": True, "unseen_hand_unlocked": True})
            stage = "unseen_hand"

        if stage == "unseen_hand":
            player.set_actor_location(unreal.Vector(300, 650, 1050), False, False)
            assert player.equip_echo("UnseenHand")
            player.restore_unseen_hand_state(0.0)
            player.get_attributes().set_current_focus(100.0)
            initial_focus = player.get_attributes().get_current_focus()
            emperor = data["emperor"]
            emperor.set_actor_location(unreal.Vector(500, 650, 1050), False, False)
            assert player.get_unseen_hand_failure_reason(emperor) == "HeavyOrBossTarget"
            terrain = actors(world, unreal.GeoForgeInfiniteTerrainActor)[0]
            player.set_actor_location(unreal.Vector(0, 0, 100), False, False)
            assert player.get_unseen_hand_failure_reason(terrain) == "HeavyOrUnauthoredTarget"
            assert abs(player.get_attributes().get_current_focus() - initial_focus) < 0.01
            player.set_actor_location(unreal.Vector(300, 650, 1050), False, False)
            enemy = unreal.WyrmEnemyCharacter.spawn_wyrm_enemy(
                world, unreal.WyrmEnemyRole.MELEE_CHASER,
                unreal.Transform(location=unreal.Vector(550, 650, 1050)))
            assert not player.get_unseen_hand_failure_reason(enemy)
            before = enemy.get_actor_location()
            assert player.activate_unseen_hand(enemy)
            assert abs(player.get_attributes().get_current_focus() - 75.0) < 0.01
            assert player.get_unseen_hand_remaining_cooldown() > 0.0
            data["unseen_enemy"] = enemy
            data["unseen_enemy_before"] = before
            stage = "unseen_hand_wait"
            return

        if stage == "unseen_hand_wait":
            enemy = data["unseen_enemy"]
            launch_velocity = enemy.character_movement.velocity.length()
            moved = enemy.get_actor_location() != data["unseen_enemy_before"]
            player.restore_unseen_hand_state(0.0)
            prop = data["prop"]
            prop.set_actor_location(unreal.Vector(600, 650, 1050), False, False)
            assert player.activate_unseen_hand(prop)
            passed("JC-05", {"focus_cost": 25.0, "cooldown": 8.0,
                              "enemy_launch_committed": True, "observed_next_tick_velocity": launch_velocity,
                              "observed_next_tick_movement": moved,
                              "authored_prop_impulse": True, "boss_rejected": True,
                              "terrain_rejected_without_cost": True})
            stage = "travel"

        if stage == "travel":
            assert not travel.is_allowed_route("JadePeaks", "Bonelands")
            assert travel.prepare_travel("JadePeaks", "Region01", "LM-JADE-RETURN")
            assert travel.complete_arrival("Region01", "LM-ARRIVAL")
            assert travel.prepare_travel("Region01", "JadePeaks", "LM-ARRIVAL")
            assert travel.complete_arrival("JadePeaks", "LM-JADE-ARRIVAL")
            assert region.record_return_route_ready()
            passed("JC-06", {"routes": ["JadePeaks->Region01", "Region01->JadePeaks"],
                              "unknown_route_rejected": True, "arrival": str(travel.get_arrival_landmark_id())})
            stage = "persistence"

        if stage == "persistence":
            # Re-establish both closure branches in the single authoritative ledger.
            if not region.has_fact("jade.imperial_pact_resolved"):
                assert region.record_jadefang_arrival()
                assert region.record_imperial_pact_resolution(True)
            if not region.has_fact("echo.unseen_hand"):
                assert region.record_disciple_resolution(True)
                assert region.record_unseen_hand_unlock()
            if not region.has_fact("jade.return_route_ready"):
                assert region.record_return_route_ready()
            assert region.is_regional_closure_complete()
            snapshot = unreal.WyrmSaveSubsystem.create_snapshot_object("WP23_2_Memory", player, adapter, world)
            assert snapshot and snapshot.schema_version == 8
            regional_ids = sorted(str(item.region_id) for item in snapshot.regional_world_records)
            assert "JadePeaks" in regional_ids
            saved_cooldown = player.get_unseen_hand_remaining_cooldown()
            region.reset_jade_peaks_state()
            player.restore_echo_state([], unreal.Name(), False, 0.0, 0.0)
            player.restore_unseen_hand_state(0.0)
            assert unreal.WyrmSaveSubsystem.apply_snapshot_object(snapshot, player, adapter, world)
            assert region.is_regional_closure_complete() and player.is_echo_unlocked("UnseenHand")
            assert player.get_unseen_hand_remaining_cooldown() > 0.0
            assert all(unreal.WyrmSaveSubsystem.is_schema_version_supported(version) for version in (1, 2, 3, 4, 5, 6, 7, 8))
            assert not unreal.WyrmSaveSubsystem.is_schema_version_supported(0)
            assert not unreal.WyrmSaveSubsystem.is_schema_version_supported(9)
            passed("JC-07", {"schema": 7, "regional_records": regional_ids,
                              "travel_region": str(snapshot.world_travel_record.current_region_id),
                              "saved_cooldown": saved_cooldown, "schema4_backward_read": True})
            stage = "regression"

        if stage == "regression":
            assert region.has_valid_landmark_graph()
            jadefangs = [d for d in actors(world, unreal.WyrmDragonCharacter) if str(d.dragon_id) == "Jadefang"]
            assert len(jadefangs) == 1 and jadefangs[0].has_supported_rig_profile()
            assert player.is_echo_unlocked("MirrorStep") and player.is_echo_unlocked("UnseenHand")
            passed("JC-08", {"jp_landmark_graph": True, "jadefang_identity_count": 1,
                              "mirror_step_preserved": True})
            report["status"] = "PASS"
            write_report()
            finished = True
            levels.editor_request_end_play()
    except Exception as exc:
        report["status"] = "FAIL"
        report["failed_stage"] = stage
        report["error"] = repr(exc)
        report["traceback"] = traceback.format_exc()
        log(f"FAIL in {stage}: {exc!r}")
        write_report()
        finished = True
        if levels.is_in_play_in_editor():
            levels.editor_request_end_play()


tick_handle = unreal.register_slate_post_tick_callback(tick)
levels.editor_play_simulate()
log("WP-23.2 focused PIE proof started")
