"""Place the complete remaining required-horror roster in L_GloamingMarches."""
import unreal

MAP = "/Game/WYRMFALL/World/Regions/L_GloamingMarches"
Z = 808.0
levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
if not levels.load_level(MAP):
    raise RuntimeError("Could not load L_GloamingMarches")
existing = {a.get_actor_label(): a for a in actors.get_all_level_actors()}

roster = [
    ("AIL_YEN", "AilYen", unreal.WyrmAilYenCharacter, (4500, 5000), (4650, 5150), 107.0),
    ("BELLRAISER", "Bellraiser", unreal.WyrmBellraiserCharacter, (5000, 5000), (5150, 4850), 93.0),
    ("SAD_ECHO", "SadEcho", unreal.WyrmSadEchoCharacter, (5400, 4500), (5550, 4650), 84.0),
    ("DREADATOR", "Dreadator", unreal.WyrmDreadatorCharacter, (5600, 3900), (5750, 4050), 104.0),
    ("ROASTFACE", "Roastface", unreal.WyrmRoastfaceCharacter, (5600, 3300), (5750, 3450), 91.0),
    ("GRAVY_DAUGHTERS", "GravyDaughtersPrimary", unreal.WyrmGravyDaughtersCharacter, (5600, 2700), (5750, 2825), 74.0),
    ("KNIT", "Knit", unreal.WyrmKnitCharacter, (5600, 2100), (5750, 2250), 89.0),
    ("CANNIBALL", "Canniball", unreal.WyrmCanniballCharacter, (5300, 1500), (5450, 1650), 90.0),
    ("MUMS_THE_WYRD", "MumsTheWyrd", unreal.WyrmMumsTheWyrdCharacter, (4700, 1200), (4850, 1350), 98.0),
    ("DREADY_FREDDIE", "DreadyFreddie", unreal.WyrmDreadyFreddieCharacter, (4000, 1100), (4150, 1250), 92.0),
    ("PYRE_MIDHEAD", "PyreMidhead", unreal.WyrmPyreMidheadCharacter, (3300, 1200), (3450, 1350), 110.0),
]
for index, (key, label, cls, anchor_xy, actor_xy, half_height) in enumerate(roster):
    anchor_label = f"GLM_ROUTE_{key}"
    anchor = existing.get(anchor_label)
    if not anchor:
        anchor = actors.spawn_actor_from_class(unreal.TargetPoint, unreal.Vector(anchor_xy[0], anchor_xy[1], Z))
        anchor.set_actor_label(anchor_label)
    anchor.set_actor_location(unreal.Vector(anchor_xy[0], anchor_xy[1], Z), False, False)

    actor_label = f"GLM_ENCOUNTER_{label}"
    encounter = existing.get(actor_label)
    if not encounter:
        encounter = actors.spawn_actor_from_class(cls, unreal.Vector(actor_xy[0], actor_xy[1], Z + half_height), unreal.Rotator(0, 0, 180 + index * 17))
        encounter.set_actor_label(actor_label)
    encounter.set_actor_location(unreal.Vector(actor_xy[0], actor_xy[1], Z + half_height), False, False)
    encounter.set_actor_rotation(unreal.Rotator(0, 0, 180 + index * 17), False)

# A separately staged second daughter makes the paired production identity visible.
secondary = existing.get("GLM_ENCOUNTER_GravyDaughtersSecondary")
if not secondary:
    secondary = actors.spawn_actor_from_class(unreal.WyrmGravyDaughtersCharacter, unreal.Vector(5950, 2825, Z + 74), unreal.Rotator(0, 0, 95))
    secondary.set_actor_label("GLM_ENCOUNTER_GravyDaughtersSecondary")
secondary.set_actor_location(unreal.Vector(5950, 2825, Z + 74), False, False)
secondary.set_actor_rotation(unreal.Rotator(0, 0, 95), False)

lights = [
    ("GLM_RemainingHorrorNorthFill", unreal.Vector(5200, 4550, Z + 430), unreal.Color(80, 150, 130, 255)),
    ("GLM_RemainingHorrorEastFill", unreal.Vector(5900, 2700, Z + 430), unreal.Color(155, 80, 95, 255)),
    ("GLM_RemainingHorrorSouthFill", unreal.Vector(4350, 1350, Z + 430), unreal.Color(110, 90, 165, 255)),
]
for label, location, color in lights:
    light = existing.get(label)
    if not light:
        light = actors.spawn_actor_from_class(unreal.PointLight, location)
        light.set_actor_label(label)
    light.set_actor_location(location, False, False)
    component = light.get_component_by_class(unreal.PointLightComponent)
    component.set_mobility(unreal.ComponentMobility.MOVABLE)
    component.set_editor_property("intensity", 34.0)
    component.set_editor_property("attenuation_radius", 1250.0)
    component.set_editor_property("light_color", color)

if not levels.save_current_level():
    raise RuntimeError("Could not save required-horror roster")
print("WP-23.5 remaining required-horror roster placement: PASS")
