"""Exercise all eleven remaining supplied required-horror identities in live PIE."""
import json
import time
import traceback
from pathlib import Path
import unreal

ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
MAP = "/Game/WYRMFALL/World/Regions/L_GloamingMarches"
REPORT = ROOT / "Saved/Diagnostics/WP23_5_required_horror_roster_proof.json"
OUT = ROOT / "Saved/Diagnostics/WP23_5_RequiredHorrorRoster"
levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
editor = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
if not editor.get_editor_world() or MAP not in editor.get_editor_world().get_path_name():
    assert levels.load_level(MAP), f"Could not load {MAP}"
if not any(isinstance(a, unreal.WyrmCharacter) for a in actors.get_all_level_actors()):
    player = actors.spawn_actor_from_class(unreal.WyrmCharacter, unreal.Vector(4300, 4800, 904), unreal.Rotator())
    player.set_actor_label("DIAG_WP23_5_RequiredHorror_Player")
camera = actors.spawn_actor_from_class(unreal.CameraActor, unreal.Vector(5900, 5700, 1250), unreal.Rotator())
camera.set_actor_label("DIAG_GLM_RequiredHorrorRoster")
OUT.mkdir(parents=True, exist_ok=True)

configs = [
    ("AIL_YEN", "AilYen", unreal.WyrmAilYenCharacter, "Alien", 21, 214.0, "gloaming.ail_yen_resolved", "gloaming.ail_yen.living_submission", "AilYen"),
    ("BELLRAISER", "Bellraiser", unreal.WyrmBellraiserCharacter, "Hellraiser", 16, 186.0, "gloaming.bellraiser_resolved", "gloaming.bellraiser.living_submission", "Bellraiser"),
    ("SAD_ECHO", "SadEcho", unreal.WyrmSadEchoCharacter, "Sadoko", 16, 168.0, "gloaming.sad_echo_resolved", "gloaming.sad_echo.living_submission", "SadEcho"),
    ("DREADATOR", "Dreadator", unreal.WyrmDreadatorCharacter, "Predator", 16, 208.0, "gloaming.dreadator_resolved", "gloaming.dreadator.living_submission", "Dreadator"),
    ("ROASTFACE", "Roastface", unreal.WyrmRoastfaceCharacter, "Ghostface", 17, 182.0, "gloaming.roastface_resolved", "gloaming.roastface.disarmed_submission", "Roastface"),
    ("GRAVY_DAUGHTERS", "GravyDaughtersPrimary", unreal.WyrmGravyDaughtersCharacter, "GradyDaughter", 16, 148.0, "gloaming.gravy_daughters_resolved", "gloaming.gravy_daughters.paired_submission", "GravyDaughters"),
    ("KNIT", "Knit", unreal.WyrmKnitCharacter, "_It", 17, 178.0, "gloaming.knit_resolved", "gloaming.knit.balloon_surrender", "Knit"),
    ("CANNIBALL", "Canniball", unreal.WyrmCanniballCharacter, "Hannibal", 13, 180.0, "gloaming.canniball_resolved", "gloaming.canniball.restrained_submission", "Canniball"),
    ("MUMS_THE_WYRD", "MumsTheWyrd", unreal.WyrmMumsTheWyrdCharacter, "TheMummy", 18, 196.0, "gloaming.mums_the_wyrd_resolved", "gloaming.mums_the_wyrd.unbound_submission", "MumsTheWyrd"),
    ("DREADY_FREDDIE", "DreadyFreddie", unreal.WyrmDreadyFreddieCharacter, "Freddy", 16, 184.0, "gloaming.dready_freddie_resolved", "gloaming.dready_freddie.waking_submission", "DreadyFreddie"),
    ("PYRE_MIDHEAD", "PyreMidhead", unreal.WyrmPyreMidheadCharacter, "PyramidHead", 17, 220.0, "gloaming.pyre_midhead_resolved", "gloaming.pyre_midhead.disarmed_submission", "PyreMidhead"),
]

report = {"kind": "wp23_5_required_horror_roster", "engine": unreal.SystemLibrary.get_engine_version(),
          "map": MAP, "status": "RUNNING", "acceptance": {}, "identities": [],
          "not_claimed": ["new Echo rewards", "Gloaming regional completion",
                          "interactive keyboard or gamepad walkthrough", "new packaged build"]}
REPORT.parent.mkdir(parents=True, exist_ok=True)
REPORT.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
started = time.monotonic()
ticks = 0
finished = False
stage = "wait"
wait_ticks = 0
nav_attempts = 0
capture_index = 0
captures = [
    ("01_north_roster.png", unreal.Vector(6100, 5700, 1280), unreal.Vector(5200, 4450, 930)),
    ("02_east_roster.png", unreal.Vector(6600, 3550, 1240), unreal.Vector(5700, 2700, 915)),
    ("03_south_roster.png", unreal.Vector(5000, 450, 1230), unreal.Vector(4300, 1350, 930)),
]

