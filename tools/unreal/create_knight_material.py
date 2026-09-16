import unreal
from pathlib import Path

print("=== CREATING KNIGHT MASTER MATERIAL ===")
ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
ASSETS = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
TOOLS = unreal.AssetToolsHelpers.get_asset_tools()
MAT = unreal.MaterialEditingLibrary

mat_path = "/Game/WYRMFALL/Characters/Player/M_Knight"
material = ASSETS.load_asset(mat_path) if ASSETS.does_asset_exist(mat_path) else None

if not material:
    material = TOOLS.create_asset("M_Knight", "/Game/WYRMFALL/Characters/Player", unreal.Material, unreal.MaterialFactoryNew())
    print("Created M_Knight asset")

# Load texture
tex_path = "/Game/WYRMFALL/Characters/Player/T_Knight"
tex = ASSETS.load_asset(tex_path)

# Texture Sample
sample = MAT.create_material_expression(material, unreal.MaterialExpressionTextureSample, -400, 0)
if tex:
    sample.set_editor_property("texture", tex)

# Vector Parameter: ArmorTint
tint = MAT.create_material_expression(material, unreal.MaterialExpressionVectorParameter, -400, 200)
tint.set_editor_property("parameter_name", "ArmorTint")
tint.set_editor_property("default_value", unreal.LinearColor(1.0, 1.0, 1.0, 1.0))

# Multiply
multiply = MAT.create_material_expression(material, unreal.MaterialExpressionMultiply, -150, 50)
MAT.connect_material_expressions(sample, "RGB", multiply, "A")
MAT.connect_material_expressions(tint, "RGB", multiply, "B")

# Connect to BaseColor
MAT.connect_material_property(multiply, "", unreal.MaterialProperty.MP_BASE_COLOR)

# Roughness
rough = MAT.create_material_expression(material, unreal.MaterialExpressionConstant, -150, 250)
rough.set_editor_property("r", 0.8)
MAT.connect_material_property(rough, "", unreal.MaterialProperty.MP_ROUGHNESS)

MAT.recompile_material(material)
ASSETS.save_loaded_asset(material)
print("Configured and saved M_Knight")

# Assign to SK_Knight and SK_KnightHelmDown
for mesh_path in ["/Game/WYRMFALL/Characters/Player/SK_Knight", "/Game/WYRMFALL/Characters/Player/SK_KnightHelmDown"]:
    sk = ASSETS.load_asset(mesh_path)
    if sk:
        mats = sk.get_editor_property("materials")
        if mats:
            mats[0].set_editor_property("material_interface", material)
        sk.set_editor_property("materials", mats)
        ASSETS.save_loaded_asset(sk)
        print(f"Assigned M_Knight to {mesh_path}")

# Assign M_Sword to SM_Sword
mat_sword_path = "/Game/WYRMFALL/Items/Weapons/M_Sword"
mat_sword = ASSETS.load_asset(mat_sword_path) if ASSETS.does_asset_exist(mat_sword_path) else None
if not mat_sword:
    mat_sword = TOOLS.create_asset("M_Sword", "/Game/WYRMFALL/Items/Weapons", unreal.Material, unreal.MaterialFactoryNew())
    tex_sword = ASSETS.load_asset("/Game/WYRMFALL/Items/Weapons/T_Sword")
    sample_sw = MAT.create_material_expression(mat_sword, unreal.MaterialExpressionTextureSample, -300, 0)
    if tex_sword:
        sample_sw.set_editor_property("texture", tex_sword)
    MAT.connect_material_property(sample_sw, "RGB", unreal.MaterialProperty.MP_BASE_COLOR)
    MAT.recompile_material(mat_sword)
    ASSETS.save_loaded_asset(mat_sword)
    
    sm_sword = ASSETS.load_asset("/Game/WYRMFALL/Items/Weapons/SM_Sword")
    if sm_sword:
        sm_mats = sm_sword.get_editor_property("static_materials")
        if sm_mats:
            sm_mats[0].set_editor_property("material_interface", mat_sword)
        sm_sword.set_editor_property("static_materials", sm_mats)
        ASSETS.save_loaded_asset(sm_sword)
        print("Assigned M_Sword to SM_Sword")

print("=== MATERIAL SETUP FINISHED ===")
