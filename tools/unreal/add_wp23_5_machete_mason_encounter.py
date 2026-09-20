"""Add the bounded Machete Mason encounter and route anchor without recomposing Gloaming."""

import unreal


MAP = "/Game/WYRMFALL/World/Regions/L_GloamingMarches"
SURFACE_Z = 808.0

levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
if not levels.load_level(MAP):
    raise RuntimeError("Could not load L_GloamingMarches")

existing = {actor.get_actor_label(): actor for actor in actors.get_all_level_actors()}
anchor_label = "GLM_ROUTE_MACHETE_MASON"
anchor = existing.get(anchor_label)
if not anchor:
    anchor = actors.spawn_actor_from_class(
        unreal.TargetPoint, unreal.Vector(4750, 2350, SURFACE_Z))
    anchor.set_actor_label(anchor_label)
anchor.set_actor_location(unreal.Vector(4750, 2350, SURFACE_Z), False, False)

encounter_label = "GLM_ENCOUNTER_MacheteMason"
encounter = existing.get(encounter_label)
if not encounter:
    encounter = actors.spawn_actor_from_class(
        unreal.WyrmMacheteMasonCharacter,
        unreal.Vector(4750, 2150, SURFACE_Z + 94), unreal.Rotator(0, 0, 225))
    encounter.set_actor_label(encounter_label)
encounter.set_actor_location(unreal.Vector(4750, 2150, SURFACE_Z + 94), False, False)
encounter.set_actor_rotation(unreal.Rotator(0, 0, 225), False)

light_label = "GLM_MacheteMasonFill"
light = existing.get(light_label)
if not light:
    light = actors.spawn_actor_from_class(
        unreal.PointLight, unreal.Vector(4950, 1950, SURFACE_Z + 340))
    light.set_actor_label(light_label)
light.set_actor_location(unreal.Vector(4950, 1950, SURFACE_Z + 340), False, False)
light_component = light.get_component_by_class(unreal.PointLightComponent)
light_component.set_mobility(unreal.ComponentMobility.MOVABLE)
light_component.set_editor_property("intensity", 25.0)
light_component.set_editor_property("attenuation_radius", 900.0)
light_component.set_editor_property("light_color", unreal.Color(175, 115, 90, 255))

if not levels.save_current_level():
    raise RuntimeError("Could not save Machete Mason encounter")
print("WP-23.5 Machete Mason map placement: PASS")
