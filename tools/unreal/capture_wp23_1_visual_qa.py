"""Capture the bounded WP-23.1 branch in settled Region 01 PIE."""

import json
from pathlib import Path
import unreal

ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
MAP = "/Game/WYRMFALL/World/Regions/L_Region01"
OUT = ROOT / "Saved/Diagnostics/WP23_1_VisualQA"
REPORT = ROOT / "Saved/Diagnostics/WP23_1_visual_qa.json"
VIEWS = [
    ("canopy_entry_route", (-100, 800, 1500), (-450, 1850, 850)),
    ("tidecaller_outpost", (-2700, 1750, 1500), (-1500, 2850, 850)),
    ("canopy_hunter", (1850, 2150, 1400), (1050, 3000, 850)),
    ("branch_overview", (0, 850, 3000), (-350, 2500, 800)),
]


def vec(values):
    return unreal.Vector(*[float(value) for value in values])


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    editor = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
    actor_sub = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    if not levels.load_level(MAP):
        raise RuntimeError(f"Could not load {MAP}")

    cameras = []
    for name, origin, target in VIEWS:
        camera = actor_sub.spawn_actor_from_class(
            unreal.CameraActor, vec(origin), unreal.MathLibrary.find_look_at_rotation(vec(origin), vec(target)))
        camera.set_actor_label(f"DIAG_WP23_1_{name}")
        cameras.append((name, camera))

    state = {
        "kind": "wp23_1_visual_qa", "engine": unreal.SystemLibrary.get_engine_version(),
        "map": MAP, "capture_status": "CAPTURING", "qa_status": "UNASSESSED",
        "screenshots": [], "measurements": {},
    }
    REPORT.parent.mkdir(parents=True, exist_ok=True)
    REPORT.write_text(json.dumps(state, indent=2) + "\n", encoding="utf-8")
    stage, index, wait_ticks, runtime_cameras, settle_frames = "wait_pie", 0, 0, [], 0

    def tick(_delta):
        nonlocal stage, index, wait_ticks, runtime_cameras, settle_frames
        if stage == "ending":
            if not levels.is_in_play_in_editor():
                unreal.unregister_slate_post_tick_callback(handle)
                for _, camera in cameras:
                    actor_sub.destroy_actor(camera)
                unreal.SystemLibrary.quit_editor()
            return
        if wait_ticks:
            wait_ticks -= 1
            return
        world = editor.get_game_world()
        if stage == "wait_pie":
            if not world or not levels.is_in_play_in_editor():
                return
            terrains = unreal.GameplayStatics.get_all_actors_of_class(world, unreal.GeoForgeInfiniteTerrainActor)
            if not terrains:
                return
            stats = terrains[0].get_runtime_render_stats()
            if (stats.get_editor_property("loaded_chunk_count") < 49 or
                    stats.get_editor_property("queued_chunk_generation_count") or
                    stats.get_editor_property("queued_chunk_rebuild_count") or
                    stats.get_editor_property("pending_chunk_apply_count")):
                return
            settle_frames += 1
            if settle_frames < 180:
                return
            named = {a.get_actor_label(): a for a in unreal.GameplayStatics.get_all_actors_of_class(world, unreal.Actor)}
            measured = {}
            for label in ("VERDANT_QueenMeridess", "VERDANT_RoyalGuard", "VERDANT_CanopyHunter",
                          "VERDANT_Tower", "VERDANT_Tent", "VERDANT_Banner", "VERDANT_Campfire"):
                actor = named[label]
                origin, extent = actor.get_actor_bounds(False)
                measured[label] = {
                    "location": [origin.x, origin.y, origin.z],
                    "bounds_extent_cm": [extent.x, extent.y, extent.z],
                }
            state["measurements"] = measured
            found = {a.get_actor_label().removeprefix("DIAG_WP23_1_"): a
                     for a in unreal.GameplayStatics.get_all_actors_of_class(world, unreal.CameraActor)
                     if a.get_actor_label().startswith("DIAG_WP23_1_")}
            runtime_cameras = [(name, found[name]) for name, _, _ in VIEWS]
            stage, wait_ticks = "capture", 30
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
        state["screenshots"].append({"view": name, "path": str(path), "requested": True})
        wait_ticks = 75
        state["screenshots"][-1]["requested"] = bool(
            unreal.AutomationLibrary.take_high_res_screenshot(1280, 720, str(path), camera))

    handle = unreal.register_slate_post_tick_callback(tick)
    levels.editor_play_simulate()


main()
