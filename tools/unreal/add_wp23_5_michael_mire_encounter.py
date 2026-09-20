"""Add the bounded Michael Mire encounter and route anchor without recomposing Gloaming."""

import unreal


MAP = "/Game/WYRMFALL/World/Regions/L_GloamingMarches"
SURFACE_Z = 808.0

levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
if not levels.load_level(MAP):
    raise RuntimeError("Could not load L_GloamingMarches")

existing = {actor.get_actor_label(): actor for actor in actors.get_all_level_actors()}
anchor_label = "GLM_ROUTE_MICHAEL_MIRE"
anchor = existing.get(anchor_label)
if not anchor:
    anchor = actors.spawn_actor_from_class(
        unreal.TargetPoint, unreal.Vector(4850, 2750, SURFACE_Z))
    anchor.set_actor_label(anchor_label)
anchor.set_actor_location(unreal.Vector(4850, 2750, SURFACE_Z), False, False)

encounter_label = "GLM_ENCOUNTER_MichaelMire"
encounter = existing.get(encounter_label)
if not encounter:
    encounter = actors.spawn_actor_from_class(
        unreal.WyrmMichaelMireCharacter,
        unreal.Vector(4950, 3000, SURFACE_Z + 95), unreal.Rotator(0, 0, 180))
    encounter.set_actor_label(encounter_label)
encounter.set_actor_location(unreal.Vector(4950, 3000, SURFACE_Z + 95), False, False)
encounter.set_actor_rotation(unreal.Rotator(0, 0, 180), False)

light_label = "GLM_MichaelMireFill"
light = existing.get(light_label)
if not light:
    light = actors.spawn_actor_from_class(
        unreal.PointLight, unreal.Vector(5150, 2750, SURFACE_Z + 372))
    light.set_actor_label(light_label)
light.set_actor_location(unreal.Vector(5150, 2750, SURFACE_Z + 372), False, False)
light_component = light.get_component_by_class(unreal.PointLightComponent)
light_component.set_mobility(unreal.ComponentMobility.MOVABLE)
light_component.set_editor_property("intensity", 80.0)
light_component.set_editor_property("attenuation_radius", 1000.0)
light_component.set_editor_property("light_color", unreal.Color(120, 155, 210, 255))

if not levels.save_current_level():
    raise RuntimeError("Could not save Michael Mire encounter")
print("WP-23.5 Michael Mire map placement: PASS")
