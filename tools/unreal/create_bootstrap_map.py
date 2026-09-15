"""Run only through Unreal's full-editor -ExecutePythonScript route, after build.
Creates exactly one diagnostic map using engine primitives. Not production art.
Existing map is loaded/inspected, never overwritten. No external content imports.
"""
from pathlib import Path
import json
import os
import hashlib
import unreal

MAP = "/Game/WYRMFALL/Development/Maps/L_DEV_Bootstrap"
ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
RECEIPT = ROOT / "Saved/Diagnostics/bootstrap.json"


def main():
    levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    assets = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
    actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    if assets.does_asset_exist(MAP):
        if not levels.load_level(MAP):
            raise RuntimeError("Existing bootstrap map could not be loaded; left unchanged.")
        action = "EXISTING_MAP_LOADED_UNCHANGED"
    else:
        if not levels.new_level(MAP):
            raise RuntimeError("Unreal could not create the diagnostic map.")
        cube = assets.load_asset("/Engine/BasicShapes/Cube.Cube")
        if not cube:
            raise RuntimeError("Engine diagnostic cube is unavailable; no substitute generated.")
        floor = actors.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(0, 0, -10))
        floor.set_actor_label("DIAGNOSTIC_FLOOR_NOT_VOXEL_TERRAIN")
        mesh = floor.get_component_by_class(unreal.StaticMeshComponent)
        mesh.set_static_mesh(cube)
        mesh.set_collision_profile_name("BlockAll")
        floor.set_actor_scale3d(unreal.Vector(30, 30, 0.2))
        player = actors.spawn_actor_from_class(unreal.PlayerStart, unreal.Vector(0, 0, 130))
        player.set_actor_label("DEV_PlayerStart")
        sun = actors.spawn_actor_from_class(unreal.DirectionalLight, unreal.Vector(0, 0, 700), unreal.Rotator(-45, -30, 0))
        sun.set_actor_label("DEV_DirectionalLight")
        sky = actors.spawn_actor_from_class(unreal.SkyLight, unreal.Vector(0, 0, 400))
        sky.set_actor_label("DEV_SkyLight")
        # Keep the map-specific authority explicit as well as project defaults.
        mode = unreal.load_class(None, "/Script/WYRMFALL.WyrmGameMode")
        if not mode:
            raise RuntimeError("WYRMFALL C++ game mode is not loaded. Compile before bootstrap.")
        world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
        world.get_world_settings().set_editor_property("default_game_mode", mode)
        if not levels.save_current_level():
            raise RuntimeError("Diagnostic level save failed.")
        action = "CREATED_DIAGNOSTIC_MAP"
    # new_level can save an empty file before later setup fails. An existing
    # partial file must not become a success on the next run merely by existing.
    expected = {"DIAGNOSTIC_FLOOR_NOT_VOXEL_TERRAIN": unreal.StaticMeshActor,
                "DEV_PlayerStart": unreal.PlayerStart,
                "DEV_DirectionalLight": unreal.DirectionalLight,
                "DEV_SkyLight": unreal.SkyLight}
    all_actors = actors.get_all_level_actors()
    validated = {}
    for label, actor_type in expected.items():
        matches = [actor for actor in all_actors if actor.get_actor_label() == label]
        if len(matches) != 1 or not isinstance(matches[0], actor_type):
            raise RuntimeError("Diagnostic map needs exactly one correctly typed actor: " + label
                               + ". Left unchanged; inspect before repairing it.")
        validated[label] = matches[0]
    floor_mesh = validated["DIAGNOSTIC_FLOOR_NOT_VOXEL_TERRAIN"].get_component_by_class(unreal.StaticMeshComponent)
    if not floor_mesh or not floor_mesh.get_editor_property("static_mesh"):
        raise RuntimeError("Diagnostic floor has no static mesh; existing map left unchanged.")
    if str(floor_mesh.get_collision_profile_name()) != "BlockAll":
        raise RuntimeError("Diagnostic floor collision profile differs; inspect it before PIE.")
    world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
    mode = unreal.load_class(None, "/Script/WYRMFALL.WyrmGameMode")
    if world.get_world_settings().get_editor_property("default_game_mode") != mode:
        raise RuntimeError("Bootstrap game mode differs; map left unchanged for inspection.")
    file = ROOT / "Content/WYRMFALL/Development/Maps/L_DEV_Bootstrap.umap"
    if not file.is_file() or file.stat().st_size < 64:
        raise RuntimeError("No plausible Unreal map bytes found after editor operation.")
    RECEIPT.parent.mkdir(parents=True, exist_ok=True)
    data = {"status": "EDITOR_MAP_SAVED", "action": action, "map": MAP,
        "engine": unreal.SystemLibrary.get_engine_version(), "map_bytes": file.stat().st_size,
        "map_sha256": hashlib.sha256(file.read_bytes()).hexdigest(), "actor_types_checked": True,
        "pie": "NOT_RUN", "terrain": "NOT_INTEGRATED", "art": "ENGINE_DIAGNOSTIC_PRIMITIVES_ONLY"}
    tmp = RECEIPT.with_suffix(".tmp")
    tmp.write_text(json.dumps(data, indent=2) + "\n", encoding="utf-8")
    os.replace(tmp, RECEIPT)
    unreal.log("WYRMFALL bootstrap receipt: " + str(RECEIPT))

try:
    main()
except Exception as exc:
    unreal.log_error("WYRMFALL bootstrap blocked: " + str(exc))
    raise
