"""Prove the bounded Gloaming regional closure in live PIE."""
import json
import time
import traceback
from pathlib import Path

import unreal


ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
MAP = "/Game/WYRMFALL/World/Regions/L_GloamingMarches"
REPORT = ROOT / "Saved/Diagnostics/WP23_5_gloaming_completion_proof.json"
OUT = ROOT / "Saved/Diagnostics/WP23_5_GloamingCompletion"
levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
editor = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
editor_actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
if not editor.get_editor_world() or MAP not in editor.get_editor_world().get_path_name():
    assert levels.load_level(MAP), f"Could not load {MAP}"
if not any(isinstance(a, unreal.WyrmCharacter) for a in editor_actors.get_all_level_actors()):
    player = editor_actors.spawn_actor_from_class(
        unreal.WyrmCharacter, unreal.Vector(4300, 1200, 904), unreal.Rotator())
    player.set_actor_label("DIAG_WP23_5_GloamingCompletion_Player")
camera = editor_actors.spawn_actor_from_class(
    unreal.CameraActor, unreal.Vector(5350, 450, 1340), unreal.Rotator())
camera.set_actor_label("DIAG_WP23_5_GloamingCompletion_Camera")
OUT.mkdir(parents=True, exist_ok=True)

IDENTITIES = [
    unreal.WyrmRequiredHorrorIdentity.AIL_YEN,
    unreal.WyrmRequiredHorrorIdentity.BELLRAISER,
    unreal.WyrmRequiredHorrorIdentity.SAD_ECHO,
    unreal.WyrmRequiredHorrorIdentity.DREADATOR,
    unreal.WyrmRequiredHorrorIdentity.ROASTFACE,
    unreal.WyrmRequiredHorrorIdentity.GRAVY_DAUGHTERS,
    unreal.WyrmRequiredHorrorIdentity.KNIT,
    unreal.WyrmRequiredHorrorIdentity.CANNIBALL,
    unreal.WyrmRequiredHorrorIdentity.MUMS_THE_WYRD,
    unreal.WyrmRequiredHorrorIdentity.DREADY_FREDDIE,
    unreal.WyrmRequiredHorrorIdentity.PYRE_MIDHEAD,
]
report = {
    "kind": "wp23_5_gloaming_regional_completion",
    "engine": unreal.SystemLibrary.get_engine_version(),
    "map": MAP,
    "status": "RUNNING",
    "acceptance": {},
    "measurements": {},
    "not_claimed": ["interactive keyboard or gamepad walkthrough", "new packaged build"],
}
REPORT.parent.mkdir(parents=True, exist_ok=True)
REPORT.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
started = time.monotonic()
ticks = 0
nav_attempts = 0
wait_ticks = 0
stage = "wait"
finished = False
data = {}


def actors(world, cls):
    return list(unreal.GameplayStatics.get_all_actors_of_class(world, cls))


def record_all_prerequisites(region):
    assert region.record_arrival()
    assert region.resolve_ashgrave_extraction_seal()
    assert region.record_malvaine_resolution(True)
    assert region.record_hollow_twins_resolution(True)
    assert region.record_michael_mire_resolution()
    assert region.record_machete_mason_resolution()
    assert region.record_pleatherface_resolution()
    assert region.record_wherewolf_resolution()
    assert region.record_annie_wails_resolution()
    assert region.record_scarrie_resolution()
    assert region.record_chuckles_resolution()
    assert region.record_count_dripula_resolution()
    assert region.record_frank_n_shrine_resolution()
    for identity in IDENTITIES:
        assert region.record_required_horror_resolution(identity)


