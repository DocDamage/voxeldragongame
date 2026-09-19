"""Exercise the bounded Count Malvaine encounter in live PIE."""

import json
from pathlib import Path

import unreal


ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
MAP = "/Game/WYRMFALL/World/Regions/L_GloamingMarches"
REPORT = ROOT / "Saved/Diagnostics/WP23_5_malvaine_slice_proof.json"
OUT = ROOT / "Saved/Diagnostics/WP23_5_Malvaine"
MESH = "/Game/WYRMFALL/Development/Intake/WP23_5/Gloaming/Roles/CountMalvaine/TVS_VoxelCathedral_Priest.TVS_VoxelCathedral_Priest"
MATERIAL = "/Game/WYRMFALL/Development/Intake/WP23_5/Gloaming/FixtureMaterials/M_CountMalvaine_SuppliedPaletteV2.M_CountMalvaine_SuppliedPaletteV2"


def asset_path(value):
    return value.get_path_name() if value else None


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    editor = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
    actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    if not levels.load_level(MAP):
        raise RuntimeError("Could not load " + MAP)

    camera_origin = unreal.Vector(650, -700, 1120)
    camera_target = unreal.Vector(650, -250, 1020)
    camera = actors.spawn_actor_from_class(
        unreal.CameraActor, camera_origin,
        unreal.MathLibrary.find_look_at_rotation(camera_origin, camera_target))
    camera.set_actor_label("DIAG_GLM_MalvaineEncounter")

    state = {
        "kind": "wp23_5_count_malvaine_encounter_slice",
        "engine": unreal.SystemLibrary.get_engine_version(),
        "map": MAP,
        "status": "RUNNING",
        "acceptance": {},
        "manual_visual_review": "NOT_RUN",
        "not_claimed": [
            "Sanguine Strike Echo reward", "Hollow Twins encounter",
            "regional completion", "save or travel recovery",
        ],
    }
    REPORT.parent.mkdir(parents=True, exist_ok=True)
    REPORT.write_text(json.dumps(state, indent=2) + "\n", encoding="utf-8")
    wait_ticks = 0
    nav_wait_checks = 0
    stage = "wait_pie"
    capture_camera = None

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
            screenshot = OUT / "01_malvaine_cathedral.png"
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
        malvaines = unreal.GameplayStatics.get_all_actors_of_class(world, unreal.WyrmCountMalvaineCharacter)
        anchors = {
            actor.get_actor_label(): actor for actor in
            unreal.GameplayStatics.get_all_actors_of_class(world, unreal.TargetPoint)
        }
        runtime_cameras = [actor for actor in
                           unreal.GameplayStatics.get_all_actors_of_class(world, unreal.CameraActor)
                           if actor.get_actor_label() == "DIAG_GLM_MalvaineEncounter"]
        if len(players) != 1 or len(malvaines) != 1 or len(runtime_cameras) != 1:
            return
        if "GLM_ROUTE_ASHGRAVE" not in anchors or "GLM_ROUTE_MALVAINE" not in anchors:
            raise RuntimeError("Missing Ashgrave-to-Malvaine route anchors")

        player, malvaine = players[0], malvaines[0]
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

        start_ok, start = project(malvaine, anchors["GLM_ROUTE_ASHGRAVE"].get_actor_location())
        end_ok, end = project(malvaine, anchors["GLM_ROUTE_MALVAINE"].get_actor_location())
        if (not start_ok or not end_ok) and nav_wait_checks < 600:
            nav_wait_checks += 1
            if nav_wait_checks % 120 == 1:
                unreal.WyrmTerrainDiagnostics.refresh_navigation_data_for_actor(terrains[0])
            return
        path = unreal.NavigationSystemV1.find_path_to_location_synchronously(world, start, end)
        route_ok = bool(start_ok and end_ok and path and path.is_valid() and not path.is_partial())

        presentation = malvaine.get_presentation_mesh()
        mesh_asset = presentation.get_editor_property("skeletal_mesh_asset")
        material_asset = presentation.get_material(0)
        presentation_height = malvaine.get_presentation_height()
        _origin, extent = malvaine.get_actor_bounds(False)
        presentation_ok = bool(
            asset_path(mesh_asset) == MESH and asset_path(material_asset) == MATERIAL and
            150.0 <= presentation_height <= 220.0 and presentation.is_playing() and
            presentation.is_visible())

        # The encounter must remain gated behind the already-proven Ashgrave step.
        region.reset_gloaming_state()
        malvaine.reset_encounter()
        rejected_before_ashgrave = not malvaine.begin_encounter(player)

        # Ordinary parley is a complete route and must remain non-Echo.
        parley_prereqs = region.record_arrival() and region.resolve_ashgrave_extraction_seal()
        parley_started = malvaine.begin_encounter(player)
        parley_resolved = malvaine.resolve_by_parley(player)
        parley_facts = all(region.has_fact(unreal.Name(fact)) for fact in (
            "gloaming.malvaine_parley", "gloaming.malvaine_encounter_resolved"))
        parley_receipts = all(region.has_receipt(unreal.Name(receipt)) for receipt in (
            "gloaming.malvaine.parley", "gloaming.malvaine.encounter_resolved"))
        duplicate_parley_rejected = not malvaine.resolve_by_parley(player)
        no_echo_after_parley = not player.is_echo_unlocked("SanguineStrike")

        # Reset only the bounded proof state, then drive the living-defeat threshold via GAS damage.
        region.reset_gloaming_state()
        malvaine.reset_encounter()
        combat_prereqs = region.record_arrival() and region.resolve_ashgrave_extraction_seal()
        combat_started = malvaine.begin_encounter(player)
        attrs = malvaine.get_attributes()
        attrs.set_current_armor(0.0)
        initial_health = attrs.get_current_health()
        strikes = 0
        while attrs.get_current_health() > attrs.get_current_max_health() * 0.25 and strikes < 10:
            unreal.WyrmMeleeAttackAbility.apply_damage_effect(
                player.get_ability_system(), malvaine.get_ability_system(), 100.0)
            strikes += 1
        threshold_health = attrs.get_current_health()
        gas_threshold_reached = 0.0 < threshold_health <= attrs.get_current_max_health() * 0.25
        combat_resolved = malvaine.resolve_after_living_defeat(player)
        combat_facts = all(region.has_fact(unreal.Name(fact)) for fact in (
            "gloaming.malvaine_living_defeat", "gloaming.malvaine_encounter_resolved"))
        combat_receipts = all(region.has_receipt(unreal.Name(receipt)) for receipt in (
            "gloaming.malvaine.living_defeat", "gloaming.malvaine.encounter_resolved"))
        duplicate_combat_rejected = not malvaine.resolve_after_living_defeat(player)
        excluded_absent = (
            not player.is_echo_unlocked("SanguineStrike") and
            not region.has_fact(unreal.Name("echo.sanguine_strike")) and
            not region.has_fact(unreal.Name("gloaming.hollow_twins_resolved")) and
            not region.has_fact(unreal.Name("gloaming.region_complete")))

        state["acceptance"] = {
            "ashgrave_anchor_projects_to_navigation": start_ok,
            "malvaine_anchor_projects_to_navigation": end_ok,
            "ashgrave_to_malvaine_path_valid_nonpartial": route_ok,
            "supplied_priest_presentation_and_preach_animation": presentation_ok,
            "encounter_rejects_before_ashgrave": rejected_before_ashgrave,
            "ordinary_parley_route_commits_once": bool(
                parley_prereqs and parley_started and parley_resolved and parley_facts and
                parley_receipts and duplicate_parley_rejected),
            "parley_does_not_unlock_echo": no_echo_after_parley,
            "gas_living_defeat_route_commits_once": bool(
                combat_prereqs and combat_started and initial_health > threshold_health and
                gas_threshold_reached and combat_resolved and combat_facts and
                combat_receipts and duplicate_combat_rejected),
            "excluded_outcomes_remain_absent": excluded_absent,
        }
        state["combat"] = {
            "initial_health": initial_health, "threshold_health": threshold_health,
            "gas_damage_applications": strikes,
        }
        state["route_path_points"] = len(path.path_points) if path else 0
        state["presentation"] = {
            "mesh": asset_path(mesh_asset), "material": asset_path(material_asset),
            "actor_height_cm": extent.z * 2.0, "presentation_height_cm": presentation_height,
            "animation_playing": presentation.is_playing(),
            "mesh_visible": presentation.is_visible(),
            "mesh_component_location": list(presentation.get_world_location().to_tuple()),
            "actor_location_before_visual_reset": list(malvaine.get_actor_location().to_tuple()),
        }
        # Frame from the live actor position so AI movement cannot turn the visual proof into an empty-landscape shot.
        encounter_location = unreal.Vector(650, -250, 990)
        malvaine.set_actor_location(encounter_location, False, True)
        malvaine.set_actor_rotation(unreal.Rotator(0, 0, -90), False)
        presentation.set_position(0.25, False)
        presentation.stop()
        capture_camera = runtime_cameras[0]
        capture_camera.set_actor_location(
            encounter_location + unreal.Vector(0, -420, 120), False, False)
        capture_camera.set_actor_rotation(unreal.MathLibrary.find_look_at_rotation(
            capture_camera.get_actor_location(), encounter_location + unreal.Vector(0, 0, 70)), False)
        controller = unreal.GameplayStatics.get_player_controller(world, 0)
        if controller:
            controller.set_view_target_with_blend(capture_camera, 0.0)
        stage = "capture"
        wait_ticks = 30
        return

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
    print("WP-23.5 Count Malvaine encounter PIE proof started")


main()
