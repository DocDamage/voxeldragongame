"""Add the bounded Frank N. Shrine encounter and route anchor without recomposing Gloaming."""
import unreal

MAP = "/Game/WYRMFALL/World/Regions/L_GloamingMarches"
Z = 808.0
levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
if not levels.load_level(MAP):
    raise RuntimeError("Could not load L_GloamingMarches")
existing = {a.get_actor_label(): a for a in actors.get_all_level_actors()}

anchor = existing.get("GLM_ROUTE_FRANK_N_SHRINE")
if not anchor:
    anchor = actors.spawn_actor_from_class(unreal.TargetPoint, unreal.Vector(3950, 4550, Z))
    anchor.set_actor_label("GLM_ROUTE_FRANK_N_SHRINE")
anchor.set_actor_location(unreal.Vector(3950, 4550, Z), False, False)

encounter = existing.get("GLM_ENCOUNTER_FrankNShrine")
if not encounter:
    encounter = actors.spawn_actor_from_class(
        unreal.WyrmFrankNShrineCharacter, unreal.Vector(4150, 4750, Z + 101), unreal.Rotator(0, 0, 135))
    encounter.set_actor_label("GLM_ENCOUNTER_FrankNShrine")
encounter.set_actor_location(unreal.Vector(4150, 4750, Z + 101), False, False)
encounter.set_actor_rotation(unreal.Rotator(0, 0, 135), False)

light = existing.get("GLM_FrankNShrineFill")
if not light:
    light = actors.spawn_actor_from_class(unreal.PointLight, unreal.Vector(4500, 5100, Z + 372))
    light.set_actor_label("GLM_FrankNShrineFill")
light.set_actor_location(unreal.Vector(4500, 5100, Z + 372), False, False)
component = light.get_component_by_class(unreal.PointLightComponent)
component.set_mobility(unreal.ComponentMobility.MOVABLE)
component.set_editor_property("intensity", 30.0)
component.set_editor_property("attenuation_radius", 900.0)
component.set_editor_property("light_color", unreal.Color(95, 145, 90, 255))

if not levels.save_current_level():
    raise RuntimeError("Could not save Frank N. Shrine encounter")
print("WP-23.5 Frank N. Shrine map placement: PASS")
