"""Run live PIE navigation checks and capture the Gloaming foundation."""

import json
from pathlib import Path

import unreal


ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
MAP = "/Game/WYRMFALL/World/Regions/L_GloamingMarches"
OUT = ROOT / "Saved/Diagnostics/WP23_5_GloamingFoundation"
REPORT = ROOT / "Saved/Diagnostics/WP23_5_gloaming_foundation_proof.json"
ORDER = ["GLM_ROUTE_ARRIVAL", "GLM_ROUTE_ASHGRAVE", "GLM_ROUTE_MALVAINE", "GLM_ROUTE_TWINS"]
VIEWS = [
    ("arrival_gate", (-4650, -3300, 1225), (-3800, -2450, 1080)),
    ("ashgrave", (-4100, 850, 1450), (-2350, 150, 1050)),
    ("malvaine_cathedral", (2500, -900, 1850), (500, 1800, 1320)),
    ("twins_overview", (5550, 1550, 1700), (3900, 2500, 1120)),
]


def vec(values):
    return unreal.Vector(float(values[0]), float(values[1]), float(values[2]))


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    editor = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
    actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    if not levels.load_level(MAP):
        raise RuntimeError("Could not load " + MAP)
    world = editor.get_editor_world()

    cameras = []
    for name, origin, target in VIEWS:
        camera = actors.spawn_actor_from_class(
            unreal.CameraActor, vec(origin), unreal.MathLibrary.find_look_at_rotation(vec(origin), vec(target)))
        camera.set_actor_label("DIAG_GLM_" + name)
        cameras.append((name, camera))
    editor_nav = unreal.NavigationSystemV1.get_navigation_system(world)
    nav_bounds = [actor for actor in actors.get_all_level_actors() if isinstance(actor, unreal.NavMeshBoundsVolume)]
    if not editor_nav or not nav_bounds:
        raise RuntimeError("Gloaming foundation requires navigation system and bounds")
    proof_bound = actors.spawn_actor_from_class(unreal.NavMeshBoundsVolume, unreal.Vector(0, 0, 900))
    proof_bound.set_actor_scale3d(unreal.Vector(100, 100, 30))
    proof_bound.set_actor_label("DIAG_GLM_NavRegistration")
    for bound in nav_bounds + [proof_bound]:
        editor_nav.on_navigation_bounds_updated(bound)

    state = {
        "kind": "wp23_5_gloaming_foundation", "engine": unreal.SystemLibrary.get_engine_version(),
        "map": MAP, "status": "RUNNING", "route_navigation": [], "screenshots": [],
        "manual_visual_review": "NOT_RUN",
        "not_claimed": ["encounters", "region completion", "interactive walkthrough"],
    }
    REPORT.parent.mkdir(parents=True, exist_ok=True)
    REPORT.write_text(json.dumps(state, indent=2) + "\n", encoding="utf-8")
    stage = "wait_pie"
    wait_ticks = 0
    nav_wait_checks = 0
    capture_index = 0
    runtime_cameras = []

    def tick(_delta):
        nonlocal stage, wait_ticks, nav_wait_checks, capture_index, runtime_cameras
        if stage == "ending":
            if not levels.is_in_play_in_editor():
                unreal.unregister_slate_post_tick_callback(handle)
                for _, camera in cameras:
                    actors.destroy_actor(camera)
                actors.destroy_actor(proof_bound)
                unreal.SystemLibrary.quit_editor()
            return
        if wait_ticks:
            wait_ticks -= 1
            return
        game_world = editor.get_game_world()
        if stage == "wait_pie":
            if not game_world or not levels.is_in_play_in_editor():
                return
            terrains = unreal.GameplayStatics.get_all_actors_of_class(game_world, unreal.GeoForgeInfiniteTerrainActor)
            if len(terrains) != 1:
                return
            stats = terrains[0].get_runtime_render_stats()
            if (stats.get_editor_property("loaded_chunk_count") < 81 or
                    stats.get_editor_property("queued_chunk_generation_count") or
                    stats.get_editor_property("queued_chunk_rebuild_count") or
                    stats.get_editor_property("pending_chunk_apply_count") or
                    unreal.WyrmTerrainDiagnostics.is_navigation_build_pending(terrains[0])):
                return
            anchors = {
                actor.get_actor_label(): actor for actor in
                unreal.GameplayStatics.get_all_actors_of_class(game_world, unreal.TargetPoint)
                if actor.get_actor_label() in ORDER
            }
            if set(anchors) != set(ORDER):
                raise RuntimeError("Missing Gloaming route anchors in PIE")
            rows = []
            projected = {}
            for label in ORDER:
                location = anchors[label].get_actor_location()
                result = unreal.WyrmTerrainDiagnostics.project_navigation_point(
                    anchors[label], location, unreal.Vector(180, 180, 400))
                if isinstance(result, (tuple, list)):
                    ok = bool(result[0])
                    point = next((item for item in result[1:] if isinstance(item, unreal.Vector)), unreal.Vector())
                elif isinstance(result, unreal.Vector):
                    ok, point = True, result
                else:
                    ok, point = bool(result), unreal.Vector()
                projected[label] = point
                rows.append({"anchor": label, "projects_to_navigation": ok,
                             "projected": list(point.to_tuple()) if ok else None})
            if not all(row["projects_to_navigation"] for row in rows) and nav_wait_checks < 600:
                nav_wait_checks += 1
                if nav_wait_checks % 120 == 1:
                    unreal.WyrmTerrainDiagnostics.refresh_navigation_data_for_actor(terrains[0])
                return
            route_rows = []
            for start, end in zip(ORDER, ORDER[1:]):
                path = unreal.NavigationSystemV1.find_path_to_location_synchronously(
                    game_world, projected[start], projected[end])
                route_rows.append({"from": start, "to": end, "valid": bool(path and path.is_valid()),
                                   "partial": bool(path and path.is_partial()),
                                   "points": len(path.path_points) if path else 0})
            state["anchor_navigation"] = rows
            state["route_navigation"] = route_rows
            found = {
                actor.get_actor_label().removeprefix("DIAG_GLM_"): actor for actor in
                unreal.GameplayStatics.get_all_actors_of_class(game_world, unreal.CameraActor)
                if actor.get_actor_label().startswith("DIAG_GLM_")
            }
            runtime_cameras = [(name, found[name]) for name, _, _ in VIEWS]
            stage = "capture"
            wait_ticks = 30
            return
        if capture_index >= len(runtime_cameras):
            anchors_ok = all(row["projects_to_navigation"] for row in state.get("anchor_navigation", []))
            routes_ok = all(row["valid"] and not row["partial"] for row in state["route_navigation"])
            captures_ok = all(row["requested"] for row in state["screenshots"])
            state["status"] = "PASS_AUTOMATED_REQUIRES_MANUAL_VISUAL_REVIEW" if anchors_ok and routes_ok and captures_ok else "FAIL"
            REPORT.write_text(json.dumps(state, indent=2) + "\n", encoding="utf-8")
            stage = "ending"
            levels.editor_request_end_play()
            return
        name, camera = runtime_cameras[capture_index]
        path = OUT / f"{capture_index + 1:02d}_{name}.png"
        capture_index += 1
        # Screenshot capture pumps Slate and can re-enter this callback. Advance
        # state before requesting the frame so each camera is captured once.
        state["screenshots"].append({"view": name, "path": str(path), "requested": True})
        wait_ticks = 75
        requested = bool(unreal.AutomationLibrary.take_high_res_screenshot(1280, 720, str(path), camera))
        state["screenshots"][-1]["requested"] = requested

    handle = unreal.register_slate_post_tick_callback(tick)
    levels.editor_play_simulate()
    print("WP-23.5 Gloaming foundation PIE proof started")


main()
