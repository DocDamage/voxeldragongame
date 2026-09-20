"""Add the bounded Wherewolf encounter and route anchor without recomposing Gloaming."""
import unreal

MAP = "/Game/WYRMFALL/World/Regions/L_GloamingMarches"
Z = 808.0
levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
if not levels.load_level(MAP):
    raise RuntimeError("Could not load L_GloamingMarches")
existing = {a.get_actor_label(): a for a in actors.get_all_level_actors()}

anchor = existing.get("GLM_ROUTE_WHEREWOLF")
if not anchor:
    anchor = actors.spawn_actor_from_class(unreal.TargetPoint, unreal.Vector(3500, 2750, Z))
    anchor.set_actor_label("GLM_ROUTE_WHEREWOLF")
anchor.set_actor_location(unreal.Vector(3500, 2750, Z), False, False)

encounter = existing.get("GLM_ENCOUNTER_Wherewolf")
if not encounter:
    encounter = actors.spawn_actor_from_class(
        unreal.WyrmWherewolfCharacter, unreal.Vector(3300, 2900, Z + 102.5), unreal.Rotator(0, 0, -35))
    encounter.set_actor_label("GLM_ENCOUNTER_Wherewolf")
encounter.set_actor_location(unreal.Vector(3300, 2900, Z + 102.5), False, False)
encounter.set_actor_rotation(unreal.Rotator(0, 0, -35), False)

light = existing.get("GLM_WherewolfFill")
if not light:
    light = actors.spawn_actor_from_class(unreal.PointLight, unreal.Vector(3150, 2700, Z + 375))
    light.set_actor_label("GLM_WherewolfFill")
light.set_actor_location(unreal.Vector(3150, 2700, Z + 375), False, False)
component = light.get_component_by_class(unreal.PointLightComponent)
component.set_mobility(unreal.ComponentMobility.MOVABLE)
component.set_editor_property("intensity", 28.0)
component.set_editor_property("attenuation_radius", 900.0)
component.set_editor_property("light_color", unreal.Color(105, 135, 175, 255))

if not levels.save_current_level():
    raise RuntimeError("Could not save Wherewolf encounter")
print("WP-23.5 Wherewolf map placement: PASS")
