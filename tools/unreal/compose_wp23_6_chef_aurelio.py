"""Add the bounded optional Chef Aurelio investigation to L_CogspireHarbor."""

import json
from pathlib import Path

import unreal


ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
MAP = "/Game/WYRMFALL/World/Regions/L_CogspireHarbor"
DEST = "/Game/WYRMFALL/Development/Intake/WP23_6"
REPORT = ROOT / "Saved/Diagnostics/WP23_6_chef_aurelio_composition.json"
levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
assets = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)


def spawn_prop(label, asset_path, location, scale=0.8, yaw=0.0):
    asset = assets.load_asset(asset_path)
    if not asset:
        raise RuntimeError(f"Missing supplied tavern asset: {asset_path}")
    actor = actors.spawn_actor_from_object(
        asset, unreal.Vector(*location), unreal.Rotator(0.0, yaw, 0.0))
    if not actor:
        raise RuntimeError(f"Could not spawn {label}")
    actor.set_actor_label("COG_CHEF_" + label)
    actor.set_actor_scale3d(unreal.Vector(scale, scale, scale))
    return actor


if not levels.load_level(MAP):
    raise RuntimeError("Could not load L_CogspireHarbor")

for actor in actors.get_all_level_actors():
    if actor.get_actor_label().startswith("COG_CHEF_"):
        actors.destroy_actor(actor)

spawn_prop("TavernTable", DEST + "/Urban/ChefScene/Table/tavern-48-table.tavern-48-table", (1010, -980, 205))
spawn_prop("MeatShelf", DEST + "/Urban/ChefScene/MeatShelf/tavern-33-shelf_meat.tavern-33-shelf_meat", (1250, -960, 205), yaw=90.0)
spawn_prop("Bar", DEST + "/Urban/ChefScene/Bar/tavern-25-bar.tavern-25-bar", (1120, -1250, 205), yaw=180.0)
spawn_prop("Knife", DEST + "/Urban/ChefScene/Knife/tavern-3-knife.tavern-3-knife", (1040, -990, 235), scale=0.5)

for label, location in (
    ("PatronTestimony", (920, -1080, 205)),
    ("KitchenEvidence", (1210, -1040, 205)),
    ("IngredientSource", (1260, -1180, 205)),
):
    marker = actors.spawn_actor_from_class(unreal.TargetPoint, unreal.Vector(*location))
    marker.set_actor_label("COG_CHEF_" + label)

chef = actors.spawn_actor_from_class(
    unreal.WyrmChefAurelioCharacter, unreal.Vector(1120, -1140, 300), unreal.Rotator(0.0, 0.0, 0.0))
if not chef:
    raise RuntimeError("Could not spawn Chef Aurelio encounter")
chef.set_actor_label("COG_CHEF_Aurelio")

if not levels.save_current_level():
    raise RuntimeError("Failed to save L_CogspireHarbor")

REPORT.parent.mkdir(parents=True, exist_ok=True)
REPORT.write_text(json.dumps({
    "status": "PASS",
    "engine": unreal.SystemLibrary.get_engine_version(),
    "map": MAP,
    "scope": "optional Chef Aurelio investigation fixture",
    "chef_actor": chef.get_actor_label(),
    "supplied_presentation": DEST + "/Roles/ChefAurelioVane_Chef/TVS_VoxelVillage_Chef",
    "investigation_markers": ["PatronTestimony", "KitchenEvidence", "IngredientSource"],
    "supplied_tavern_props": 4,
    "not_a_completion_requirement": True,
}, indent=2) + "\n", encoding="utf-8")
print("WP-23.6 Chef Aurelio map composition: PASS")
