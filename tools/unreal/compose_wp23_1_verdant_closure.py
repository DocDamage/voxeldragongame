"""Add the bounded Verdant Reach closure branch to the existing L_Region01 map."""

import json
from pathlib import Path
import unreal

ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
MAP = "/Game/WYRMFALL/World/Regions/L_Region01"
DEST = "/Game/WYRMFALL/Development/Intake/WP23_1/VerdantReach/Environment"
GROUND_MATERIAL = "/Game/WYRMFALL/World/Regions/Materials/M_VerdantReachGround"
REPORT = ROOT / "Saved/Diagnostics/WP23_1_verdant_map_composition.json"


def ensure_ground_material(asset_sub):
    material = asset_sub.load_asset(GROUND_MATERIAL) if asset_sub.does_asset_exist(GROUND_MATERIAL) else None
    if material:
        return material
    tools = unreal.AssetToolsHelpers.get_asset_tools()
    material = tools.create_asset(
        "M_VerdantReachGround", "/Game/WYRMFALL/World/Regions/Materials",
        unreal.Material, unreal.MaterialFactoryNew())
    if not material:
        raise RuntimeError("Could not create the Verdant Reach ground material")
    editing = unreal.MaterialEditingLibrary
    color = editing.create_material_expression(material, unreal.MaterialExpressionVectorParameter, -300, 0)
    color.set_editor_property("parameter_name", "GroundColor")
    color.set_editor_property("default_value", unreal.LinearColor(0.10, 0.24, 0.09, 1.0))
    editing.connect_material_property(color, "RGB", unreal.MaterialProperty.MP_BASE_COLOR)
    roughness = editing.create_material_expression(material, unreal.MaterialExpressionConstant, -300, 180)
    roughness.set_editor_property("r", 0.94)
    editing.connect_material_property(roughness, "", unreal.MaterialProperty.MP_ROUGHNESS)
    editing.recompile_material(material)
    if not asset_sub.save_loaded_asset(material):
        raise RuntimeError("Could not save the Verdant Reach ground material")
    return material


