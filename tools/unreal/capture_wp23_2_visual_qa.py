"""Capture the two WP-23.2 additions in settled Jade Peaks PIE."""

import json
from pathlib import Path

import unreal


ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
MAP = "/Game/WYRMFALL/World/Regions/L_JadePeaks"
OUT = ROOT / "Saved/Diagnostics/WP23_2_VisualQA"
REPORT = ROOT / "Saved/Diagnostics/WP23_2_visual_qa.json"
VIEWS = [
    ("emperor_palace", (-2500, -2350, 1550), (-850, -650, 1150)),
    ("unseen_hand_practice", (3000, -650, 1450), (1900, 650, 1050)),
]


def vector(values):
    return unreal.Vector(*[float(value) for value in values])


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    editor = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
    actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    if not levels.load_level(MAP):
        raise RuntimeError(f"Could not load {MAP}")

    cameras = []
    for name, origin, target in VIEWS:
        camera = actors.spawn_actor_from_class(
            unreal.CameraActor,
            vector(origin),
            unreal.MathLibrary.find_look_at_rotation(vector(origin), vector(target)),
        )
        camera.set_actor_label(f"DIAG_WP23_2_{name}")
        cameras.append((name, camera))

    state = {
        "kind": "wp23_2_visual_qa",
        "engine": unreal.SystemLibrary.get_engine_version(),
        "map": MAP,
        "capture_status": "CAPTURING",
        "qa_status": "UNASSESSED",
        "screenshots": [],
        "measurements": {},
    }
    REPORT.parent.mkdir(parents=True, exist_ok=True)
    REPORT.write_text(json.dumps(state, indent=2) + "\n", encoding="utf-8")

    stage = "wait_pie"
    index = 0
    wait_ticks = 0
    runtime_cameras = []

    def tick(_delta):
        nonlocal stage, index, wait_ticks, runtime_cameras
        if stage == "ending":
            if not levels.is_in_play_in_editor():
                unreal.unregister_slate_post_tick_callback(handle)
                for _, camera in cameras:
                    actors.destroy_actor(camera)
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
            if not terrains:
                return
            stats = terrains[0].get_runtime_render_stats()
            if (stats.get_editor_property("loaded_chunk_count") < 81 or
                    stats.get_editor_property("queued_chunk_generation_count") or
                    stats.get_editor_property("queued_chunk_rebuild_count") or
                    stats.get_editor_property("pending_chunk_apply_count")):
                return
            emperor = unreal.GameplayStatics.get_all_actors_of_class(game_world, unreal.WyrmJadeEmperorCharacter)[0]
            props = unreal.GameplayStatics.get_all_actors_of_class(game_world, unreal.WyrmUnseenHandTarget)
            prop = next(item for item in props if item.get_actor_label() == "JADE_UnseenHandPracticeProp")
            emperor_origin, emperor_extent = emperor.get_actor_bounds(False)
            prop_origin, prop_extent = prop.get_actor_bounds(False)
            emperor_actor_location = emperor.get_actor_location()
            emperor_mesh_location = emperor.mesh.get_editor_property("relative_location")
            state["measurements"] = {
                "emperor": {
                    "actor_location": [emperor_actor_location.x, emperor_actor_location.y, emperor_actor_location.z],
                    "mesh_relative_location": [emperor_mesh_location.x, emperor_mesh_location.y, emperor_mesh_location.z],
                    "location": [emperor_origin.x, emperor_origin.y, emperor_origin.z],
                    "bounds_extent_cm": [emperor_extent.x, emperor_extent.y, emperor_extent.z],
                },
                "practice_prop": {
                    "location": [prop_origin.x, prop_origin.y, prop_origin.z],
                    "bounds_extent_cm": [prop_extent.x, prop_extent.y, prop_extent.z],
                    "mesh": prop.mesh_component.static_mesh.get_path_name(),
                    "simulates_physics": prop.mesh_component.is_simulating_physics(),
                },
            }
            found = {
                item.get_actor_label().removeprefix("DIAG_WP23_2_"): item
                for item in unreal.GameplayStatics.get_all_actors_of_class(game_world, unreal.CameraActor)
                if item.get_actor_label().startswith("DIAG_WP23_2_")
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
        # Screenshot capture can re-enter this callback; record the request
        # before calling it so completion cannot write an empty list.
        state["screenshots"].append({"view": name, "path": str(path), "requested": True})
        wait_ticks = 75
        task = unreal.AutomationLibrary.take_high_res_screenshot(1280, 720, str(path), camera)
        state["screenshots"][-1]["requested"] = bool(task)

    handle = unreal.register_slate_post_tick_callback(tick)
    levels.editor_play_simulate()


main()
