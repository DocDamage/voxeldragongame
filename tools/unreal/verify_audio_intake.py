
import unreal
ASSETS = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
cue_path = '/Game/UltimateUIMenusSFX/Fantasy_RPG/Cue/FantasyRPG_01_Cue'
cue = ASSETS.load_asset(cue_path)
print('Loaded Fantasy RPG Cue:', cue.get_path_name() if cue else 'Failed')
