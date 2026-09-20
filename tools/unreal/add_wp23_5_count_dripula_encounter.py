"""Add the bounded Count Dripula encounter and route anchor without recomposing Gloaming."""
import unreal

MAP = "/Game/WYRMFALL/World/Regions/L_GloamingMarches"
Z = 808.0
levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
if not levels.load_level(MAP):
    raise RuntimeError("Could not load L_GloamingMarches")
existing = {a.get_actor_label(): a for a in actors.get_all_level_actors()}

anchor = existing.get("GLM_ROUTE_COUNT_DRIPULA")
if not anchor:
    anchor = actors.spawn_actor_from_class(unreal.TargetPoint, unreal.Vector(3400, 4000, Z))
    anchor.set_actor_label("GLM_ROUTE_COUNT_DRIPULA")
anchor.set_actor_location(unreal.Vector(3400, 4000, Z), False, False)

encounter = existing.get("GLM_ENCOUNTER_CountDripula")
if not encounter:
    encounter = actors.spawn_actor_from_class(
        unreal.WyrmCountDripulaCharacter, unreal.Vector(3600, 4200, Z + 92), unreal.Rotator(0, 0, 135))
    encounter.set_actor_label("GLM_ENCOUNTER_CountDripula")
encounter.set_actor_location(unreal.Vector(3600, 4200, Z + 92), False, False)
encounter.set_actor_rotation(unreal.Rotator(0, 0, 135), False)

light = existing.get("GLM_CountDripulaFill")
if not light:
    light = actors.spawn_actor_from_class(unreal.PointLight, unreal.Vector(3950, 4550, Z + 330))
    light.set_actor_label("GLM_CountDripulaFill")
light.set_actor_location(unreal.Vector(3950, 4550, Z + 330), False, False)
component = light.get_component_by_class(unreal.PointLightComponent)
component.set_mobility(unreal.ComponentMobility.MOVABLE)
component.set_editor_property("intensity", 28.0)
component.set_editor_property("attenuation_radius", 850.0)
component.set_editor_property("light_color", unreal.Color(145, 70, 105, 255))

if not levels.save_current_level():
    raise RuntimeError("Could not save Count Dripula encounter")
print("WP-23.5 Count Dripula map placement: PASS")
