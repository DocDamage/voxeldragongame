"""Focused PIE probe for WP-12's Region 01 fact ledger and unified save path.

This creates a deliberately diagnostic blank map. It does not claim that real
Tidecross, Crowncut, cave, cart, or worker art has been placed in production.
"""
import json
from pathlib import Path
import time
import traceback
import unreal


ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
REPORT_PATH = ROOT / "Saved/Diagnostics/WP12_region01_proof.json"
report = {
    "kind": "wp12_region01_fact_ledger_pie",
    "engine": unreal.SystemLibrary.get_engine_version(),
    "fixture_scope": "diagnostic blank map with real player and Green Dragon runtime actors",
    "production_landmark_placement": "BLOCKED: no audited quarry, settlement, cave, cart, or distinct worker assets are imported",
    "status": "INITIALIZING",
    "tests": {
        "REG-01.LogicalLandmarkGraph": {"status": "NOT_RUN"},
        "REG-02.IndependentWorkerReceipts": {"status": "NOT_RUN"},
        "REG-03.SellaFirst": {"status": "NOT_RUN"},
        "REG-04.RedundantEvidence": {"status": "NOT_RUN"},
        "REG-05.PostBondRuskCustody": {"status": "NOT_RUN"},
        "REG-09.LateRescueAndImmediateDragon": {"status": "NOT_RUN"},
        "REG-10.UnifiedFactSaveRestore": {"status": "NOT_RUN"},
        "REG-11.NoActivityFactGate": {"status": "NOT_RUN"},
    },
    "details": {},
    "logs": [],
}


def log(message):
    line = f"[{time.strftime('%H:%M:%S')}] {message}"
    print(line)
    report["logs"].append(line)


def write_report():
    REPORT_PATH.parent.mkdir(parents=True, exist_ok=True)
    REPORT_PATH.write_text(json.dumps(report, indent=2, default=str) + "\n", encoding="utf-8")


def finish_test(name, passed, details):
    report["tests"][name]["status"] = "PASS" if passed else "FAIL"
    report["details"][name] = details
    log(f"Test {name}: {'PASS' if passed else 'FAIL'}")
    if not passed:
        raise RuntimeError(f"{name} failed: {details}")


subsystem_lookup_logged = False


def get_region_subsystem(world):
    global subsystem_lookup_logged
    gi = unreal.GameplayStatics.get_game_instance(world)
    if not subsystem_lookup_logged:
        log(
            f"Region subsystem lookup: game_instance={gi!r}; "
            f"has_get_subsystem={hasattr(gi, 'get_subsystem') if gi else False}; "
            f"has_blueprint_library={hasattr(unreal, 'SubsystemBlueprintLibrary')}")
        subsystem_lookup_logged = True
    return unreal.WyrmRegion01Subsystem.get_region01_subsystem(world)


write_report()

# Explicit diagnostic fixture only. It is not a stand-in for Region 01 art.
unreal.EditorLoadingAndSavingUtils.new_blank_map(False)
editor_actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
cube_mesh = unreal.load_asset("/Engine/BasicShapes/Cube.Cube")
floor_actor = editor_actor_subsystem.spawn_actor_from_class(
    unreal.StaticMeshActor, unreal.Vector(0.0, 0.0, -10.0))
if floor_actor and cube_mesh:
    floor_mesh = floor_actor.get_component_by_class(unreal.StaticMeshComponent)
    if floor_mesh:
        floor_mesh.set_static_mesh(cube_mesh)
        floor_mesh.set_collision_profile_name("BlockAll")
    floor_actor.set_actor_scale3d(unreal.Vector(100.0, 100.0, 0.2))

level_editor = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
started_at = time.monotonic()
finished = False
player_ref = None
dragon_ref = None
subsystem_wait_logged = False


