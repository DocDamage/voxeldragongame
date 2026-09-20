"""Add the bounded Chuckles encounter and route anchor without recomposing Gloaming."""
import unreal

MAP = "/Game/WYRMFALL/World/Regions/L_GloamingMarches"
Z = 808.0
levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
if not levels.load_level(MAP):
    raise RuntimeError("Could not load L_GloamingMarches")
existing = {a.get_actor_label(): a for a in actors.get_all_level_actors()}

anchor = existing.get("GLM_ROUTE_CHUCKLES")
if not anchor:
    anchor = actors.spawn_actor_from_class(unreal.TargetPoint, unreal.Vector(2850, 3450, Z))
    anchor.set_actor_label("GLM_ROUTE_CHUCKLES")
anchor.set_actor_location(unreal.Vector(2850, 3450, Z), False, False)

encounter = existing.get("GLM_ENCOUNTER_Chuckles")
if not encounter:
    encounter = actors.spawn_actor_from_class(
        unreal.WyrmChucklesCharacter, unreal.Vector(3050, 3650, Z + 55), unreal.Rotator(0, 0, 135))
    encounter.set_actor_label("GLM_ENCOUNTER_Chuckles")
encounter.set_actor_location(unreal.Vector(3050, 3650, Z + 55), False, False)
encounter.set_actor_rotation(unreal.Rotator(0, 0, 135), False)

light = existing.get("GLM_ChucklesFill")
if not light:
    light = actors.spawn_actor_from_class(unreal.PointLight, unreal.Vector(3350, 3950, Z + 260))
    light.set_actor_label("GLM_ChucklesFill")
light.set_actor_location(unreal.Vector(3350, 3950, Z + 260), False, False)
component = light.get_component_by_class(unreal.PointLightComponent)
component.set_mobility(unreal.ComponentMobility.MOVABLE)
component.set_editor_property("intensity", 24.0)
component.set_editor_property("attenuation_radius", 700.0)
component.set_editor_property("light_color", unreal.Color(185, 120, 75, 255))

if not levels.save_current_level():
    raise RuntimeError("Could not save Chuckles encounter")
print("WP-23.5 Chuckles map placement: PASS")