def main():
    levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    actor_sub = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    asset_sub = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
    if not levels.load_level(MAP):
        raise RuntimeError(f"Could not load {MAP}")

    prefixes = ("VERDANT_", "Landmark_LM-CANOPY")
    for actor in list(actor_sub.get_all_level_actors()):
        if actor.get_actor_label().startswith(prefixes):
            actor_sub.destroy_actor(actor)

    ground_material = ensure_ground_material(asset_sub)
    terrains = [a for a in actor_sub.get_all_level_actors() if isinstance(a, unreal.GeoForgeInfiniteTerrainActor)]
    if len(terrains) != 1:
        raise RuntimeError(f"Expected one Region 01 terrain actor, found {len(terrains)}")
    for key in ("terrain_material", "surface_material", "soil_material", "rock_material",
                "deep_rock_material", "sand_material", "snow_material"):
        terrains[0].set_editor_property(key, ground_material)
    terrains[0].set_editor_property("use_general_biome_terrain_material", True)
    terrains[0].set_editor_property("general_biome_terrain_material", ground_material)

    for actor in actor_sub.get_all_level_actors():
        if isinstance(actor, unreal.DirectionalLight):
            component = actor.get_component_by_class(unreal.DirectionalLightComponent)
            component.set_mobility(unreal.ComponentMobility.MOVABLE)
            component.set_editor_property("intensity", 1.35)
            component.set_editor_property("light_color", unreal.Color(255, 238, 218, 255))
        elif isinstance(actor, unreal.SkyLight):
            component = actor.get_component_by_class(unreal.SkyLightComponent)
            component.set_mobility(unreal.ComponentMobility.MOVABLE)
            component.set_editor_property("intensity", 0.75)
            component.set_editor_property("real_time_capture", True)

    landmarks = [
        ("LM-CANOPYENTRY", unreal.Vector(-450, 1650, 800)),
        ("LM-CANOPYHUNT", unreal.Vector(1050, 3000, 800)),
        ("LM-TIDECALLEROUTPOST", unreal.Vector(-1650, 2850, 800)),
    ]
    for lm_id, location in landmarks:
        actor = actor_sub.spawn_actor_from_class(unreal.WyrmRegion01Landmark, location)
        actor.set_actor_label(f"Landmark_{lm_id}")
        actor.set_editor_property("LandmarkId", unreal.Name(lm_id))
        actor.set_editor_property("TriggerRadius", 400.0)

    meridess = actor_sub.spawn_actor_from_class(
        unreal.WyrmQueenMeridessCharacter, unreal.Vector(-1600, 2700, 1500), unreal.Rotator(0, 0, -90))
    meridess.set_actor_label("VERDANT_QueenMeridess")
    guard = actor_sub.spawn_actor_from_class(
        unreal.WyrmVerdantRoyalGuardCharacter, unreal.Vector(-1300, 2600, 1500), unreal.Rotator(0, 0, -120))
    guard.set_actor_label("VERDANT_RoyalGuard")
    hunter = actor_sub.spawn_actor_from_class(
        unreal.WyrmCanopyHunterCharacter, unreal.Vector(1050, 3000, 1500), unreal.Rotator(0, 0, 135))
    hunter.set_actor_label("VERDANT_CanopyHunter")

    scenery = [
        ("Tower", "SM_RangerTower", unreal.Vector(-2050, 3400, 800), unreal.Rotator(0, 0, 35), unreal.Vector(1, 1, 1)),
        ("Tent", "SM_RangerTent", unreal.Vector(-1150, 3300, 800), unreal.Rotator(0, 0, -25), unreal.Vector(1, 1, 1)),
        ("Banner", "SM_RangerBanner", unreal.Vector(-1600, 3200, 800), unreal.Rotator(0, 0, 0), unreal.Vector(1, 1, 1)),
        ("WallA", "SM_RangerWall", unreal.Vector(-2050, 2700, 800), unreal.Rotator(0, 0, 20), unreal.Vector(1, 1, 1)),
        ("WallB", "SM_RangerWall", unreal.Vector(-1050, 2850, 800), unreal.Rotator(0, 0, 100), unreal.Vector(1, 1, 1)),
        ("Campfire", "SM_RangerCampfire", unreal.Vector(-1450, 3000, 800), unreal.Rotator(), unreal.Vector(1, 1, 1)),
        ("BushEntryA", "SM_RangerBush", unreal.Vector(-200, 1850, 800), unreal.Rotator(0, 0, 15), unreal.Vector(1.2, 1.2, 1.2)),
        ("BushEntryB", "SM_RangerBush", unreal.Vector(-700, 2000, 800), unreal.Rotator(0, 0, -30), unreal.Vector(1.3, 1.3, 1.3)),
        ("BushHuntA", "SM_RangerBush", unreal.Vector(800, 2750, 800), unreal.Rotator(0, 0, 20), unreal.Vector(1.5, 1.5, 1.5)),
        ("BushHuntB", "SM_RangerBush", unreal.Vector(1300, 3200, 800), unreal.Rotator(0, 0, -40), unreal.Vector(1.4, 1.4, 1.4)),
        ("EntryBannerWest", "SM_RangerBanner", unreal.Vector(-700, 1650, 800), unreal.Rotator(0, 0, 90), unreal.Vector(0.8, 0.8, 0.8)),
        ("EntryBannerEast", "SM_RangerBanner", unreal.Vector(-200, 1650, 800), unreal.Rotator(0, 0, 90), unreal.Vector(0.8, 0.8, 0.8)),
        ("OutpostTrailBanner", "SM_RangerBanner", unreal.Vector(-950, 2300, 800), unreal.Rotator(0, 0, 45), unreal.Vector(0.7, 0.7, 0.7)),
        ("HunterTrailBanner", "SM_RangerBanner", unreal.Vector(350, 2450, 800), unreal.Rotator(0, 0, -35), unreal.Vector(0.7, 0.7, 0.7)),
    ]
    for label, mesh_name, location, rotation, scale in scenery:
        mesh = unreal.load_asset(f"{DEST}/{mesh_name}")
        if not mesh:
            raise RuntimeError(f"Missing supplied ranger environment mesh {mesh_name}")
        actor = actor_sub.spawn_actor_from_class(unreal.StaticMeshActor, location, rotation)
        actor.set_actor_label(f"VERDANT_{label}")
        actor.static_mesh_component.set_static_mesh(mesh)
        actor.static_mesh_component.set_collision_profile_name("BlockAll")
        actor.set_actor_scale3d(scale)

    if not levels.save_current_level():
        raise RuntimeError("Failed to save L_Region01")
    REPORT.parent.mkdir(parents=True, exist_ok=True)
    REPORT.write_text(json.dumps({
        "status": "PASS", "map": MAP, "landmarks": [x[0] for x in landmarks],
        "actors": [meridess.get_path_name(), guard.get_path_name(), hunter.get_path_name()],
        "supplied_scenery_count": len(scenery),
        "terrain_material": ground_material.get_path_name(),
        "authored_surface_z": 800.0,
    }, indent=2), encoding="utf-8")


if __name__ == "__main__":
    main()