def world_actors(world, cls):
    return list(unreal.GameplayStatics.get_all_actors_of_class(world, cls))

def prerequisites(region):
    return (region.record_arrival() and region.resolve_ashgrave_extraction_seal() and
            region.record_malvaine_resolution(True) and region.record_hollow_twins_resolution(True) and
            region.record_michael_mire_resolution() and region.record_machete_mason_resolution() and
            region.record_pleatherface_resolution() and region.record_wherewolf_resolution() and
            region.record_annie_wails_resolution() and region.record_scarrie_resolution() and
            region.record_chuckles_resolution() and region.record_count_dripula_resolution() and
            region.record_frank_n_shrine_resolution())

def tick(_delta):
    global ticks, finished, stage, wait_ticks, nav_attempts, capture_index
    try:
        if finished:
            if not levels.is_in_play_in_editor():
                unreal.unregister_slate_post_tick_callback(handle)
                actors.destroy_actor(camera)
                unreal.SystemLibrary.quit_editor()
            return
        if time.monotonic() - started > 300:
            raise RuntimeError(f"Timed out in {stage}")
        if wait_ticks:
            wait_ticks -= 1
            return
        if stage == "capture":
            name, origin, target = captures[capture_index]
            camera.set_actor_location(origin, False, False)
            camera.set_actor_rotation(unreal.MathLibrary.find_look_at_rotation(origin, target), False)
            capture_index += 1
            stage = "capture_delay"
            wait_ticks = 45
            requested = bool(unreal.AutomationLibrary.take_high_res_screenshot(1280, 720, str(OUT / name), camera))
            report.setdefault("screenshots", []).append({"path": str(OUT / name), "requested": requested})
            if not requested:
                raise RuntimeError(f"Screenshot request failed: {name}")
            return
        if stage == "capture_delay":
            if capture_index < len(captures):
                stage = "capture"
                return
            report["status"] = "PASS_AUTOMATED_REQUIRES_MANUAL_VISUAL_REVIEW" if all(report["acceptance"].values()) else "FAIL"
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
        adapters = world_actors(world, unreal.WyrmGeoForgeAdapter)
        cameras = [a for a in world_actors(world, unreal.CameraActor) if a.get_actor_label() == "DIAG_GLM_RequiredHorrorRoster"]
        all_by_label = {a.get_actor_label(): a for a in world_actors(world, unreal.Actor)}
        anchors = {a.get_actor_label(): a for a in world_actors(world, unreal.TargetPoint)}
        if len(players) != 1 or len(adapters) != 1 or len(cameras) != 1:
            return
        required_labels = [f"GLM_ENCOUNTER_{c[1]}" for c in configs] + ["GLM_ENCOUNTER_GravyDaughtersSecondary"]
        if not all(label in all_by_label for label in required_labels):
            return
        player = players[0]
        region = unreal.WyrmGloamingSubsystem.get_gloaming_subsystem(world)
        if not region:
            raise RuntimeError("Gloaming fact owner unavailable")

        roster = []
        presentation_ok = True
        for key, label, cls, token, part_count, height, fact, receipt, echo_key in configs:
            actor = all_by_label[f"GLM_ENCOUNTER_{label}"]
            if not isinstance(actor, cls):
                raise RuntimeError(f"Wrong class for {label}")
            loaded_parts = [p for p in actor.get_components_by_class(unreal.StaticMeshComponent)
                            if p.get_editor_property("static_mesh")]
            meshes = [p.get_editor_property("static_mesh") for p in loaded_parts]
            identity_ok = bool(actor.has_complete_presentation() and actor.get_presentation_part_count() == part_count and
                len(loaded_parts) == part_count and all(token in m.get_path_name() for m in meshes) and
                all(p.get_material(0) for p in loaded_parts) and abs(actor.get_presentation_height() - height) <= 1.0)
            presentation_ok = presentation_ok and identity_ok
            roster.append((key, label, actor, fact, receipt, echo_key, part_count, height))
        secondary = all_by_label["GLM_ENCOUNTER_GravyDaughtersSecondary"]
        secondary_loaded = [p for p in secondary.get_components_by_class(unreal.StaticMeshComponent)
                            if p.get_editor_property("static_mesh")]
        pair_ok = bool(secondary.has_complete_presentation() and len(secondary_loaded) == 16 and
                       all("GradyDaughter" in p.get_editor_property("static_mesh").get_path_name() for p in secondary_loaded))

        def project(context, location):
            result = unreal.WyrmTerrainDiagnostics.project_navigation_point(context, location, unreal.Vector(260, 260, 600))
            if isinstance(result, (tuple, list)):
                point = next((v for v in result[1:] if isinstance(v, unreal.Vector)), unreal.Vector())
                return bool(result[0]), point
            return (True, result) if isinstance(result, unreal.Vector) else (False, unreal.Vector())

        route_labels = ["GLM_ROUTE_FRANK_N_SHRINE"] + [f"GLM_ROUTE_{c[0]}" for c in configs]
        if not all(label in anchors for label in route_labels):
            raise RuntimeError("One or more roster route anchors are missing")
        projected = []
        for index, route_label in enumerate(route_labels):
            context = roster[max(0, index - 1)][2]
            projected.append(project(context, anchors[route_label].get_actor_location()))
        if not all(item[0] for item in projected):
            nav_attempts += 1
            if nav_attempts % 120 == 1:
                unreal.WyrmTerrainDiagnostics.refresh_navigation_data_for_actor(terrain)
            if nav_attempts < 600:
                return
            raise RuntimeError("Required-horror route anchors did not project")
        path_counts = []
        for index in range(len(projected) - 1):
            path_counts.append(int(unreal.WyrmTerrainDiagnostics.find_complete_navigation_path_point_count(
                roster[index][2], projected[index][1], projected[index + 1][1])))

        region.reset_gloaming_state()
        for item in roster:
            item[2].reset_encounter()
        rejected = not roster[0][2].begin_encounter(player)
        prereqs = prerequisites(region)
        combat_ok = prereqs
        identity_results = []
        for key, label, actor, fact, receipt, echo_key, part_count, height in roster:
            began = actor.begin_encounter(player)
            attrs = actor.get_attributes()
            attrs.set_current_armor(0.0)
            initial_health = attrs.get_current_health()
            strike_damage = attrs.get_current_max_health() / 8.0
            for _ in range(6):
                unreal.WyrmMeleeAttackAbility.apply_damage_effect(
                    player.get_ability_system(), actor.get_ability_system(), strike_damage)
            submission_health = attrs.get_current_health()
            threshold = 0.0 < submission_health <= attrs.get_current_max_health() * 0.25 + 0.01
            resolved = actor.resolve_after_living_submission(player)
            committed = region.has_fact(unreal.Name(fact)) and region.has_receipt(unreal.Name(receipt))
            duplicate_rejected = not actor.resolve_after_living_submission(player)
            passed = bool(began and threshold and resolved and committed and duplicate_rejected)
            combat_ok = combat_ok and passed
            identity_results.append({"identity": label, "source_parts": part_count, "height_cm": height,
                "initial_health": initial_health, "submission_health": submission_health,
                "gas_damage_applications": 6, "passed": passed})

        snapshot = unreal.WyrmSaveSubsystem.create_snapshot_object("WP23_5_RequiredHorrorRosterMemory", player, adapters[0], world)
        region.reset_gloaming_state()
        restored = bool(snapshot and unreal.WyrmSaveSubsystem.apply_snapshot_object(snapshot, player, adapters[0], world) and
                        all(region.has_fact(unreal.Name(item[3])) and region.has_receipt(unreal.Name(item[4])) for item in roster))
        excluded = (not region.has_fact(unreal.Name("gloaming.region_complete")) and
                    all(not region.has_fact(unreal.Name(f"echo.{item[5].lower()}")) and not player.is_echo_unlocked(item[5]) for item in roster))
        report["identities"] = identity_results
        report["acceptance"] = {
            "all_11_route_anchors_project_to_navigation": all(item[0] for item in projected[1:]),
            "all_11_ordered_route_legs_valid_nonpartial": all(count > 0 for count in path_counts),
            "all_11_supplied_assemblies_complete_and_distinct": presentation_ok,
            "gravy_daughters_present_as_two_instances": pair_ok,
            "first_encounter_rejects_before_frank_n_shrine": rejected,
            "all_11_gas_living_submissions_commit_once": combat_ok,
            "all_11_schema7_facts_and_receipts_restore": restored,
            "new_echoes_and_regional_completion_absent": excluded}
        report["measurements"] = {"identity_count": len(roster), "staged_actor_count": len(roster) + 1,
            "route_leg_count": len(path_counts), "route_path_points": path_counts,
            "total_source_parts": sum(item[6] for item in roster) + 16}
        if not all(report["acceptance"].values()):
            raise RuntimeError(f"Acceptance failure: {report['acceptance']}")
        controller = unreal.GameplayStatics.get_player_controller(world, 0)
        if controller:
            controller.set_view_target_with_blend(camera, 0.0)
        stage = "capture"
        wait_ticks = 30
    except Exception as exc:
        report["status"] = "FAIL"
        report["failed_stage"] = stage
        report["error"] = repr(exc)
        report["traceback"] = traceback.format_exc()
        REPORT.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
        unreal.log_error(f"WP23_5_REQUIRED_HORROR_ROSTER_FAIL {stage}: {exc!r}")
        finished = True
        if levels.is_in_play_in_editor():
            levels.editor_request_end_play()

handle = unreal.register_slate_post_tick_callback(tick)
levels.editor_play_simulate()
unreal.log("WP-23.5 complete required-horror roster PIE proof started")
