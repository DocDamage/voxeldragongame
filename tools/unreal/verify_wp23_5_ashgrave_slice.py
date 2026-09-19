"""Exercise the bounded Gloaming Arrival-to-Ashgrave gameplay slice in live PIE."""

import json
from pathlib import Path

import unreal


ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
MAP = "/Game/WYRMFALL/World/Regions/L_GloamingMarches"
REPORT = ROOT / "Saved/Diagnostics/WP23_5_ashgrave_slice_proof.json"


def main():
    levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    editor = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
    if not levels.load_level(MAP):
        raise RuntimeError("Could not load " + MAP)

    state = {
        "kind": "wp23_5_arrival_to_ashgrave_gameplay_slice",
        "engine": unreal.SystemLibrary.get_engine_version(),
        "map": MAP,
        "status": "RUNNING",
        "acceptance": {},
        "not_claimed": [
            "Count Malvaine encounter",
            "Hollow Twins encounter",
            "Echo reward",
            "regional completion",
            "save or travel recovery",
        ],
    }
    REPORT.parent.mkdir(parents=True, exist_ok=True)
    REPORT.write_text(json.dumps(state, indent=2) + "\n", encoding="utf-8")
    wait_ticks = 0
    nav_wait_checks = 0
    stage = "wait_pie"

    def tick(_delta):
        nonlocal wait_ticks, nav_wait_checks, stage
        if stage == "ending":
            if not levels.is_in_play_in_editor():
                unreal.unregister_slate_post_tick_callback(handle)
                unreal.SystemLibrary.quit_editor()
            return
        if wait_ticks:
            wait_ticks -= 1
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
        arrivals = unreal.GameplayStatics.get_all_actors_of_class(world, unreal.WyrmGloamingArrivalTrigger)
        seals = unreal.GameplayStatics.get_all_actors_of_class(world, unreal.WyrmGloamingAshgraveSeal)
        anchors = {
            actor.get_actor_label(): actor for actor in
            unreal.GameplayStatics.get_all_actors_of_class(world, unreal.TargetPoint)
        }
        if len(players) != 1 or len(arrivals) != 1 or len(seals) != 1:
            return
        if "GLM_ROUTE_ARRIVAL" not in anchors or "GLM_ROUTE_ASHGRAVE" not in anchors:
            raise RuntimeError("Missing Arrival-to-Ashgrave route anchors")

        player, arrival, seal = players[0], arrivals[0], seals[0]
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

        start_ok, start = project(arrival, anchors["GLM_ROUTE_ARRIVAL"].get_actor_location())
        end_ok, end = project(seal, anchors["GLM_ROUTE_ASHGRAVE"].get_actor_location())
        if (not start_ok or not end_ok) and nav_wait_checks < 600:
            nav_wait_checks += 1
            if nav_wait_checks % 120 == 1:
                unreal.WyrmTerrainDiagnostics.refresh_navigation_data_for_actor(terrains[0])
            return
        path = unreal.NavigationSystemV1.find_path_to_location_synchronously(world, start, end)
        route_ok = bool(start_ok and end_ok and path and path.is_valid() and not path.is_partial())

        region.reset_gloaming_state()
        player.set_actor_location(seal.get_actor_location(), False, True)
        precondition_rejected = not seal.interact(player)

        player.set_actor_location(arrival.get_actor_location(), False, True)
        arrival_automatic = region.has_fact(unreal.Name("gloaming.arrival"))
        arrival_committed = arrival_automatic or arrival.trigger_arrival(player)
        arrival_fact = region.has_fact(unreal.Name("gloaming.arrival"))
        duplicate_arrival_rejected = not arrival.trigger_arrival(player)

        player.set_actor_location(seal.get_actor_location(), False, True)
        ashgrave_resolved = seal.interact(player)
        ashgrave_fact = region.has_fact(unreal.Name("gloaming.ashgrave_extraction_seal_resolved"))
        receipt = region.has_receipt(unreal.Name("gloaming.ashgrave_extraction_seal.resolved"))
        duplicate_seal_rejected = not seal.interact(player)
        excluded_facts_absent = not any(region.has_fact(unreal.Name(fact)) for fact in (
            "echo.gloaming", "gloaming.region_complete", "gloaming.malvaine_resolved",
            "gloaming.hollow_twins_resolved"))

        state["acceptance"] = {
            "arrival_anchor_projects_to_navigation": start_ok,
            "ashgrave_anchor_projects_to_navigation": end_ok,
            "arrival_to_ashgrave_path_valid_nonpartial": route_ok,
            "seal_rejects_before_arrival": precondition_rejected,
            "arrival_trigger_commits_once": arrival_committed and arrival_fact and duplicate_arrival_rejected,
            "ashgrave_seal_commits_once": ashgrave_resolved and ashgrave_fact and receipt and duplicate_seal_rejected,
            "excluded_outcomes_remain_absent": excluded_facts_absent,
        }
        state["route_path_points"] = len(path.path_points) if path else 0
        state["arrival_commit_mode"] = "overlap" if arrival_automatic else "explicit_proof_call"
        state["status"] = "PASS" if all(state["acceptance"].values()) else "FAIL"
        REPORT.write_text(json.dumps(state, indent=2) + "\n", encoding="utf-8")
        stage = "ending"
        levels.editor_request_end_play()

    handle = unreal.register_slate_post_tick_callback(tick)
    levels.editor_request_begin_play()
    print("WP-23.5 Arrival-to-Ashgrave gameplay PIE proof started")


main()
