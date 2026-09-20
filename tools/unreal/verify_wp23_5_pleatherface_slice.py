"""Exercise the bounded Pleatherface required-horror encounter in live PIE."""
import json
import time
import traceback
from pathlib import Path
import unreal

ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
MAP = "/Game/WYRMFALL/World/Regions/L_GloamingMarches"
REPORT = ROOT / "Saved/Diagnostics/WP23_5_pleatherface_slice_proof.json"
OUT = ROOT / "Saved/Diagnostics/WP23_5_Pleatherface"
levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
editor = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
if not editor.get_editor_world() or MAP not in editor.get_editor_world().get_path_name():
    assert levels.load_level(MAP), f"Could not load {MAP}"
if not any(isinstance(a, unreal.WyrmCharacter) for a in actors.get_all_level_actors()):
    player = actors.spawn_actor_from_class(unreal.WyrmCharacter, unreal.Vector(3900, 1700, 904), unreal.Rotator())
    player.set_actor_label("DIAG_WP23_5_Pleatherface_Player")
camera_origin = unreal.Vector(3675, 1725, 1040)
camera_target = unreal.Vector(4050, 2050, 990)
camera = actors.spawn_actor_from_class(
    unreal.CameraActor, camera_origin, unreal.MathLibrary.find_look_at_rotation(camera_origin, camera_target))
camera.set_actor_label("DIAG_GLM_PleatherfaceEncounter")
OUT.mkdir(parents=True, exist_ok=True)
report = {"kind": "wp23_5_pleatherface_encounter_slice", "engine": unreal.SystemLibrary.get_engine_version(),
          "map": MAP, "status": "RUNNING", "acceptance": {},
          "not_claimed": ["new Echo reward", "additional required-horror identities", "regional completion",
                          "interactive keyboard or gamepad walkthrough", "new packaged build"]}
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

