"""Add the bounded Gloaming arrival/return anchors without recomposing the map."""

import unreal


MAP = "/Game/WYRMFALL/World/Regions/L_GloamingMarches"
SURFACE_Z = 808.0

levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
if not levels.load_level(MAP):
    raise RuntimeError("Could not load L_GloamingMarches")

existing = {actor.get_actor_label(): actor for actor in actors.get_all_level_actors()}
for label, location in (
    ("LM-GLOAMING-ARRIVAL", (-5000, -3400, SURFACE_Z)),
    ("LM-GLOAMING-RETURN", (-4550, -3050, SURFACE_Z)),
):
    actor = existing.get(label)
    if not actor:
        actor = actors.spawn_actor_from_class(unreal.TargetPoint, unreal.Vector(*location))
        actor.set_actor_label(label)
    actor.set_actor_location(unreal.Vector(*location), False, False)

if not levels.save_current_level():
    raise RuntimeError("Could not save Gloaming travel anchors")
print("WP-23.5 Gloaming travel anchors: PASS")
