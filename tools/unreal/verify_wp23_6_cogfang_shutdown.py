"""Exercise captive Cogfang combat, selective governor shutdown, and voluntary bond in live PIE."""

import json
from pathlib import Path
import time
import traceback

import unreal


ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
MAP = "/Game/WYRMFALL/World/Regions/L_CogspireHarbor"
REPORT_PATH = ROOT / "Saved/Diagnostics/WP23_6_cogfang_shutdown_proof.json"
OUT = ROOT / "Saved/Diagnostics/WP23_6_CogfangShutdown"
levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
editor = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
editor_actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
if not levels.load_level(MAP):
    raise RuntimeError("Could not load saved Cogspire Harbor map")
if not any(isinstance(actor, unreal.WyrmCharacter) for actor in editor_actors.get_all_level_actors()):
    player = editor_actors.spawn_actor_from_class(
        unreal.WyrmCharacter, unreal.Vector(-150, 650, 310), unreal.Rotator())
    player.set_actor_label("DIAG_WP23_6_CogfangShutdown_Player")
camera_origin = unreal.Vector(-1450, -450, 850)
camera_target = unreal.Vector(-100, 900, 320)
camera = editor_actors.spawn_actor_from_class(
    unreal.CameraActor, camera_origin,
    unreal.MathLibrary.find_look_at_rotation(camera_origin, camera_target))
camera.set_actor_label("DIAG_WP23_6_CogfangShutdown_Camera")
OUT.mkdir(parents=True, exist_ok=True)

report = {
    "kind": "wp23_6_captive_cogfang_selective_shutdown",
    "engine": unreal.SystemLibrary.get_engine_version(),
    "map": MAP,
    "status": "RUNNING",
    "acceptance": {},
    "screenshots": [],
    "not_claimed": [
        "Cogspire regional completion", "House Mark investigation", "Chef Aurelio investigation",
        "new Echo rewards", "save persistence or schema extension",
        "interactive keyboard or gamepad walkthrough", "new packaged build",
    ],
}
REPORT_PATH.parent.mkdir(parents=True, exist_ok=True)


def write_report():
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
capture_camera = None
state = {}


def request_capture(name):
    path = OUT / name
    requested = bool(unreal.AutomationLibrary.take_high_res_screenshot(
        1280, 720, str(path), capture_camera))
    report["screenshots"].append({"path": str(path), "requested": requested})
    write_report()


