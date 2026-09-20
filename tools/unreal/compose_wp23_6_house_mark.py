"""Add the bounded optional House Mark investigation to L_CogspireHarbor."""

import json
from pathlib import Path

import unreal


ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
MAP = "/Game/WYRMFALL/World/Regions/L_CogspireHarbor"
DEST = "/Game/WYRMFALL/Development/Intake/WP23_6"
REPORT = ROOT / "Saved/Diagnostics/WP23_6_house_mark_composition.json"
levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
assets = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)


def spawn_prop(label, asset_path, location, scale=0.8, yaw=0.0):
    asset = assets.load_asset(asset_path)
    if not asset:
        raise RuntimeError(f"Missing supplied casino asset: {asset_path}")
    actor = actors.spawn_actor_from_object(
        asset, unreal.Vector(*location), unreal.Rotator(0.0, yaw, 0.0))
    if not actor:
        raise RuntimeError(f"Could not spawn {label}")
    actor.set_actor_label("COG_HOUSEMARK_" + label)
    actor.set_actor_scale3d(unreal.Vector(scale, scale, scale))
    return actor


if not levels.load_level(MAP):
    raise RuntimeError("Could not load L_CogspireHarbor")

for actor in actors.get_all_level_actors():
    if actor.get_actor_label().startswith("COG_HOUSEMARK_"):
        actors.destroy_actor(actor)

spawn_prop("CasinoPokerTable", DEST + "/Urban/Casino/Poker_Table-9_00.Poker_Table-9_00", (-850, 980, 205))
spawn_prop("CasinoSlotMachine", DEST + "/Urban/Casino/SlotMachine_Red-13_00.SlotMachine_Red-13_00", (-1120, 980, 205))
spawn_prop("CasinoBarrier", DEST + "/Urban/Casino/BeltBarriers_Red-10_00.BeltBarriers_Red-10_00", (-980, 760, 205))

for label, location in (
    ("VictimEvidence", (-1110, 870, 205)),
    ("RelicTradeEvidence", (-820, 870, 205)),
):
    marker = actors.spawn_actor_from_class(unreal.TargetPoint, unreal.Vector(*location))
    marker.set_actor_label("COG_HOUSEMARK_" + label)

house_mark = actors.spawn_actor_from_class(
    unreal.WyrmHouseMarkCharacter, unreal.Vector(-960, 1120, 300), unreal.Rotator(0.0, 180.0, 0.0))
if not house_mark:
    raise RuntimeError("Could not spawn House Mark encounter")
house_mark.set_actor_label("COG_HOUSEMARK_Champion")

if not levels.save_current_level():
    raise RuntimeError("Failed to save L_CogspireHarbor")

REPORT.parent.mkdir(parents=True, exist_ok=True)
REPORT.write_text(json.dumps({
    "status": "PASS",
    "engine": unreal.SystemLibrary.get_engine_version(),
    "map": MAP,
    "scope": "optional House Mark investigation fixture",
    "house_mark_actor": house_mark.get_actor_label(),
    "supplied_presentation": DEST + "/Roles/HouseMark_Champion/TVS_VoxelKnights_Champion",
    "investigation_markers": ["VictimEvidence", "RelicTradeEvidence"],
    "supplied_casino_props": 3,
    "not_a_completion_requirement": True,
}, indent=2) + "\n", encoding="utf-8")
print("WP-23.6 House Mark map composition: PASS")
