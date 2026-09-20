"""Exercise Schema 8 Cogspire recovery and repeat-stable bonded Cogfang restoration in live PIE."""

import json
from pathlib import Path
import time
import traceback

import unreal


ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
MAP = "/Game/WYRMFALL/World/Regions/L_CogspireHarbor"
REPORT_PATH = ROOT / "Saved/Diagnostics/WP23_6_cogspire_save_proof.json"
OUT = ROOT / "Saved/Diagnostics/WP23_6_CogspireSave"
levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
editor = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
editor_actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
if not levels.load_level(MAP):
    raise RuntimeError("Could not load saved Cogspire Harbor map")
if not any(isinstance(actor, unreal.WyrmCharacter) for actor in editor_actors.get_all_level_actors()):
    player = editor_actors.spawn_actor_from_class(
        unreal.WyrmCharacter, unreal.Vector(-150, 650, 310), unreal.Rotator())
    player.set_actor_label("DIAG_WP23_6_CogspireSave_Player")
camera_origin = unreal.Vector(-1450, -450, 850)
camera_target = unreal.Vector(-100, 900, 320)
camera = editor_actors.spawn_actor_from_class(
    unreal.CameraActor, camera_origin,
    unreal.MathLibrary.find_look_at_rotation(camera_origin, camera_target))
camera.set_actor_label("DIAG_WP23_6_CogspireSave_Camera")
OUT.mkdir(parents=True, exist_ok=True)

report = {
    "kind": "wp23_6_cogspire_schema8_recovery",
    "engine": unreal.SystemLibrary.get_engine_version(),
    "map": MAP,
    "status": "RUNNING",
    "acceptance": {},
    "measurements": {},
    "not_claimed": [
        "Cogspire regional completion", "House Mark investigation", "Chef Aurelio investigation",
        "new Echo rewards", "interactive keyboard or gamepad walkthrough", "new packaged build",
    ],
}


def write_report():
    REPORT_PATH.parent.mkdir(parents=True, exist_ok=True)
    REPORT_PATH.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")


def actors(world, cls):
    return list(unreal.GameplayStatics.get_all_actors_of_class(world, cls))


def pump_signature(actor):
    component = actor.get_component_by_class(unreal.StaticMeshComponent)
    return {
        "location": list(actor.get_actor_location().to_tuple()),
        "scale": list(actor.get_actor_scale3d().to_tuple()),
        "collision": str(component.get_collision_enabled()),
        "mesh": component.get_editor_property("static_mesh").get_path_name(),
        "material": component.get_material(0).get_path_name(),
    }


started = time.monotonic()
ticks = 0
wait_ticks = 0
stage = "wait"
finished = False
state = {}


