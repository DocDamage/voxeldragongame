"""Add the bounded Scarrie encounter and route anchor without recomposing Gloaming."""
import unreal

MAP = "/Game/WYRMFALL/World/Regions/L_GloamingMarches"
Z = 808.0
levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
if not levels.load_level(MAP):
    raise RuntimeError("Could not load L_GloamingMarches")
existing = {a.get_actor_label(): a for a in actors.get_all_level_actors()}

anchor = existing.get("GLM_ROUTE_SCARRIE")
if not anchor:
    anchor = actors.spawn_actor_from_class(unreal.TargetPoint, unreal.Vector(2350, 2900, Z))
    anchor.set_actor_label("GLM_ROUTE_SCARRIE")
anchor.set_actor_location(unreal.Vector(2350, 2900, Z), False, False)

encounter = existing.get("GLM_ENCOUNTER_Scarrie")
if not encounter:
    encounter = actors.spawn_actor_from_class(
        unreal.WyrmScarrieCharacter, unreal.Vector(2100, 3100, Z + 87), unreal.Rotator(0, 0, -20))
    encounter.set_actor_label("GLM_ENCOUNTER_Scarrie")
encounter.set_actor_location(unreal.Vector(2100, 3100, Z + 87), False, False)
encounter.set_actor_rotation(unreal.Rotator(0, 0, -20), False)

light = existing.get("GLM_ScarrieFill")
if not light:
    light = actors.spawn_actor_from_class(unreal.PointLight, unreal.Vector(2350, 3350, Z + 320))
    light.set_actor_label("GLM_ScarrieFill")
light.set_actor_location(unreal.Vector(2350, 3350, Z + 320), False, False)
component = light.get_component_by_class(unreal.PointLightComponent)
component.set_mobility(unreal.ComponentMobility.MOVABLE)
component.set_editor_property("intensity", 30.0)
component.set_editor_property("attenuation_radius", 850.0)
component.set_editor_property("light_color", unreal.Color(135, 105, 155, 255))

if not levels.save_current_level():
    raise RuntimeError("Could not save Scarrie encounter")
print("WP-23.5 Scarrie map placement: PASS")
