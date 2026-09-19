"""Capture reproducible editor-viewport frames and measurements for WP-22 visual QA."""

import json
import time
from pathlib import Path

import unreal


ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
MAP = "/Game/WYRMFALL/World/Regions/L_JadePeaks"
OUT = ROOT / "Saved/Diagnostics/WP22_VisualQA"
REPORT = ROOT / "Saved/Diagnostics/WP22_visual_qa.json"

VIEWS = [
    ("arrival_to_palace", (-5400, -3900, 1550), (0, 600, 1250)),
    ("palace_approach", (-2850, -2450, 1450), (0, 600, 1250)),
    ("mirror_court", (2800, -900, 1450), (2000, 300, 975)),
    ("stormcourt_to_aerie", (2850, -450, 1500), (4100, 1750, 1080)),
    ("route_overview", (-6500, -6500, 9000), (400, -300, 900)),
]


def look_rotation(origin, target):
    return unreal.MathLibrary.find_look_at_rotation(vec(origin), vec(target))


def vec(values):
    return unreal.Vector(float(values[0]), float(values[1]), float(values[2]))


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    editor = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
    actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    if not levels.load_level(MAP):
        raise RuntimeError(f"Could not load {MAP}")
    world = editor.get_editor_world()

    rows = []
    for actor in actor_subsystem.get_all_level_actors():
        if not isinstance(actor, unreal.StaticMeshActor):
            continue
        mesh = actor.static_mesh_component.static_mesh
        if not mesh:
            continue
        origin, extent = actor.get_actor_bounds(False)
        rows.append({
            "label": actor.get_actor_label(),
            "asset": mesh.get_path_name(),
            "location": [actor.get_actor_location().x, actor.get_actor_location().y, actor.get_actor_location().z],
            "bounds_extent_cm": [extent.x, extent.y, extent.z],
            "bounds_bottom_z": origin.z - extent.z,
            "bounds_top_z": origin.z + extent.z,
        })

    cameras = []
    for name, origin, target in VIEWS:
        camera = actor_subsystem.spawn_actor_from_class(unreal.CameraActor, vec(origin), look_rotation(origin, target))
        camera.set_actor_label(f"DIAG_WP22_QA_{name}")
        cameras.append((name, camera))

    editor_nav = unreal.NavigationSystemV1.get_navigation_system(world)
    nav_bounds = [actor for actor in actor_subsystem.get_all_level_actors()
                  if isinstance(actor, unreal.NavMeshBoundsVolume)]
    if not editor_nav or not nav_bounds:
        raise RuntimeError("Jade Peaks visual QA requires navigation system and saved bounds")
    proof_nav_bound = actor_subsystem.spawn_actor_from_class(
        unreal.NavMeshBoundsVolume, unreal.Vector(0, 0, 900))
    proof_nav_bound.set_actor_scale3d(unreal.Vector(100, 100, 30))
    proof_nav_bound.set_actor_label("DIAG_WP22_QA_NavRegistration")
    nav_bounds.append(proof_nav_bound)
    for nav_bound in nav_bounds:
        editor_nav.on_navigation_bounds_updated(nav_bound)

    state = {
        "kind": "wp22_visual_editor_qa",
        "engine": unreal.SystemLibrary.get_engine_version(),
        "map": MAP,
        "capture_status": "CAPTURING",
        "qa_status": "UNASSESSED",
        "screenshots": [],
        "static_mesh_placements": rows,
        "landmark_navigation": [],
        "route_navigation": [],
        "notes": [
            "Frames are deterministic Unreal PIE camera captures, not an interactive human playthrough.",
            "GeoForge terrain and navigation were allowed to settle before capture.",
        ],
    }
    REPORT.parent.mkdir(parents=True, exist_ok=True)
    REPORT.write_text(json.dumps(state, indent=2) + "\n", encoding="utf-8")

    index = 0
    wait_ticks = 0
    nav_wait_checks = 0
    stage = "wait_pie"
    runtime_cameras = []

    def tick(_delta):
        nonlocal index, wait_ticks, nav_wait_checks, stage, runtime_cameras
        if stage == "ending":
            if not levels.is_in_play_in_editor():
                unreal.unregister_slate_post_tick_callback(handle)
                for _, camera in cameras:
                    actor_subsystem.destroy_actor(camera)
                actor_subsystem.destroy_actor(proof_nav_bound)
                unreal.SystemLibrary.quit_editor()
            return
        if wait_ticks > 0:
            wait_ticks -= 1
            return
        game_world = editor.get_game_world()
        if stage == "wait_pie":
            if not game_world or not levels.is_in_play_in_editor():
                return
            terrains = unreal.GameplayStatics.get_all_actors_of_class(game_world, unreal.GeoForgeInfiniteTerrainActor)
            if not terrains:
                return
            stats = terrains[0].get_runtime_render_stats()
            if (stats.get_editor_property("loaded_chunk_count") < 81 or
                    stats.get_editor_property("queued_chunk_generation_count") or
                    stats.get_editor_property("queued_chunk_rebuild_count") or
                    stats.get_editor_property("pending_chunk_apply_count")):
                return
            if unreal.WyrmTerrainDiagnostics.is_navigation_build_pending(terrains[0]):
                return
            nav_rows = []
            for actor in unreal.GameplayStatics.get_all_actors_of_class(game_world, unreal.WyrmJadePeaksLandmark):
                location = actor.get_actor_location()
                nav_result = unreal.WyrmTerrainDiagnostics.project_navigation_point(
                    actor, location, unreal.Vector(150, 150, 400))
                if isinstance(nav_result, (tuple, list)):
                    ok = bool(nav_result[0])
                    projected = next((item for item in nav_result[1:] if isinstance(item, unreal.Vector)), unreal.Vector())
                elif isinstance(nav_result, unreal.Vector):
                    ok = True
                    projected = nav_result
                else:
                    ok = bool(nav_result)
                    projected = unreal.Vector()
                nav_rows.append({
                    "label": actor.get_actor_label(),
                    "location": [location.x, location.y, location.z],
                    "projects_to_navigation": bool(ok),
                    "projected": [projected.x, projected.y, projected.z] if ok else None,
                })
            state["landmark_navigation"] = sorted(nav_rows, key=lambda row: row["label"])
            if not all(row["projects_to_navigation"] for row in nav_rows) and nav_wait_checks < 600:
                nav_wait_checks += 1
                if nav_wait_checks % 120 == 1:
                    unreal.WyrmTerrainDiagnostics.refresh_navigation_data_for_actor(terrains[0])
                return
            ordered_ids = [
                "Landmark_LM-JADE-ARRIVAL", "Landmark_LM-JADE-PALACE",
                "Landmark_LM-JADE-STORMCOURT", "Landmark_LM-JADE-MIRRORWELL",
                "Landmark_LM-JADE-AERIE",
            ]
            by_label = {actor.get_actor_label(): actor for actor in
                        unreal.GameplayStatics.get_all_actors_of_class(game_world, unreal.WyrmJadePeaksLandmark)}
            route_rows = []
            for start_label, end_label in zip(ordered_ids, ordered_ids[1:]):
                path = unreal.NavigationSystemV1.find_path_to_location_synchronously(
                    game_world, by_label[start_label].get_actor_location(),
                    by_label[end_label].get_actor_location())
                route_rows.append({
                    "from": start_label.removeprefix("Landmark_"),
                    "to": end_label.removeprefix("Landmark_"),
                    "valid": bool(path and path.is_valid()),
                    "partial": bool(path and path.is_partial()),
                    "points": len(path.path_points) if path else 0,
                })
            state["route_navigation"] = route_rows
            found = {
                actor.get_actor_label().removeprefix("DIAG_WP22_QA_"): actor
                for actor in unreal.GameplayStatics.get_all_actors_of_class(game_world, unreal.CameraActor)
                if actor.get_actor_label().startswith("DIAG_WP22_QA_")
            }
            runtime_cameras = [(name, found[name]) for name, _, _ in VIEWS]
            stage = "capture"
            wait_ticks = 30
            return
        if index >= len(runtime_cameras):
            state["capture_status"] = "PASS"
            REPORT.write_text(json.dumps(state, indent=2) + "\n", encoding="utf-8")
            stage = "ending"
            levels.editor_request_end_play()
            return
        name, camera = runtime_cameras[index]
        path = OUT / f"{index + 1:02d}_{name}.png"
        index += 1
        # Screenshot capture pumps Slate and can re-enter this callback. Advance
        # first so every authored camera is requested exactly once.
        state["screenshots"].append({"view": name, "path": str(path), "requested": True})
        wait_ticks = 75
        task = unreal.AutomationLibrary.take_high_res_screenshot(1280, 720, str(path), camera)
        state["screenshots"][-1]["requested"] = bool(task)

    handle = unreal.register_slate_post_tick_callback(tick)
    levels.editor_play_simulate()
    print(f"WP-22 visual QA capture started at {time.strftime('%H:%M:%S')}")


main()