def pie_tick(_delta_seconds):
    global finished, player_ref, dragon_ref, subsystem_wait_logged
    try:
        if finished:
            if not level_editor.is_in_play_in_editor():
                unreal.unregister_slate_post_tick_callback(tick_handle)
                log("PIE closed cleanly; exiting editor.")
                unreal.SystemLibrary.quit_editor()
            return

        if time.monotonic() - started_at > 90:
            raise RuntimeError("Timed out waiting for the Region 01 PIE fixture")

        world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
        if not world or not level_editor.is_in_play_in_editor():
            return

        pc = unreal.GameplayStatics.get_player_controller(world, 0)
        wyrm_pc = unreal.WyrmPlayerController.cast(pc) if pc else None
        player = wyrm_pc.get_controlled_pawn() if wyrm_pc else None
        player = unreal.WyrmCharacter.cast(player) if player else None
        if not player:
            return

        region = get_region_subsystem(world)
        if not region:
            if not subsystem_wait_logged:
                log("Waiting for the Region 01 game-instance subsystem to initialize in PIE.")
                subsystem_wait_logged = True
            if time.monotonic() - started_at < 15:
                return
            raise RuntimeError("PIE did not provide the Region 01 game-instance subsystem")
        region.reset_region01_state()
        player.set_actor_location(unreal.Vector(0.0, 0.0, 100.0), False, False)

        # REG-01 and REG-11: the graph is logical and needs no activities.
        graph_ok = region.has_valid_landmark_graph()
        heart_to_town = region.has_route_between_landmarks("LM-HEART", "LM-TIDECROSS")
        town_to_arena = region.has_route_between_landmarks("LM-TIDECROSS", "LM-ARENA")
        silent_landing_route = region.has_route_between_landmarks(
            "LM-TIDECROSS", "LM-SILENTLANDING")
        silent_landing_locked = not region.is_landmark_currently_available("LM-SILENTLANDING")
        compact_cave_locked = not region.is_landmark_currently_available("LM-COMPACTCAVE")
        finish_test("REG-01.LogicalLandmarkGraph", (
            graph_ok and heart_to_town and town_to_arena and silent_landing_route
            and silent_landing_locked and compact_cave_locked), {
            "landmark_count": len(region.get_landmark_definitions()),
            "heart_to_tidecross": heart_to_town,
            "tidecross_to_arena": town_to_arena,
            "silent_landing_optional_route": silent_landing_route,
            "silent_landing_locked_before_homecoming": silent_landing_locked,
            "compact_cave_locked_before_bond": compact_cave_locked,
            "production_map_placed": False,
        })

        no_activity_facts_before = (
            not region.has_fact("activity.fishing.complete")
            and not region.has_fact("activity.cooking.complete")
            and not region.has_fact("activity.camp.complete"))
        exit_recorded = region.record_heart_exit_reached()
        town_visited = region.visit_landmark("LM-TIDECROSS")
        no_activity_gate = region.has_route_between_landmarks("LM-TIDECROSS", "LM-ARENA")
        finish_test("REG-11.NoActivityFactGate", (
            no_activity_facts_before and exit_recorded and town_visited and no_activity_gate), {
            "no_fishing_cooking_camp_facts": no_activity_facts_before,
            "heart_exit_recorded": exit_recorded,
            "town_visit_recorded": town_visited,
            "onward_quarry_route_available": no_activity_gate,
        })

        # REG-03 / REG-04: Sella first makes her account and auxiliary option
        # available; machine + witness keeps the conflict clear without records.
        sella_first = region.secure_worker(unreal.WyrmRegion01Worker.SELLA)
        sella_account = region.has_fact("evidence.sella_account")
        auxiliary_available = region.is_auxiliary_shutdown_available()
        auxiliary_disabled = region.disable_auxiliary_restraint()
        machine_seen = region.discover_evidence(unreal.WyrmRegion01Evidence.MACHINE)
        readable_without_records = region.has_conflict_evidence()
        notice_later = region.read_crown_notice()
        duplicate_sella_rejected = not region.secure_worker(unreal.WyrmRegion01Worker.SELLA)
        finish_test("REG-03.SellaFirst", (
            sella_first and sella_account and auxiliary_available and auxiliary_disabled
            and notice_later and duplicate_sella_rejected), {
            "sella_secured_before_notice": sella_first,
            "sella_account_available": sella_account,
            "auxiliary_available": auxiliary_available,
            "auxiliary_disabled": auxiliary_disabled,
            "notice_read_later": notice_later,
            "duplicate_receipt_rejected": duplicate_sella_rejected,
        })
        finish_test("REG-04.RedundantEvidence", machine_seen and readable_without_records, {
            "machine_seen": machine_seen,
            "sella_account": sella_account,
            "records_deliberately_missing": not region.has_fact("evidence.records"),
            "conflict_remains_readable": readable_without_records,
        })

        # REG-05 / REG-09: existing dragon authority issues the bond first;
        # fact layer then enables the late Rusk custody branch, with no worker gate.
        defeated_alive = region.record_verdance_defeated_alive()
        claim_broken = region.break_verdance_claim()
        spawn_tf = unreal.Transform(
            location=unreal.Vector(500.0, 0.0, 100.0),
            rotation=unreal.Rotator(0.0, 0.0, 0.0),
            scale=unreal.Vector(1.0, 1.0, 1.0))
        dragon = unreal.WyrmDragonCharacter.spawn_wyrm_dragon(
            world, unreal.WyrmDragonRole.HOSTILE_BOSS, spawn_tf)
        if not dragon:
            raise RuntimeError("Failed to spawn the real Green Dragon runtime actor")
        dragon_ref = dragon
        dragon_defeated = dragon.perform_boss_defeat()
        dragon_bonded = dragon.bond_with_humanoid(player)
        fact_bonded = region.record_verdance_bond_accepted(dragon)
        immediate_dragon = region.is_bonded_dragon_available() and dragon.has_bond_receipt()
        compact_cave_available = region.is_landmark_currently_available("LM-COMPACTCAVE")
        partial_debrief = region.is_partial_debrief_available()
        direct_control_started = wyrm_pc.transfer_control_to_dragon(dragon)
        direct_control_returned = wyrm_pc.return_control_to_humanoid() if direct_control_started else False
        finish_test("REG-09.LateRescueAndImmediateDragon", (
            defeated_alive and claim_broken and dragon_defeated and dragon_bonded
            and fact_bonded and immediate_dragon and partial_debrief
            and direct_control_started and direct_control_returned
            and not region.is_homecoming_complete()), {
            "living_defeat_recorded": defeated_alive,
            "claim_broken": claim_broken,
            "dragon_bond_receipt": dragon_bonded,
            "region_bond_receipt": fact_bonded,
            "compact_cave_available_after_bond": compact_cave_available,
            "direct_control_started": direct_control_started,
            "direct_control_returned": direct_control_returned,
            "partial_debrief_before_remaining_workers": partial_debrief,
            "homecoming_before_remaining_workers": region.is_homecoming_complete(),
        })

        rusk_custody = region.resolve_rusk(
            unreal.WyrmRegion01RuskOutcome.SURRENDERED_CUSTODY)
        extraction_stopped_by_claim = region.has_fact("quarry.extraction_stopped")
        finish_test("REG-05.PostBondRuskCustody", rusk_custody and extraction_stopped_by_claim, {
            "rusk_started_unresolved": True,
            "post_bond_surrendered_custody": rusk_custody,
            "extraction_stopped_by_primary_claim_break": extraction_stopped_by_claim,
            "rusk_outcome": str(region.get_rusk_outcome()),
        })

        pell_secured = region.secure_worker(unreal.WyrmRegion01Worker.PELL)
        iven_secured = region.secure_worker(unreal.WyrmRegion01Worker.IVEN)
        wage_once = region.recover_optional_wage_record()
        wage_repeat_rejected = not region.recover_optional_wage_record()
        workers_independent = (
            region.has_fact("worker.pell.secured")
            and region.has_fact("worker.iven.secured")
            and region.has_fact("worker.sella.secured"))
        homecoming_blocked_without_relief = not region.is_homecoming_ready()
        # This fact-ledger probe models the future WP-14 relief completion;
        # it does not represent a production relief encounter.
        relief_recorded = region.record_relief_resolved()
        homecoming_ready = region.is_homecoming_ready()
        homecoming_completed = region.complete_homecoming()
        silent_landing_available = region.is_landmark_currently_available("LM-SILENTLANDING")
        finish_test("REG-02.IndependentWorkerReceipts", (
            pell_secured and iven_secured and workers_independent
            and homecoming_blocked_without_relief and relief_recorded
            and homecoming_ready and homecoming_completed and silent_landing_available
            and region.is_homecoming_complete()), {
            "pell_secured_after_bond": pell_secured,
            "iven_secured_after_bond": iven_secured,
            "three_individual_facts": workers_independent,
            "homecoming_blocked_without_relief": homecoming_blocked_without_relief,
            "relief_fact_modeled_for_predicate": relief_recorded,
            "production_relief_encounter": False,
            "homecoming_ready_after_full_local_closure": homecoming_ready,
            "homecoming_completed_at_tidecross": homecoming_completed,
            "silent_landing_available_after_homecoming": silent_landing_available,
            "wage_record_once": wage_once,
            "wage_record_repeat_rejected": wage_repeat_rejected,
        })

        # REG-10: use the real GameInstance subsystem path through the single
        # save owner, then prove facts/consequences return after reset.
        snapshot = unreal.WyrmSaveSubsystem.create_snapshot_object(
            "WP12_Region01_PIE", player, None, world)
        if not snapshot:
            raise RuntimeError("Failed to create unified Region 01 snapshot")
        saved_record = getattr(snapshot, "region01_record", None)
        if saved_record is None:
            saved_record = snapshot.get_editor_property("region01_record")
        saved_homecoming = any(str(fact) == "homecoming.complete" for fact in saved_record.known_facts)
        saved_extraction = any(str(fact) == "quarry.extraction_stopped" for fact in saved_record.known_facts)
        saved_wage = any(str(fact) == "wage.recovered" for fact in saved_record.known_facts)
        region.reset_region01_state()
        reset_cleared = not region.is_homecoming_complete()
        restored = unreal.WyrmSaveSubsystem.apply_snapshot_object(snapshot, player, None, world)
        restore_facts = (
            region.is_homecoming_complete()
            and region.has_fact("quarry.extraction_stopped")
            and region.has_fact("wage.recovered")
            and region.get_rusk_outcome() == unreal.WyrmRegion01RuskOutcome.SURRENDERED_CUSTODY)
        finish_test("REG-10.UnifiedFactSaveRestore", (
            saved_homecoming and saved_extraction and saved_wage and reset_cleared and restored and restore_facts), {
            "saved_homecoming": saved_homecoming,
            "saved_extraction_stop": saved_extraction,
            "saved_wage_recovery": saved_wage,
            "reset_cleared": reset_cleared,
            "unified_restore_succeeded": restored,
            "facts_restored": restore_facts,
        })

        report["status"] = "PASS"
        write_report()
        log("All WP-12 Region 01 fact-ledger PIE checks passed.")
        if dragon_ref:
            dragon_ref.destroy_actor()
        finished = True
        level_editor.editor_request_end_play()

    except Exception as exc:
        report["status"] = "FAIL"
        report["error"] = str(exc)
        report["traceback"] = traceback.format_exc()
        log(f"PIE exception: {exc}")
        write_report()
        if dragon_ref:
            dragon_ref.destroy_actor()
        finished = True
        level_editor.editor_request_end_play()


tick_handle = unreal.register_slate_post_tick_callback(pie_tick)
level_editor.editor_request_begin_play()
log("PIE session started for WP-12 Region 01 fact-ledger verification.")
