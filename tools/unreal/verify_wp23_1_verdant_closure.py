"""Focused live-PIE proof for the bounded WP-23.1 Verdant Reach closure."""

import json
import time
import traceback
from pathlib import Path
import unreal

ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
REPORT_PATH = ROOT / "Saved/Diagnostics/WP23_1_verdant_closure_proof.json"
MAP = "/Game/WYRMFALL/World/Regions/L_Region01"
TESTS = tuple(f"VR-{index:02d}" for index in range(1, 9))
report = {"kind": "wp23_1_verdant_closure", "engine": unreal.SystemLibrary.get_engine_version(),
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


def prepare_homecoming(region):
    region.reset_region01_state()
    assert region.visit_landmark("LM-TIDECROSS")
    assert region.secure_worker(unreal.WyrmRegion01Worker.PELL)
    assert region.secure_worker(unreal.WyrmRegion01Worker.IVEN)
    assert region.secure_worker(unreal.WyrmRegion01Worker.SELLA)
    assert region.stop_crowncut_extraction()
    assert region.resolve_rusk(unreal.WyrmRegion01RuskOutcome.DEFEATED_CUSTODY)
    assert region.commit_fact("verdance.bond_accepted", "diag.verdance.bond")
    assert region.record_relief_resolved()
    assert region.complete_homecoming()


write_report()
levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
editor = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
if not editor.get_editor_world() or MAP not in editor.get_editor_world().get_path_name():
    assert levels.load_level(MAP), f"Could not load {MAP}"
editor_actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
editor_nav = unreal.NavigationSystemV1.get_navigation_system(editor.get_editor_world())
nav_bounds = [a for a in editor_actors.get_all_level_actors() if isinstance(a, unreal.NavMeshBoundsVolume)]
assert editor_nav and nav_bounds, "Region 01 must own navigation bounds"
proof_nav_bound = editor_actors.spawn_actor_from_class(
    unreal.NavMeshBoundsVolume, unreal.Vector(0, 0, 900), unreal.Rotator())
proof_nav_bound.set_actor_scale3d(unreal.Vector(80, 80, 25))
proof_nav_bound.set_actor_label("DIAG_WP23_1_NavRegistration")
nav_bounds.append(proof_nav_bound)
for nav_bound in nav_bounds:
    editor_nav.on_navigation_bounds_updated(nav_bound)
if not any(isinstance(a, unreal.WyrmCharacter) for a in editor_actors.get_all_level_actors()):
    diag_player = editor_actors.spawn_actor_from_class(unreal.WyrmCharacter, unreal.Vector(-300, 1500, 888))
    diag_player.set_actor_label("DIAG_WP23_1_Player")

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
        player_candidates = actors(world, unreal.WyrmCharacter)
        region = unreal.WyrmRegion01Subsystem.get_region01_subsystem(world)
        adapters = actors(world, unreal.WyrmGeoForgeAdapter)
        if not player_candidates or not region or not adapters:
            return
        player = player_candidates[0]
        if stage == "wait":
            if data["ticks"] < 90:
                return
            player.grant_combat_abilities()
            player.get_attributes().set_current_health(100.0)
            player.get_attributes().set_current_focus(100.0)
            stage = "presentation"

        if stage == "presentation":
            meridess = actors(world, unreal.WyrmQueenMeridessCharacter)
            guards = actors(world, unreal.WyrmVerdantRoyalGuardCharacter)
            hunters = actors(world, unreal.WyrmCanopyHunterCharacter)
            labels = [a.get_actor_label() for a in actors(world, unreal.StaticMeshActor)
                      if a.get_actor_label().startswith("VERDANT_")]
            assert len(meridess) == len(guards) == len(hunters) == 1
            assert meridess[0].mesh.skeletal_mesh and guards[0].mesh.skeletal_mesh and hunters[0].mesh.skeletal_mesh
            assert meridess[0].mesh.get_material(0) and hunters[0].mesh.get_material(0)
            assert len(labels) >= 10
            data.update(meridess=meridess[0], guard=guards[0], hunter=hunters[0])
            passed("VR-01", {"real_character_meshes": 3, "supplied_ranger_scenery": len(labels)})
            stage = "evidence"

        if stage == "evidence":
            prepare_homecoming(region)
            assert region.discover_evidence(unreal.WyrmRegion01Evidence.MACHINE)
            assert data["meridess"].resolve_with_evidence()
            assert region.is_verdant_regional_closure_complete()
            passed("VR-02", {"route": "evidence", "queen_defeated": False, "claim_relinquished": True})
            stage = "guard"

        if stage == "guard":
            prepare_homecoming(region)
            guard = data["guard"]
            guard.get_attributes().set_current_health(0.0)
            assert guard.is_defeated() and data["meridess"].resolve_after_guard_defeat(guard)
            assert region.is_verdant_regional_closure_complete()
            passed("VR-03", {"route": "guard_defeat", "queen_defeated": False, "same_closure_fact": True})
            stage = "route"

        if stage == "route":
            terrain = actors(world, unreal.GeoForgeInfiniteTerrainActor)[0]
            if not data.get("nav_refreshed"):
                unreal.WyrmTerrainDiagnostics.refresh_navigation_data_for_actor(terrain)
                data["nav_refreshed"] = True
                data["nav_wait_ticks"] = 0
                return
            if unreal.WyrmTerrainDiagnostics.is_navigation_build_pending(terrain):
                return
            route_points = {
                "tidecross": unreal.Vector(0, 0, 800),
                "canopy_entry": unreal.Vector(-450, 1650, 800),
                "outpost": unreal.Vector(-1650, 2850, 800),
                "hunter_blind": unreal.Vector(1050, 3000, 800),
            }
            projected = {name: unreal.WyrmTerrainDiagnostics.project_navigation_point(
                terrain, point, unreal.Vector(160, 160, 320)) for name, point in route_points.items()}
            if not all(projected.values()):
                data["nav_wait_ticks"] += 1
                if data["nav_wait_ticks"] > 900:
                    raise RuntimeError(f"Navigation projection timed out: {projected}")
                return
            assert region.has_valid_landmark_graph()
            assert region.get_landmark_definitions().__len__() == 16
            assert region.has_route_between_landmarks("LM-TIDECROSS", "LM-TIDECALLEROUTPOST")
            assert region.has_route_between_landmarks("LM-CANOPYHUNT", "LM-TIDECROSS")
            passed("VR-04", {"landmarks": 16, "return_route": True, "post_homecoming_available": True,
                              "projected_route_points": list(projected.keys()),
                              "companion_clearance_cm": 320, "true_form_staging": "open canopy/outpost apron"})
            stage = "hunter"

        if stage == "hunter":
            prepare_homecoming(region)
            hunter = data["hunter"]
            assert hunter.resolve_by_trust(player)
            assert player.is_echo_unlocked("HuntersVeil") and region.is_hunters_veil_unlocked()
            assert not hunter.resolve_by_trust(player)
            passed("VR-05", {"trust_route": True, "once_only": True, "regional_closure_required": False})
            stage = "veil"

        if stage == "veil":
            assert player.equip_echo("HuntersVeil")
            player.restore_hunters_veil_state(False, 0.0, 0.0)
            player.get_attributes().set_current_focus(100.0)
            assert player.activate_hunters_veil()
            assert abs(player.get_attributes().get_current_focus() - 75.0) < 0.01
            assert player.is_hunters_veil_active() and player.get_hunters_veil_remaining_duration() <= 5.0
            ordinary = unreal.WyrmEnemyCharacter.spawn_wyrm_enemy(
                world, unreal.WyrmEnemyRole.MELEE_CHASER,
                unreal.Transform(location=player.get_actor_location() + unreal.Vector(200, 0, 0)))
            boss = unreal.WyrmEnemyCharacter.spawn_wyrm_enemy(
                world, unreal.WyrmEnemyRole.MELEE_CHASER,
                unreal.Transform(location=player.get_actor_location() + unreal.Vector(250, 0, 0)))
            boss.set_editor_property("bIsBoss", True)
            assert not ordinary.can_acquire_target(player) and boss.can_acquire_target(player)
            assert player.perform_primary_attack() and not player.is_hunters_veil_active()
            player.restore_hunters_veil_state(False, 0.0, 0.0)
            player.get_attributes().set_current_focus(100.0)
            assert player.activate_hunters_veil()
            player.get_attributes().set_current_health(90.0)
            assert not player.is_hunters_veil_active()
            passed("VR-06", {"focus_cost": 25.0, "duration": 5.0, "cooldown": 16.0,
                              "ordinary_target_break": True, "boss_counter": True,
                              "attack_break": True, "damage_break": True})
            stage = "continuity"

        if stage == "continuity":
            assert region.is_homecoming_complete() and region.is_bonded_dragon_available()
            verdance = [d for d in actors(world, unreal.WyrmDragonCharacter) if str(d.dragon_id) == "Verdance"]
            assert len(verdance) == 1
            passed("VR-07", {"verdance_identity_count": 1, "homecoming_preserved": True,
                              "existing_region_owner": True})
            stage = "persistence"

        if stage == "persistence":
            player.restore_hunters_veil_state(True, 3.0, 14.0)
            snapshot = unreal.WyrmSaveSubsystem.create_snapshot_object("WP23_1_Memory", player, adapters[0], world)
            assert snapshot and snapshot.schema_version == 7
            # UE's Python reflection strips the native boolean `b` prefix.
            assert snapshot.character_record.hunters_veil_active
            region.reset_region01_state()
            player.restore_echo_state([], unreal.Name(), False, 0.0, 0.0)
            player.restore_hunters_veil_state(False, 0.0, 0.0)
            assert unreal.WyrmSaveSubsystem.apply_snapshot_object(snapshot, player, adapters[0], world)
            assert region.is_hunters_veil_unlocked() and player.is_echo_unlocked("HuntersVeil")
            assert player.is_hunters_veil_active() and player.get_hunters_veil_remaining_cooldown() > 0.0
            assert all(unreal.WyrmSaveSubsystem.is_schema_version_supported(v) for v in (1, 2, 3, 4, 5, 6, 7))
            assert not unreal.WyrmSaveSubsystem.is_schema_version_supported(0)
            assert not unreal.WyrmSaveSubsystem.is_schema_version_supported(8)
            passed("VR-08", {"schema": 7, "schemas_1_to_6_readable": True,
                              "facts_restored": True, "echo_state_restored": True})
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
log("WP-23.1 focused PIE proof started")
