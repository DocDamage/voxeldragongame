"""Import the supplied Count Malvaine presentation for the production slice."""

import json
from pathlib import Path
import zipfile

import unreal


ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
ARCHIVE = ROOT / "assets and old docs/voxel/characters/Voxel Cathedral.zip"
STAGING = ROOT / "Saved/AssetIntake/WP23_5/Malvaine"
ROLE_DEST = "/Game/WYRMFALL/Development/Intake/WP23_5/Gloaming/Roles/CountMalvaine"
ANIM_DEST = "/Game/WYRMFALL/Development/Intake/WP23_5/Gloaming/RoleAnimations"
TEXTURE_DEST = "/Game/WYRMFALL/Development/Intake/WP23_5/Gloaming/FixtureTextures"
MATERIAL_DEST = "/Game/WYRMFALL/Development/Intake/WP23_5/Gloaming/FixtureMaterials"
MESH_PATH = ROLE_DEST + "/TVS_VoxelCathedral_Priest"
ANIM_PATH = ANIM_DEST + "/A_CountMalvaine_Preach"
TEXTURE_PATH = TEXTURE_DEST + "/T_CountMalvaine"
MATERIAL_PATH = MATERIAL_DEST + "/M_CountMalvaine_SuppliedPaletteV2"
REPORT = ROOT / "Saved/Diagnostics/WP23_5_malvaine_intake.json"


def load(assets, path, expected):
    asset = assets.load_asset(path)
    if asset is None or not isinstance(asset, expected):
        raise RuntimeError("Missing or wrong-class asset: " + path)
    return asset


def main():
    if not ARCHIVE.is_file():
        raise RuntimeError("Missing supplied Voxel Cathedral archive")
    STAGING.mkdir(parents=True, exist_ok=True)
    with zipfile.ZipFile(ARCHIVE) as archive:
        archive.extractall(STAGING)
    source = STAGING / "Voxel Cathedral"
    mesh_source = source / "FBX/Characters/TVS_VoxelCathedral_Priest.fbx"
    anim_source = source / "Animations/Human_Preach_Anim.fbx"
    texture_source = source / "Textures/Characters/TVS_VoxelCathedral_Priest_Texture.png"
    for required in (mesh_source, anim_source, texture_source):
        if not required.is_file():
            raise RuntimeError("Missing supplied Malvaine source: " + required.as_posix())

    assets = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
    tools = unreal.AssetToolsHelpers.get_asset_tools()
    mesh = assets.load_asset(MESH_PATH) if assets.does_asset_exist(MESH_PATH) else None
    if mesh is None:
        task = unreal.AssetImportTask()
        task.filename = mesh_source.as_posix()
        task.destination_path = ROLE_DEST
        task.automated = True
        task.save = True
        task.replace_existing = False
        task.async_ = False
        tools.import_asset_tasks([task])
    mesh = load(assets, MESH_PATH, unreal.SkeletalMesh)
    skeleton = mesh.get_editor_property("skeleton")
    if not skeleton:
        raise RuntimeError("Supplied Priest mesh has no skeleton")

    texture = assets.load_asset(TEXTURE_PATH) if assets.does_asset_exist(TEXTURE_PATH) else None
    if texture is None:
        task = unreal.AssetImportTask()
        task.filename = texture_source.as_posix()
        task.destination_path = TEXTURE_DEST
        task.destination_name = "T_CountMalvaine"
        task.automated = True
        task.save = True
        task.replace_existing = False
        task.async_ = False
        task.factory = unreal.TextureFactory()
        tools.import_asset_tasks([task])
    texture = load(assets, TEXTURE_PATH, unreal.Texture2D)

    material = assets.load_asset(MATERIAL_PATH) if assets.does_asset_exist(MATERIAL_PATH) else None
    if material is None:
        material = tools.create_asset(
            "M_CountMalvaine_SuppliedPaletteV2", MATERIAL_DEST,
            unreal.Material, unreal.MaterialFactoryNew())
        sample = unreal.MaterialEditingLibrary.create_material_expression(
            material, unreal.MaterialExpressionTextureSample, -240, 0)
        sample.set_editor_property("texture", texture)
        unreal.MaterialEditingLibrary.connect_material_property(
            sample, "RGB", unreal.MaterialProperty.MP_BASE_COLOR)
        roughness = unreal.MaterialEditingLibrary.create_material_expression(
            material, unreal.MaterialExpressionConstant, -240, 160)
        roughness.set_editor_property("r", 0.9)
        unreal.MaterialEditingLibrary.connect_material_property(
            roughness, "", unreal.MaterialProperty.MP_ROUGHNESS)
        unreal.MaterialEditingLibrary.recompile_material(material)
        assets.save_loaded_asset(material)
    material = load(assets, MATERIAL_PATH, unreal.MaterialInterface)
    if isinstance(material, unreal.Material):
        material.set_editor_property("used_with_skeletal_mesh", True)
        unreal.MaterialEditingLibrary.recompile_material(material)
        assets.save_loaded_asset(material)

    animation = assets.load_asset(ANIM_PATH) if assets.does_asset_exist(ANIM_PATH) else None
    if animation is None:
        options = unreal.FbxImportUI()
        options.set_editor_property("automated_import_should_detect_type", False)
        options.set_editor_property("mesh_type_to_import", unreal.FBXImportType.FBXIT_ANIMATION)
        options.set_editor_property("import_mesh", False)
        options.set_editor_property("import_as_skeletal", True)
        options.set_editor_property("import_animations", True)
        options.set_editor_property("import_materials", False)
        options.set_editor_property("import_textures", False)
        options.set_editor_property("skeleton", skeleton)
        task = unreal.AssetImportTask()
        task.filename = anim_source.as_posix()
        task.destination_path = ANIM_DEST
        task.destination_name = "A_CountMalvaine_Preach"
        task.automated = True
        task.save = True
        task.replace_existing = False
        task.async_ = False
        task.factory = unreal.FbxFactory()
        task.options = options
        tools.import_asset_tasks([task])
    animation = load(assets, ANIM_PATH, unreal.AnimSequence)

    REPORT.parent.mkdir(parents=True, exist_ok=True)
    REPORT.write_text(json.dumps({
        "kind": "wp23_5_count_malvaine_production_intake",
        "engine": unreal.SystemLibrary.get_engine_version(), "status": "PASS",
        "source_archive": str(ARCHIVE), "mesh": mesh.get_path_name(),
        "material": material.get_path_name(), "animation": animation.get_path_name(),
        "animation_seconds": animation.get_play_length(),
    }, indent=2) + "\n", encoding="utf-8")
    print("WP-23.5 Count Malvaine intake: PASS")


main()
