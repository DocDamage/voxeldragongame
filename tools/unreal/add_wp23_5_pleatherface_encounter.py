"""Add the bounded Pleatherface encounter and route anchor."""
import unreal

MAP = "/Game/WYRMFALL/World/Regions/L_GloamingMarches"
Z = 808.0
levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
if not levels.load_level(MAP):
    raise RuntimeError("Could not load L_GloamingMarches")
existing = {a.get_actor_label(): a for a in actors.get_all_level_actors()}

anchor = existing.get("GLM_ROUTE_PLEATHERFACE")
if not anchor:
    anchor = actors.spawn_actor_from_class(unreal.TargetPoint, unreal.Vector(4200, 2250, Z))
    anchor.set_actor_label("GLM_ROUTE_PLEATHERFACE")
anchor.set_actor_location(unreal.Vector(4200, 2250, Z), False, False)

encounter = existing.get("GLM_ENCOUNTER_Pleatherface")
if not encounter:
    encounter = actors.spawn_actor_from_class(
        unreal.WyrmPleatherfaceCharacter, unreal.Vector(4050, 2050, Z + 96), unreal.Rotator(0, 0, 45))
    encounter.set_actor_label("GLM_ENCOUNTER_Pleatherface")
encounter.set_actor_location(unreal.Vector(4050, 2050, Z + 96), False, False)
encounter.set_actor_rotation(unreal.Rotator(0, 0, 45), False)

light = existing.get("GLM_PleatherfaceFill")
if not light:
    light = actors.spawn_actor_from_class(unreal.PointLight, unreal.Vector(3850, 1800, Z + 350))
    light.set_actor_label("GLM_PleatherfaceFill")
light.set_actor_location(unreal.Vector(3850, 1800, Z + 350), False, False)
component = light.get_component_by_class(unreal.PointLightComponent)
component.set_mobility(unreal.ComponentMobility.MOVABLE)
component.set_editor_property("intensity", 22.0)
component.set_editor_property("attenuation_radius", 850.0)
component.set_editor_property("light_color", unreal.Color(145, 105, 90, 255))

if not levels.save_current_level():
    raise RuntimeError("Could not save Pleatherface encounter")
print("WP-23.5 Pleatherface map placement: PASS")