def tick(_delta):
    global ticks, wait_ticks, stage, finished, capture_camera
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
        if stage == "capture_before":
            stage = "capture_before_wait"
            request_capture("01_captive_cogfang_and_governor.png")
            wait_ticks = 75
            return
        if stage == "capture_before_wait":
            stage = "exercise"
        if stage == "capture_after":
            stage = "capture_after_wait"
            request_capture("02_freed_cogfang_civic_pumps_intact.png")
            wait_ticks = 75
            return
        if stage == "capture_after_wait":
            report["status"] = "PASS" if all(report["acceptance"].values()) and all(
                row["requested"] for row in report["screenshots"]) else "FAIL"
            write_report()
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
        stats = terrains[0].get_runtime_render_stats()
        if (ticks < 90 or stats.get_editor_property("loaded_chunk_count") < 49 or
                stats.get_editor_property("queued_chunk_generation_count") or
                stats.get_editor_property("queued_chunk_rebuild_count") or
                stats.get_editor_property("pending_chunk_apply_count") or
                unreal.WyrmTerrainDiagnostics.is_navigation_build_pending(terrains[0])):
            return

        players = actors(world, unreal.WyrmCharacter)
        dragons = [actor for actor in actors(world, unreal.WyrmDragonCharacter)
                   if str(actor.dragon_id) == "Cogfang"]
        encounters = actors(world, unreal.WyrmCogspireCogfangEncounter)
        cameras = [actor for actor in actors(world, unreal.CameraActor)
                   if actor.get_actor_label() == "DIAG_WP23_6_CogfangShutdown_Camera"]
        labels = {actor.get_actor_label(): actor for actor in actors(world, unreal.Actor)}
        required_pumps = ("COG_ENV_CivicWaterPumpA", "COG_ENV_CivicWaterPumpB")
        if len(players) != 1 or len(dragons) != 1 or len(encounters) != 1 or len(cameras) != 1:
            return
        if not all(label in labels for label in required_pumps):
            raise RuntimeError("Missing one or both civic pump actors")
        player, cogfang, encounter = players[0], dragons[0], encounters[0]
        region = unreal.WyrmCogspireSubsystem.get_cogspire_subsystem(world)
        if not region:
            raise RuntimeError("Cogspire fact owner unavailable")

        if stage == "wait":
            region.reset_cogspire_state()
            player.set_actor_location(encounter.get_actor_location(), False, False)
            before_evidence_rejected = not encounter.begin_encounter(player)
            prereqs = (region.record_arrival() and region.record_public_machinery_observed() and
                       region.record_coercion_diversion_observed() and
                       region.record_baron_acknowledgment())
            state["before_evidence_rejected"] = before_evidence_rejected
            state["prereqs"] = prereqs
            state["pump_before"] = {label: pump_signature(labels[label]) for label in required_pumps}
            capture_camera = cameras[0]
            capture_camera.set_actor_location(camera_origin, False, False)
            capture_camera.set_actor_rotation(
                unreal.MathLibrary.find_look_at_rotation(camera_origin, camera_target), False)
            controller = unreal.GameplayStatics.get_player_controller(world, 0)
            if controller:
                controller.set_view_target_with_blend(capture_camera, 0.0)
            stage = "capture_before"
            wait_ticks = 30
            return

        if stage == "exercise":
            player.set_actor_location(encounter.get_actor_location(), False, False)
            began = encounter.begin_encounter(player)
            duplicate_begin_rejected = not encounter.begin_encounter(player)
            shutdown_before_defeat_rejected = not encounter.interact_shutdown_governor(player)
            initial_health = cogfang.get_attributes().get_current_health()
            cogfang.get_attributes().set_current_armor(0.0)
            strikes = 0
            while cogfang.get_dragon_role() == unreal.WyrmDragonRole.HOSTILE_BOSS and strikes < 12:
                unreal.WyrmMeleeAttackAbility.apply_damage_effect(
                    player.get_ability_system(), cogfang.get_ability_system(), 250.0)
                strikes += 1
            defeated = (cogfang.get_dragon_role() == unreal.WyrmDragonRole.DEFEATED_ALIVE and
                        region.has_fact(unreal.Name("cogspire.cogfang_defeated_alive")))
            defeat_health = cogfang.get_attributes().get_current_health()

            player.set_actor_location(cogfang.get_actor_location(), False, False)
            bond_before_shutdown_rejected = not encounter.interact_voluntary_bond(player)
            player.set_actor_location(encounter.get_actor_location(), False, False)
            shutdown = encounter.interact_shutdown_governor(player)
            shutdown_duplicate_rejected = not encounter.interact_shutdown_governor(player)
            pump_after = {label: pump_signature(labels[label]) for label in required_pumps}
            pumps_unchanged = state["pump_before"] == pump_after
            selective_state = (not region.is_coercion_governor_active() and
                               region.is_civic_machinery_operational())

            player.set_actor_location(cogfang.get_actor_location(), False, False)
            bonded = encounter.interact_voluntary_bond(player)
            bond_duplicate_rejected = not encounter.interact_voluntary_bond(player)
            identity = (str(cogfang.dragon_id) == "Cogfang" and cogfang.has_supported_rig_profile() and
                        len(cogfang.follower_mesh_components) == 34 and cogfang.has_bond_receipt() and
                        cogfang.get_dragon_role() == unreal.WyrmDragonRole.ALLIED_COMPANION)
            bounded = (not region.has_fact(unreal.Name("cogspire.region_complete")) and
                       not region.has_fact(unreal.Name("echo.deathmark")) and
                       not region.has_fact(unreal.Name("echo.carvers_precision")) and
                       unreal.WyrmSaveSubsystem.is_schema_version_supported(8) and
                       not unreal.WyrmSaveSubsystem.is_schema_version_supported(9))

            report["acceptance"] = {
                "encounter_requires_baron_evidence_and_commits_once": bool(
                    state["before_evidence_rejected"] and state["prereqs"] and began and duplicate_begin_rejected),
                "shutdown_rejects_before_gas_living_defeat": shutdown_before_defeat_rejected,
                "gas_combat_reaches_living_defeat": defeated,
                "bond_rejects_before_coercion_shutdown": bond_before_shutdown_rejected,
                "coercion_governor_shutdown_commits_once": bool(shutdown and shutdown_duplicate_rejected),
                "coercion_off_while_civic_machinery_stays_operational": selective_state,
                "both_civic_pump_actors_remain_unchanged": pumps_unchanged,
                "supplied_34_part_cogfang_bonds_once": bool(bonded and bond_duplicate_rejected and identity),
                "regional_completion_and_optional_echoes_absent_schema8_current": bounded,
            }
            report["measurements"] = {
                "initial_cogfang_health": initial_health,
                "final_defeat_health": defeat_health,
                "gas_damage_applications": strikes,
                "follower_component_count": len(cogfang.follower_mesh_components),
                "coercion_governor_active": region.is_coercion_governor_active(),
                "civic_machinery_operational": region.is_civic_machinery_operational(),
                "civic_pump_count": len(required_pumps),
                "pump_signatures_before": state["pump_before"],
                "pump_signatures_after": pump_after,
            }
            if not all(report["acceptance"].values()):
                raise RuntimeError("Acceptance failure: " + repr(report["acceptance"]))
            stage = "capture_after"
            wait_ticks = 30
    except Exception as exc:
        report["status"] = "FAIL"
        report["failed_stage"] = stage
        report["error"] = repr(exc)
        report["traceback"] = traceback.format_exc()
        write_report()
        unreal.log_error(f"WP23_6_COGFANG_SHUTDOWN_FAIL {stage}: {exc!r}")
        finished = True
        if levels.is_in_play_in_editor():
            levels.editor_request_end_play()


write_report()
handle = unreal.register_slate_post_tick_callback(tick)
levels.editor_play_simulate()
unreal.log("WP-23.6 captive Cogfang selective shutdown PIE proof started")