def tick(_delta):
    global ticks, wait_ticks, stage, finished
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
        if stage == "capture_wait":
            report["status"] = "PASS" if all(report["acceptance"].values()) else "FAIL"
            write_report()
            finished = True
            levels.editor_request_end_play()
            return

        world = editor.get_game_world()
        if not world or not levels.is_in_play_in_editor():
            return
        ticks += 1
        terrains = actors(world, unreal.GeoForgeInfiniteTerrainActor)
        adapters = actors(world, unreal.WyrmGeoForgeAdapter)
        if len(terrains) != 1 or len(adapters) != 1:
            return
        stats = terrains[0].get_runtime_render_stats()
        if (ticks < 90 or stats.get_editor_property("loaded_chunk_count") < 49 or
                stats.get_editor_property("queued_chunk_generation_count") or
                stats.get_editor_property("queued_chunk_rebuild_count") or
                stats.get_editor_property("pending_chunk_apply_count") or
                unreal.WyrmTerrainDiagnostics.is_navigation_build_pending(terrains[0])):
            return

        players = actors(world, unreal.WyrmCharacter)
        cogfangs = [actor for actor in actors(world, unreal.WyrmDragonCharacter)
                    if str(actor.dragon_id) == "Cogfang"]
        encounters = actors(world, unreal.WyrmCogspireCogfangEncounter)
        labels = {actor.get_actor_label(): actor for actor in actors(world, unreal.Actor)}
        required_pumps = ("COG_ENV_CivicWaterPumpA", "COG_ENV_CivicWaterPumpB")
        if len(players) != 1 or len(cogfangs) != 1 or len(encounters) != 1:
            return
        if not all(label in labels for label in required_pumps):
            raise RuntimeError("Missing one or both civic pump actors")
        player, cogfang, encounter = players[0], cogfangs[0], encounters[0]
        adapter = adapters[0]
        region = unreal.WyrmCogspireSubsystem.get_cogspire_subsystem(world)
        if not region:
            raise RuntimeError("Cogspire fact owner unavailable")

        if stage == "wait":
            state["pump_before"] = {label: pump_signature(labels[label]) for label in required_pumps}
            region.reset_cogspire_state()
            assert region.record_arrival()
            assert region.record_public_machinery_observed()
            assert region.record_coercion_diversion_observed()
            assert region.record_baron_acknowledgment()
            player.set_actor_location(encounter.get_actor_location(), False, False)
            assert encounter.begin_encounter(player)
            cogfang.get_attributes().set_current_armor(0.0)
            strikes = 0
            while cogfang.get_dragon_role() == unreal.WyrmDragonRole.HOSTILE_BOSS and strikes < 12:
                unreal.WyrmMeleeAttackAbility.apply_damage_effect(
                    player.get_ability_system(), cogfang.get_ability_system(), 250.0)
                strikes += 1
            assert cogfang.get_dragon_role() == unreal.WyrmDragonRole.DEFEATED_ALIVE
            player.set_actor_location(encounter.get_actor_location(), False, False)
            assert encounter.interact_shutdown_governor(player)
            player.set_actor_location(cogfang.get_actor_location(), False, False)
            assert encounter.interact_voluntary_bond(player)

            snapshot = unreal.WyrmSaveSubsystem.create_snapshot_object(
                "WP23_6_CogspireSchema8Memory", player, adapter, world)
            assert snapshot and snapshot.schema_version == 8
            facts = [str(item) for item in snapshot.cogspire_record.known_facts]
            receipts = [str(item) for item in snapshot.cogspire_record.fact_receipts]
            snapshot_cogfangs = [
                item for item in snapshot.dragon_records
                if str(item.dragon_id) == "Cogfang" and
                getattr(item, "has_bond_receipt", getattr(item, "b_has_bond_receipt", False))]
            assert "cogspire.coercion_governor_shutdown" in facts
            assert "cogspire.cogfang_bonded" in facts
            assert "cogspire.engine.coercion_governor_shutdown" in receipts
            assert "cogspire.cogfang.voluntary_bond" in receipts
            assert not snapshot.cogspire_record.coercion_governor_active
            assert snapshot.cogspire_record.civic_machinery_operational
            assert len(snapshot_cogfangs) == 1

            region.reset_cogspire_state()
            cogfang.destroy_actor()
            assert unreal.WyrmSaveSubsystem.apply_snapshot_object(snapshot, player, adapter, world)
            restored = [actor for actor in actors(world, unreal.WyrmDragonCharacter)
                        if str(actor.dragon_id) == "Cogfang"]
            assert len(restored) == 1 and restored[0].has_bond_receipt()
            assert restored[0].get_dragon_role() == unreal.WyrmDragonRole.ALLIED_COMPANION
            assert region.has_fact("cogspire.coercion_governor_shutdown")
            assert region.has_fact("cogspire.cogfang_bonded")
            assert not region.is_coercion_governor_active()
            assert region.is_civic_machinery_operational()

            assert unreal.WyrmSaveSubsystem.apply_snapshot_object(snapshot, player, adapter, world)
            restored_again = [actor for actor in actors(world, unreal.WyrmDragonCharacter)
                              if str(actor.dragon_id) == "Cogfang"]
            assert len(restored_again) == 1 and restored_again[0].has_bond_receipt()
            assert state["pump_before"] == {
                label: pump_signature(labels[label]) for label in required_pumps}
            assert all(unreal.WyrmSaveSubsystem.is_schema_version_supported(v) for v in range(1, 9))
            assert not unreal.WyrmSaveSubsystem.is_schema_version_supported(0)
            assert not unreal.WyrmSaveSubsystem.is_schema_version_supported(9)
            assert not region.has_fact("cogspire.region_complete")
            assert not region.has_fact("echo.deathmark")
            assert not region.has_fact("echo.carvers_precision")

            report["acceptance"] = {
                "schema8_captures_complete_cogspire_ledger": True,
                "selective_engine_state_restores": True,
                "exactly_one_bonded_cogfang_restores": True,
                "repeated_apply_does_not_duplicate_cogfang": True,
                "civic_pump_actors_remain_unchanged": True,
                "schemas1_through8_read_and_schema9_rejects": True,
                "optional_investigations_echoes_and_completion_absent": True,
            }
            report["measurements"] = {
                "schema": snapshot.schema_version,
                "fact_count": len(facts),
                "receipt_count": len(receipts),
                "snapshot_bonded_cogfang_count": len(snapshot_cogfangs),
                "restored_bonded_cogfang_count": len(restored_again),
                "gas_damage_applications": strikes,
                "coercion_governor_active": region.is_coercion_governor_active(),
                "civic_machinery_operational": region.is_civic_machinery_operational(),
            }
            controller = unreal.GameplayStatics.get_player_controller(world, 0)
            cameras = [actor for actor in actors(world, unreal.CameraActor)
                       if actor.get_actor_label() == "DIAG_WP23_6_CogspireSave_Camera"]
            if controller and cameras:
                controller.set_view_target_with_blend(cameras[0], 0.0)
            screenshot = OUT / "01_schema8_restored_cogfang.png"
            assert unreal.AutomationLibrary.take_high_res_screenshot(1280, 720, str(screenshot), cameras[0])
            report["screenshot"] = str(screenshot)
            write_report()
            stage = "capture_wait"
            wait_ticks = 90
    except Exception as exc:
        report["status"] = "FAIL"
        report["failed_stage"] = stage
        report["error"] = repr(exc)
        report["traceback"] = traceback.format_exc()
        write_report()
        unreal.log_error(f"WP23_6_COGSPIRE_SAVE_FAIL {stage}: {exc!r}")
        finished = True
        if levels.is_in_play_in_editor():
            levels.editor_request_end_play()


write_report()
handle = unreal.register_slate_post_tick_callback(tick)
levels.editor_play_simulate()
unreal.log("WP-23.6 Cogspire Schema 8 recovery PIE proof started")
