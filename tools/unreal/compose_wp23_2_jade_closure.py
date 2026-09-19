"""Add the bounded WP-23.2 ruler and authored shove prop to L_JadePeaks."""

import json
from pathlib import Path
import unreal

ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
MAP = "/Game/WYRMFALL/World/Regions/L_JadePeaks"
REPORT = ROOT / "Saved/Diagnostics/WP23_2_jade_map_composition.json"
PRACTICE_PROP_MESH = "/Game/WYRMFALL/Development/Intake/WP22/JadePeaks/Palace/Props/TVS_VoxelPalace_Spear"


def main():
    levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    actor_sub = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    if not levels.load_level(MAP):
        raise RuntimeError(f"Could not load {MAP}")
    for actor in list(actor_sub.get_all_level_actors()):
        if actor.get_actor_label() in ("JADE_EmperorWeiLongzhu", "JADE_UnseenHandPracticeProp"):
            actor_sub.destroy_actor(actor)
    emperor = actor_sub.spawn_actor_from_class(
        unreal.WyrmJadeEmperorCharacter, unreal.Vector(-850, -650, 988), unreal.Rotator(0, 0, -90))
    emperor.set_actor_label("JADE_EmperorWeiLongzhu")
    prop = actor_sub.spawn_actor_from_class(
        unreal.WyrmUnseenHandTarget, unreal.Vector(1900, 650, 920), unreal.Rotator())
    prop.set_actor_label("JADE_UnseenHandPracticeProp")
    practice_prop_mesh = unreal.load_asset(PRACTICE_PROP_MESH)
    if not practice_prop_mesh:
        raise RuntimeError(f"Missing supplied practice prop mesh: {PRACTICE_PROP_MESH}")
    prop.mesh_component.set_static_mesh(practice_prop_mesh)
    prop.set_actor_scale3d(unreal.Vector(1.0, 1.0, 1.0))
    if not levels.save_current_level():
        raise RuntimeError("Failed to save L_JadePeaks")
    REPORT.parent.mkdir(parents=True, exist_ok=True)
    REPORT.write_text(json.dumps({
        "status": "PASS", "map": MAP,
        "emperor": emperor.get_path_name(), "practice_prop": prop.get_path_name(),
        "practice_prop_mesh": PRACTICE_PROP_MESH,
        "king_mesh": "/Game/WYRMFALL/Development/Intake/WP23/JadePeaks/Palace/Characters/TVS_VoxelPalace_King",
    }, indent=2), encoding="utf-8")


if __name__ == "__main__":
    main()