def prerequisites(region):
    return (region.record_arrival() and region.resolve_ashgrave_extraction_seal() and
            region.record_malvaine_resolution(True) and region.record_hollow_twins_resolution(True) and
            region.record_michael_mire_resolution() and region.record_machete_mason_resolution())

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
            screenshot = OUT / "01_pleatherface_encounter.png"
            requested = bool(unreal.AutomationLibrary.take_high_res_screenshot(1280, 720, str(screenshot), capture_camera))
            report["screenshot"] = {"path": str(screenshot), "requested": requested}
            report["status"] = "PASS_AUTOMATED_REQUIRES_MANUAL_VISUAL_REVIEW" if requested and all(report["acceptance"].values()) else "FAIL"
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
        encounters = world_actors(world, unreal.WyrmPleatherfaceCharacter)
        cameras = [a for a in world_actors(world, unreal.CameraActor) if a.get_actor_label() == "DIAG_GLM_PleatherfaceEncounter"]
        anchors = {a.get_actor_label(): a for a in world_actors(world, unreal.TargetPoint)}
        adapters = world_actors(world, unreal.WyrmGeoForgeAdapter)
        if len(players) != 1 or len(encounters) != 1 or len(cameras) != 1 or len(adapters) != 1:
            return
        if "GLM_ROUTE_MACHETE_MASON" not in anchors or "GLM_ROUTE_PLEATHERFACE" not in anchors:
            raise RuntimeError("Missing Machete-Mason-to-Pleatherface route anchors")
        player, face = players[0], encounters[0]
        region = unreal.WyrmGloamingSubsystem.get_gloaming_subsystem(world)
        if not region:
            raise RuntimeError("Gloaming fact owner unavailable")
        def project(location):
            result = unreal.WyrmTerrainDiagnostics.project_navigation_point(face, location, unreal.Vector(240, 240, 500))
            if isinstance(result, (tuple, list)):
                point = next((v for v in result[1:] if isinstance(v, unreal.Vector)), unreal.Vector())
                return bool(result[0]), point
            return (True, result) if isinstance(result, unreal.Vector) else (False, unreal.Vector())
        start_ok, start = project(anchors["GLM_ROUTE_MACHETE_MASON"].get_actor_location())
        end_ok, end = project(anchors["GLM_ROUTE_PLEATHERFACE"].get_actor_location())
        if not start_ok or not end_ok:
            nav_attempts += 1
            if nav_attempts % 120 == 1:
                unreal.WyrmTerrainDiagnostics.refresh_navigation_data_for_actor(terrain)
            if nav_attempts < 600:
                return
            raise RuntimeError("Pleatherface route anchors did not project")
        path_points = int(unreal.WyrmTerrainDiagnostics.find_complete_navigation_path_point_count(face, start, end))
        parts = face.get_components_by_class(unreal.StaticMeshComponent)
        meshes = [p.get_editor_property("static_mesh") for p in parts]
        presentation_ok = bool(face.has_complete_presentation() and face.get_presentation_part_count() == 17 and
            len(parts) == 17 and all(meshes) and all("Leatherface" in m.get_path_name() for m in meshes) and
            all(p.get_material(0) for p in parts) and any("Sword_Leatherface" in m.get_path_name() for m in meshes) and
            191.0 <= face.get_presentation_height() <= 193.0)
        region.reset_gloaming_state()
        face.reset_encounter()
        rejected = not face.begin_encounter(player)
        prereqs = prerequisites(region)
        began = face.begin_encounter(player)
        attrs = face.get_attributes()
        attrs.set_current_armor(0.0)
        initial_health = attrs.get_current_health()
        strikes = 0
        while attrs.get_current_health() > attrs.get_current_max_health() * 0.25 and strikes < 12:
            unreal.WyrmMeleeAttackAbility.apply_damage_effect(player.get_ability_system(), face.get_ability_system(), 75.0)
            strikes += 1
        submission_health = attrs.get_current_health()
        threshold = 0.0 < submission_health <= attrs.get_current_max_health() * 0.25
        resolved = face.resolve_after_disarmed_submission(player)
        committed = region.has_fact(unreal.Name("gloaming.pleatherface_resolved")) and region.has_receipt(unreal.Name("gloaming.pleatherface.disarmed_submission"))
        duplicate_rejected = not face.resolve_after_disarmed_submission(player)
        snapshot = unreal.WyrmSaveSubsystem.create_snapshot_object("WP23_5_PleatherfaceMemory", player, adapters[0], world)
        region.reset_gloaming_state()
        restored = bool(snapshot and unreal.WyrmSaveSubsystem.apply_snapshot_object(snapshot, player, adapters[0], world) and
            region.has_fact(unreal.Name("gloaming.pleatherface_resolved")) and region.has_receipt(unreal.Name("gloaming.pleatherface.disarmed_submission")))
        excluded = not region.has_fact(unreal.Name("gloaming.region_complete")) and not region.has_fact(unreal.Name("echo.pleatherface")) and not player.is_echo_unlocked("Pleatherface")
        report["acceptance"] = {
            "machete_mason_anchor_projects_to_navigation": start_ok,
            "pleatherface_anchor_projects_to_navigation": end_ok,
            "mason_to_pleatherface_path_valid_nonpartial": path_points > 0,
            "supplied_17_part_weapon_192cm_presentation": presentation_ok,
            "encounter_rejects_before_machete_mason": rejected,
            "gas_disarmed_submission_commits_once": bool(prereqs and began and threshold and resolved and committed and duplicate_rejected),
            "gloaming_ledger_roundtrip_restores_receipt": restored,
            "echo_and_regional_completion_absent": excluded}
        report["measurements"] = {"part_count": len(parts), "presentation_height_cm": face.get_presentation_height(),
            "initial_health": initial_health, "submission_health": submission_health,
            "gas_damage_applications": strikes, "route_path_points": path_points}
        if not all(report["acceptance"].values()):
            raise RuntimeError(f"Acceptance failure: {report['acceptance']}")
        face.set_actor_location(unreal.Vector(4050, 2050, 904), False, True)
        face.set_actor_rotation(unreal.Rotator(0, 0, 45), False)
        capture_camera = cameras[0]
        capture_camera.set_actor_location(camera_origin, False, False)
        capture_camera.set_actor_rotation(unreal.MathLibrary.find_look_at_rotation(camera_origin, camera_target), False)
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
        unreal.log_error(f"WP23_5_PLEATHERFACE_FAIL {stage}: {exc!r}")
        finished = True
        if levels.is_in_play_in_editor():
            levels.editor_request_end_play()

handle = unreal.register_slate_post_tick_callback(tick)
levels.editor_play_simulate()
unreal.log("WP-23.5 Pleatherface PIE proof started")
