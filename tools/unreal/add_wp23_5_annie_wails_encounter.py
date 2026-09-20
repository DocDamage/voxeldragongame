"""Add the bounded Annie Wails encounter and route anchor without recomposing Gloaming."""
import unreal

MAP = "/Game/WYRMFALL/World/Regions/L_GloamingMarches"
Z = 808.0
levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
if not levels.load_level(MAP):
    raise RuntimeError("Could not load L_GloamingMarches")
existing = {a.get_actor_label(): a for a in actors.get_all_level_actors()}

anchor = existing.get("GLM_ROUTE_ANNIE_WAILS")
if not anchor:
    anchor = actors.spawn_actor_from_class(unreal.TargetPoint, unreal.Vector(2900, 2400, Z))
    anchor.set_actor_label("GLM_ROUTE_ANNIE_WAILS")
anchor.set_actor_location(unreal.Vector(2900, 2400, Z), False, False)

encounter = existing.get("GLM_ENCOUNTER_AnnieWails")
if not encounter:
    encounter = actors.spawn_actor_from_class(
        unreal.WyrmAnnieWailsCharacter, unreal.Vector(2650, 2250, Z + 88), unreal.Rotator(0, 0, 25))
    encounter.set_actor_label("GLM_ENCOUNTER_AnnieWails")
encounter.set_actor_location(unreal.Vector(2650, 2250, Z + 88), False, False)
encounter.set_actor_rotation(unreal.Rotator(0, 0, 25), False)

light = existing.get("GLM_AnnieWailsFill")
if not light:
    light = actors.spawn_actor_from_class(unreal.PointLight, unreal.Vector(2450, 2050, Z + 330))
    light.set_actor_label("GLM_AnnieWailsFill")
light.set_actor_location(unreal.Vector(2450, 2050, Z + 330), False, False)
component = light.get_component_by_class(unreal.PointLightComponent)
component.set_mobility(unreal.ComponentMobility.MOVABLE)
component.set_editor_property("intensity", 32.0)
component.set_editor_property("attenuation_radius", 850.0)
component.set_editor_property("light_color", unreal.Color(165, 120, 115, 255))

if not levels.save_current_level():
    raise RuntimeError("Could not save Annie Wails encounter")
print("WP-23.5 Annie Wails map placement: PASS")