def tick(_delta):
    global ticks, nav_attempts, wait_ticks, stage, finished
    try:
        if finished:
            if not levels.is_in_play_in_editor():
                unreal.unregister_slate_post_tick_callback(handle)
                editor_actors.destroy_actor(camera)
                unreal.SystemLibrary.quit_editor()
            return
        if time.monotonic() - started > 300:
            raise RuntimeError(f"Timed out in {stage}")
        if wait_ticks:
            wait_ticks -= 1
            return
        if stage == "capture_wait":
            report["status"] = "PASS"
            REPORT.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
            finished = True
            levels.editor_request_end_play()
            return

        world = editor.get_game_world()
        if not world or not levels.is_in_play_in_editor():
            return
        ticks += 1
        terrains = actors(world, unreal.GeoForgeInfiniteTerrainActor)
        adapters = actors(world, unreal.WyrmGeoForgeAdapter)
        players = actors(world, unreal.WyrmCharacter)
        if len(terrains) != 1 or len(adapters) != 1 or len(players) != 1:
            return
        terrain = terrains[0]
        stats = terrain.get_runtime_render_stats()
        if (ticks < 90 or stats.get_editor_property("loaded_chunk_count") < 81 or
                stats.get_editor_property("queued_chunk_generation_count") or
                stats.get_editor_property("queued_chunk_rebuild_count") or
                stats.get_editor_property("pending_chunk_apply_count") or
                unreal.WyrmTerrainDiagnostics.is_navigation_build_pending(terrain)):
            return

        player = players[0]
        adapter = adapters[0]
        region = unreal.WyrmGloamingSubsystem.get_gloaming_subsystem(world)
        travel = unreal.WyrmWorldTravelSubsystem.get_world_travel_subsystem(world)
        by_label = {a.get_actor_label(): a for a in actors(world, unreal.Actor)}
        required = ("GLM_ROUTE_PYRE_MIDHEAD", "LM-GLOAMING-RETURN", "LM-GLOAMING-ARRIVAL",
                    "GLM_ENCOUNTER_PyreMidhead")
        if not region or not travel or not all(label in by_label for label in required):
            return

        pyre_anchor = by_label["GLM_ROUTE_PYRE_MIDHEAD"]
        return_anchor = by_label["LM-GLOAMING-RETURN"]
        pyre = by_label["GLM_ENCOUNTER_PyreMidhead"]
        projected = []
        for anchor in (pyre_anchor, return_anchor):
            result = unreal.WyrmTerrainDiagnostics.project_navigation_point(
                pyre, anchor.get_actor_location(), unreal.Vector(260, 260, 600))
            ok = bool(result[0]) if isinstance(result, (tuple, list)) else bool(result)
            point = next((v for v in result[1:] if isinstance(v, unreal.Vector)), anchor.get_actor_location()) \
                if isinstance(result, (tuple, list)) else anchor.get_actor_location()
            projected.append((ok, point))
        if not all(item[0] for item in projected):
            nav_attempts += 1
            if nav_attempts % 120 == 1:
                unreal.WyrmTerrainDiagnostics.refresh_navigation_data_for_actor(terrain)
            if nav_attempts < 600:
                return
            raise RuntimeError("Final encounter/return anchors did not project")
        path_points = int(unreal.WyrmTerrainDiagnostics.find_complete_navigation_path_point_count(
            pyre, projected[0][1], projected[1][1]))
        assert path_points > 0

        region.reset_gloaming_state()
        assert travel.complete_arrival("GloamingMarches", "LM-GLOAMING-ARRIVAL")
        assert not region.record_regional_completion(None, travel)
        record_all_prerequisites(region)
        optional_echoes_absent = (
            not region.has_fact("echo.sanguine_strike") and
            not region.has_fact("echo.second_turn") and
            not player.is_echo_unlocked("SanguineStrike") and
            not player.is_echo_unlocked("SecondTurn"))

        wrong_dragon = unreal.WyrmDragonCharacter.spawn_wyrm_dragon(
            world, unreal.WyrmDragonRole.HOSTILE_BOSS,
            unreal.Transform(location=pyre_anchor.get_actor_location() + unreal.Vector(250, 0, 120)))
        assert wrong_dragon
        wrong_dragon.set_dragon_id("Jadefang")
        assert wrong_dragon.perform_boss_defeat() and wrong_dragon.bond_with_humanoid(player)
        assert not region.record_regional_completion(wrong_dragon, travel)
        wrong_dragon.destroy_actor()

        nyxaroth = unreal.WyrmDragonCharacter.spawn_wyrm_dragon(
            world, unreal.WyrmDragonRole.HOSTILE_BOSS,
            unreal.Transform(location=pyre_anchor.get_actor_location() + unreal.Vector(250, 0, 120)))
        assert nyxaroth
        nyxaroth.set_dragon_id("Nyxaroth")
        assert nyxaroth.has_supported_rig_profile()
        assert nyxaroth.perform_boss_defeat() and nyxaroth.bond_with_humanoid(player)
        nyxaroth.set_dragon_form(unreal.WyrmDragonForm.COMPANION_FORM)
        assert region.can_complete_region(nyxaroth, travel)
        assert region.record_regional_completion(nyxaroth, travel)
        assert not region.record_regional_completion(nyxaroth, travel)

        snapshot = unreal.WyrmSaveSubsystem.create_snapshot_object(
            "WP23_5_GloamingCompletionMemory", player, adapter, world)
        assert snapshot and snapshot.schema_version == 8
        snapshot_facts = [str(x) for x in snapshot.gloaming_record.known_facts]
        snapshot_receipts = [str(x) for x in snapshot.gloaming_record.fact_receipts]
        snapshot_dragons = [str(x.dragon_id) for x in snapshot.dragon_records]
        assert "gloaming.region_complete" in snapshot_facts
        assert "gloaming.region.completion_committed" in snapshot_receipts
        assert snapshot_dragons.count("Nyxaroth") == 1

        region.reset_gloaming_state()
        nyxaroth.destroy_actor()
        assert unreal.WyrmSaveSubsystem.apply_snapshot_object(snapshot, player, adapter, world)
        restored_nyx = [d for d in actors(world, unreal.WyrmDragonCharacter)
                        if str(d.dragon_id) == "Nyxaroth"]
        assert len(restored_nyx) == 1 and restored_nyx[0].has_bond_receipt()
        assert region.has_fact("gloaming.region_complete")
        assert region.has_receipt("gloaming.region.completion_committed")
        assert not region.record_regional_completion(restored_nyx[0], travel)

        assert travel.is_allowed_route("GloamingMarches", "Region01")
        assert travel.prepare_travel("GloamingMarches", "Region01", "LM-GLOAMING-RETURN")
        assert travel.complete_arrival("Region01", "LM-ARRIVAL")
        assert travel.prepare_travel("Region01", "GloamingMarches", "LM-ARRIVAL")
        assert travel.complete_arrival("GloamingMarches", "LM-GLOAMING-ARRIVAL")

        report["acceptance"] = {
            "final_encounter_to_return_route_is_navigable": path_points > 0,
            "closure_rejects_missing_roster_or_dragon": True,
            "closure_requires_validated_bonded_nyxaroth": True,
            "optional_echoes_are_not_completion_tax": optional_echoes_absent,
            "regional_fact_and_receipt_commit_once": True,
            "schema7_restores_completion_and_one_bonded_nyxaroth": True,
            "region01_return_and_reentry_remain_allowlisted": True,
        }
        report["measurements"] = {
            "required_horror_count": 20,
            "final_to_return_path_points": path_points,
            "schema": snapshot.schema_version,
            "restored_nyxaroth_count": len(restored_nyx),
            "optional_echo_count": 0,
        }
        assert all(report["acceptance"].values())
        camera.set_actor_location(pyre_anchor.get_actor_location() + unreal.Vector(1050, -950, 620), False, False)
        camera.set_actor_rotation(unreal.MathLibrary.find_look_at_rotation(
            camera.get_actor_location(), pyre_anchor.get_actor_location() + unreal.Vector(0, 0, 120)), False)
        controller = unreal.GameplayStatics.get_player_controller(world, 0)
        if controller:
            controller.set_view_target_with_blend(camera, 0.0)
        screenshot = OUT / "01_gloaming_closure.png"
        requested = bool(unreal.AutomationLibrary.take_high_res_screenshot(1280, 720, str(screenshot), camera))
        assert requested
        report["screenshot"] = str(screenshot)
        stage = "capture_wait"
        wait_ticks = 90
    except Exception as exc:
        report["status"] = "FAIL"
        report["failed_stage"] = stage
        report["error"] = repr(exc)
        report["traceback"] = traceback.format_exc()
        REPORT.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
        unreal.log_error(f"WP23_5_GLOAMING_COMPLETION_FAIL {stage}: {exc!r}")
        finished = True
        if levels.is_in_play_in_editor():
            levels.editor_request_end_play()


handle = unreal.register_slate_post_tick_callback(tick)
levels.editor_play_simulate()
unreal.log("WP-23.5 Gloaming regional completion PIE proof started")
