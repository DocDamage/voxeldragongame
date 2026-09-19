
import unreal
ASSETS = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
material = ASSETS.load_asset('/Game/WYRMFALL/Characters/Player/M_Knight')
for name in ['SK_KnightArcher', 'SK_KnightCaptain', 'SK_KnightChampion', 'SK_KnightCommander']:
    path = f'/Game/WYRMFALL/Characters/Player/{name}'
    sk = ASSETS.load_asset(path)
    if sk:
        mats = sk.get_editor_property('materials')
        if mats and len(mats) > 0:
            mats[0].set_editor_property('material_interface', material)
            sk.set_editor_property('materials', mats)
            ASSETS.save_loaded_asset(sk)
            print(f'Assigned M_Knight to {name}')
