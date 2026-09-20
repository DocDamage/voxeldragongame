"""Exercise the bounded Hollow Twins encounter in live PIE."""

import json
from pathlib import Path

import unreal


ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
MAP = "/Game/WYRMFALL/World/Regions/L_GloamingMarches"
REPORT = ROOT / "Saved/Diagnostics/WP23_5_hollow_twins_slice_proof.json"
OUT = ROOT / "Saved/Diagnostics/WP23_5_HollowTwins"
MESH = "/Game/WYRMFALL/Development/Intake/WP23_5/Gloaming/Roles/HollowTwins/TVS_VoxelCathedral_Nun.TVS_VoxelCathedral_Nun"
MATERIALS = {
    "GLM_ENCOUNTER_HollowTwinMorrow": "/Game/WYRMFALL/Development/Intake/WP23_5/Gloaming/FixtureMaterials/M_HollowTwinMorrow_ProductionPalette.M_HollowTwinMorrow_ProductionPalette",
    "GLM_ENCOUNTER_HollowTwinMourn": "/Game/WYRMFALL/Development/Intake/WP23_5/Gloaming/FixtureMaterials/M_HollowTwinMourn_ProductionPalette.M_HollowTwinMourn_ProductionPalette",
}


def asset_path(value):
    return value.get_path_name() if value else None


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    editor = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
    actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    if not levels.load_level(MAP):
        raise RuntimeError("Could not load " + MAP)

    camera_origin = unreal.Vector(3900, 1150, 1210)
    camera_target = unreal.Vector(3900, 2100, 1010)
    camera = actors.spawn_actor_from_class(
        unreal.CameraActor, camera_origin,
        unreal.MathLibrary.find_look_at_rotation(camera_origin, camera_target))
    camera.set_actor_label("DIAG_GLM_HollowTwinsEncounter")

    state = {
        "kind": "wp23_5_hollow_twins_encounter_slice",
        "engine": unreal.SystemLibrary.get_engine_version(),
        "map": MAP,
        "status": "RUNNING",
        "acceptance": {},
        "manual_visual_review": "NOT_RUN",
        "not_claimed": [
            "Second Turn Echo reward", "Gloaming travel or save recovery",
            "regional completion", "interactive keyboard or gamepad walkthrough",
        ],
    }
    REPORT.parent.mkdir(parents=True, exist_ok=True)
    REPORT.write_text(json.dumps(state, indent=2) + "\n", encoding="utf-8")
    wait_ticks = 0
    nav_wait_checks = 0
    stage = "wait_pie"
    capture_camera = None

    def prepare_prerequisites(region):
        return (region.record_arrival() and region.resolve_ashgrave_extraction_seal() and
                region.record_malvaine_resolution(True))

    def tick(_delta):
        nonlocal wait_ticks, nav_wait_checks, stage, capture_camera
        if stage == "ending":
            if not levels.is_in_play_in_editor():
                unreal.unregister_slate_post_tick_callback(handle)
                actors.destroy_actor(camera)
                unreal.SystemLibrary.quit_editor()
            return
        if stage == "capture_wait":
            return
        if wait_ticks:
            wait_ticks -= 1
            return
        if stage == "capture":
            screenshot = OUT / "01_hollow_twins_haunting.png"
            state["screenshot"] = {"path": str(screenshot), "requested": True}
            stage = "capture_wait"
            wait_ticks = 75
            requested = bool(unreal.AutomationLibrary.take_high_res_screenshot(
                1280, 720, str(screenshot), capture_camera))
            state["screenshot"]["requested"] = requested
            state["status"] = (
                "PASS_AUTOMATED_REQUIRES_MANUAL_VISUAL_REVIEW"
                if all(state["acceptance"].values()) and requested else "FAIL")
            REPORT.write_text(json.dumps(state, indent=2) + "\n", encoding="utf-8")
            return

        world = editor.get_game_world()
        if not world or not levels.is_in_play_in_editor():
            return
        terrains = unreal.GameplayStatics.get_all_actors_of_class(world, unreal.GeoForgeInfiniteTerrainActor)
        if len(terrains) != 1:
            return
        stats = terrains[0].get_runtime_render_stats()
        if (stats.get_editor_property("loaded_chunk_count") < 81 or
                stats.get_editor_property("queued_chunk_generation_count") or
                stats.get_editor_property("queued_chunk_rebuild_count") or
                stats.get_editor_property("pending_chunk_apply_count") or
                unreal.WyrmTerrainDiagnostics.is_navigation_build_pending(terrains[0])):
            return

        players = unreal.GameplayStatics.get_all_actors_of_class(world, unreal.WyrmCharacter)
        twins = unreal.GameplayStatics.get_all_actors_of_class(world, unreal.WyrmHollowTwinCharacter)
        twins_by_label = {actor.get_actor_label(): actor for actor in twins}
        anchors = {actor.get_actor_label(): actor for actor in
                   unreal.GameplayStatics.get_all_actors_of_class(world, unreal.TargetPoint)}
        runtime_cameras = [actor for actor in
                           unreal.GameplayStatics.get_all_actors_of_class(world, unreal.CameraActor)
                           if actor.get_actor_label() == "DIAG_GLM_HollowTwinsEncounter"]
        if len(players) != 1 or len(twins) != 2 or len(runtime_cameras) != 1:
            return
        if not all(label in twins_by_label for label in MATERIALS):
            raise RuntimeError("Missing authored Hollow Twin identities")
        if "GLM_ROUTE_MALVAINE" not in anchors or "GLM_ROUTE_TWINS" not in anchors:
            raise RuntimeError("Missing Malvaine-to-Twins route anchors")

        player = players[0]
        morrow = twins_by_label["GLM_ENCOUNTER_HollowTwinMorrow"]
        mourn = twins_by_label["GLM_ENCOUNTER_HollowTwinMourn"]
        region = unreal.WyrmGloamingSubsystem.get_gloaming_subsystem(world)
        if not region:
            raise RuntimeError("Gloaming fact owner unavailable in PIE")

        def project(context, location):
            result = unreal.WyrmTerrainDiagnostics.project_navigation_point(
                context, location, unreal.Vector(180, 180, 400))
            if isinstance(result, (tuple, list)):
                return bool(result[0]), next(
                    (item for item in result[1:] if isinstance(item, unreal.Vector)), unreal.Vector())
            if isinstance(result, unreal.Vector):
                return True, result
            return False, unreal.Vector()

        start_ok, start = project(morrow, anchors["GLM_ROUTE_MALVAINE"].get_actor_location())
        end_ok, end = project(morrow, anchors["GLM_ROUTE_TWINS"].get_actor_location())
        if (not start_ok or not end_ok) and nav_wait_checks < 600:
            nav_wait_checks += 1
            if nav_wait_checks % 120 == 1:
                unreal.WyrmTerrainDiagnostics.refresh_navigation_data_for_actor(terrains[0])
            return
        path = unreal.NavigationSystemV1.find_path_to_location_synchronously(world, start, end)
        route_ok = bool(start_ok and end_ok and path and path.is_valid() and not path.is_partial())

        presentations = {}
        presentation_ok = True
        for label, twin in twins_by_label.items():
            component = twin.get_presentation_mesh()
            mesh_asset = component.get_editor_property("skeletal_mesh_asset")
            material_asset = component.get_material(0)
            height = twin.get_presentation_height()
            row_ok = bool(
                asset_path(mesh_asset) == MESH and asset_path(material_asset) == MATERIALS[label] and
                150.0 <= height <= 220.0 and component.is_playing() and component.is_visible())
            presentation_ok = presentation_ok and row_ok
            presentations[label] = {
                "identity": str(twin.get_editor_property("twin_identity")),
                "mesh": asset_path(mesh_asset), "material": asset_path(material_asset),
                "presentation_height_cm": height, "animation_playing": component.is_playing(),
                "mesh_visible": component.is_visible(),
            }
        distinct_pair = bool(
            morrow.has_distinct_valid_partner() and mourn.has_distinct_valid_partner() and
            morrow.get_editor_property("twin_identity") != mourn.get_editor_property("twin_identity") and
            MATERIALS[morrow.get_actor_label()] != MATERIALS[mourn.get_actor_label()])

        region.reset_gloaming_state()
        morrow.reset_encounter_pair()
        rejected_before_malvaine = not morrow.begin_encounter(player)

        release_prereqs = prepare_prerequisites(region)
        release_started = morrow.begin_encounter(player)
        release_resolved = morrow.resolve_by_release(player)
        release_facts = all(region.has_fact(unreal.Name(fact)) for fact in (
            "gloaming.hollow_twins_released", "gloaming.hollow_twins_resolved"))
        release_receipts = all(region.has_receipt(unreal.Name(receipt)) for receipt in (
            "gloaming.hollow_twins.released", "gloaming.hollow_twins.encounter_resolved"))
        duplicate_release_rejected = not mourn.resolve_by_release(player)
        both_released = bool(
            morrow.has_matching_gameplay_tag("State.Combat.Invulnerable") and
            mourn.has_matching_gameplay_tag("State.Combat.Invulnerable"))

        region.reset_gloaming_state()
        morrow.reset_encounter_pair()
        combat_prereqs = prepare_prerequisites(region)
        combat_started = mourn.begin_encounter(player)
        combat = {}
        gas_thresholds = True
        for twin in (morrow, mourn):
            attrs = twin.get_attributes()
            attrs.set_current_armor(0.0)
            initial = attrs.get_current_health()
            strikes = 0
            while attrs.get_current_health() > attrs.get_current_max_health() * 0.25 and strikes < 10:
                unreal.WyrmMeleeAttackAbility.apply_damage_effect(
                    player.get_ability_system(), twin.get_ability_system(), 75.0)
                strikes += 1
            current = attrs.get_current_health()
            gas_thresholds = gas_thresholds and 0.0 < current <= attrs.get_current_max_health() * 0.25
            combat[twin.get_actor_label()] = {
                "initial_health": initial, "threshold_health": current,
                "gas_damage_applications": strikes,
            }
        submission_resolved = mourn.resolve_after_living_submission(player)
        submission_facts = all(region.has_fact(unreal.Name(fact)) for fact in (
            "gloaming.hollow_twins_living_submission", "gloaming.hollow_twins_resolved"))
        submission_receipts = all(region.has_receipt(unreal.Name(receipt)) for receipt in (
            "gloaming.hollow_twins.living_submission", "gloaming.hollow_twins.encounter_resolved"))
        duplicate_submission_rejected = not morrow.resolve_after_living_submission(player)
        excluded_absent = (
            not player.is_echo_unlocked("SecondTurn") and
            not player.is_echo_unlocked("SanguineStrike") and
            not region.has_fact(unreal.Name("echo.second_turn")) and
            not region.has_fact(unreal.Name("echo.sanguine_strike")) and
            not region.has_fact(unreal.Name("gloaming.region_complete")))

        state["acceptance"] = {
            "malvaine_anchor_projects_to_navigation": start_ok,
            "twins_anchor_projects_to_navigation": end_ok,
            "malvaine_to_twins_path_valid_nonpartial": route_ok,
            "two_distinct_authored_twin_identities": distinct_pair,
            "supplied_nun_presentations_and_prayer_animations": presentation_ok,
            "encounter_rejects_before_malvaine": rejected_before_malvaine,
            "compassionate_release_route_commits_once": bool(
                release_prereqs and release_started and release_resolved and release_facts and
                release_receipts and duplicate_release_rejected and both_released),
            "gas_living_submission_requires_both_twins_and_commits_once": bool(
                combat_prereqs and combat_started and gas_thresholds and submission_resolved and
                submission_facts and submission_receipts and duplicate_submission_rejected),
            "echoes_and_later_outcomes_remain_absent": excluded_absent,
        }
        state["combat"] = combat
        state["route_path_points"] = len(path.path_points) if path else 0
        state["presentations"] = presentations

        morrow_location = unreal.Vector(3600, 2050, 990)
        mourn_location = unreal.Vector(4200, 2050, 990)
        morrow.set_actor_location(morrow_location, False, True)
        mourn.set_actor_location(mourn_location, False, True)
        morrow.set_actor_rotation(unreal.Rotator(0, 0, -30), False)
        mourn.set_actor_rotation(unreal.Rotator(0, 0, -30), False)
        morrow.get_presentation_mesh().set_position(0.15, False)
        mourn.get_presentation_mesh().set_position(0.75, False)
        morrow.get_presentation_mesh().stop()
        mourn.get_presentation_mesh().stop()
        capture_camera = runtime_cameras[0]
        capture_camera.set_actor_location(camera_origin, False, False)
        capture_camera.set_actor_rotation(unreal.MathLibrary.find_look_at_rotation(
            camera_origin, camera_target), False)
        controller = unreal.GameplayStatics.get_player_controller(world, 0)
        if controller:
            controller.set_view_target_with_blend(capture_camera, 0.0)
        stage = "capture"
        wait_ticks = 30

    def finish_tick(_delta):
        nonlocal stage, wait_ticks
        if stage != "capture_wait":
            return
        if wait_ticks:
            wait_ticks -= 1
            return
        stage = "ending"
        levels.editor_request_end_play()

    def combined_tick(delta):
        tick(delta)
        finish_tick(delta)

    handle = unreal.register_slate_post_tick_callback(combined_tick)
    levels.editor_request_begin_play()
    print("WP-23.5 Hollow Twins encounter PIE proof started")


main()
