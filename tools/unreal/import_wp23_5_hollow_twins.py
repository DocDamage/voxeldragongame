"""Import the supplied Hollow Twins presentation for the production slice."""

import json
from pathlib import Path
import zipfile

import unreal


ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
ARCHIVE = ROOT / "assets and old docs/voxel/characters/Voxel Cathedral.zip"
STAGING = ROOT / "Saved/AssetIntake/WP23_5/HollowTwins"
ROLE_DEST = "/Game/WYRMFALL/Development/Intake/WP23_5/Gloaming/Roles/HollowTwins"
ANIM_DEST = "/Game/WYRMFALL/Development/Intake/WP23_5/Gloaming/RoleAnimations"
TEXTURE_DEST = "/Game/WYRMFALL/Development/Intake/WP23_5/Gloaming/FixtureTextures"
MATERIAL_DEST = "/Game/WYRMFALL/Development/Intake/WP23_5/Gloaming/FixtureMaterials"
MESH_PATH = ROLE_DEST + "/TVS_VoxelCathedral_Nun"
REPORT = ROOT / "Saved/Diagnostics/WP23_5_hollow_twins_intake.json"
IDENTITIES = (
    ("HollowTwinMorrow", 0.0),
    ("HollowTwinMourn", 0.65),
)


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
    mesh_source = source / "FBX/Characters/TVS_VoxelCathedral_Nun.fbx"
    animation_source = source / "Animations/Human_Praying_Anim.fbx"
    texture_source = source / "Textures/Characters/TVS_VoxelCathedral_Nun_Texture.png"
    for required in (mesh_source, animation_source, texture_source):
        if not required.is_file():
            raise RuntimeError("Missing supplied Hollow Twins source: " + required.as_posix())

    assets = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
    tools = unreal.AssetToolsHelpers.get_asset_tools()
    if not assets.does_asset_exist(MESH_PATH):
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
        raise RuntimeError("Supplied Nun mesh has no skeleton")

    rows = []
    for identity, phase in IDENTITIES:
        texture_path = f"{TEXTURE_DEST}/T_{identity}"
        if not assets.does_asset_exist(texture_path):
            task = unreal.AssetImportTask()
            task.filename = texture_source.as_posix()
            task.destination_path = TEXTURE_DEST
            task.destination_name = "T_" + identity
            task.automated = True
            task.save = True
            task.replace_existing = False
            task.async_ = False
            task.factory = unreal.TextureFactory()
            tools.import_asset_tasks([task])
        texture = load(assets, texture_path, unreal.Texture2D)

        material_path = f"{MATERIAL_DEST}/M_{identity}_ProductionPalette"
        material = assets.load_asset(material_path) if assets.does_asset_exist(material_path) else None
        if material is None:
            material = tools.create_asset(
                f"M_{identity}_ProductionPalette", MATERIAL_DEST,
                unreal.Material, unreal.MaterialFactoryNew())
            sample = unreal.MaterialEditingLibrary.create_material_expression(
                material, unreal.MaterialExpressionTextureSample, -240, 0)
            sample.set_editor_property("texture", texture)
            tint = unreal.MaterialEditingLibrary.create_material_expression(
                material, unreal.MaterialExpressionVectorParameter, -240, 120)
            tint.set_editor_property("parameter_name", "IdentityTint")
            tint.set_editor_property(
                "default_value",
                unreal.LinearColor(0.72, 0.82, 1.0, 1.0) if identity.endswith("Morrow")
                else unreal.LinearColor(0.82, 0.62, 0.88, 1.0))
            multiply = unreal.MaterialEditingLibrary.create_material_expression(
                material, unreal.MaterialExpressionMultiply, 0, 0)
            unreal.MaterialEditingLibrary.connect_material_expressions(sample, "RGB", multiply, "A")
            unreal.MaterialEditingLibrary.connect_material_expressions(tint, "RGB", multiply, "B")
            unreal.MaterialEditingLibrary.connect_material_property(
                multiply, "", unreal.MaterialProperty.MP_BASE_COLOR)
            roughness = unreal.MaterialEditingLibrary.create_material_expression(
                material, unreal.MaterialExpressionConstant, -240, 160)
            roughness.set_editor_property("r", 0.88)
            unreal.MaterialEditingLibrary.connect_material_property(
                roughness, "", unreal.MaterialProperty.MP_ROUGHNESS)
            unreal.MaterialEditingLibrary.recompile_material(material)
            assets.save_loaded_asset(material)
        material = load(assets, material_path, unreal.MaterialInterface)
        if isinstance(material, unreal.Material):
            material.set_editor_property("used_with_skeletal_mesh", True)
            unreal.MaterialEditingLibrary.recompile_material(material)
            assets.save_loaded_asset(material)

        animation_path = f"{ANIM_DEST}/A_{identity}_Praying"
        if not assets.does_asset_exist(animation_path):
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
            task.filename = animation_source.as_posix()
            task.destination_path = ANIM_DEST
            task.destination_name = f"A_{identity}_Praying"
            task.automated = True
            task.save = True
            task.replace_existing = False
            task.async_ = False
            task.factory = unreal.FbxFactory()
            task.options = options
            tools.import_asset_tasks([task])
        animation = load(assets, animation_path, unreal.AnimSequence)
        rows.append({
            "identity": identity, "material": material.get_path_name(),
            "animation": animation.get_path_name(), "animation_phase": phase,
        })

    REPORT.parent.mkdir(parents=True, exist_ok=True)
    REPORT.write_text(json.dumps({
        "kind": "wp23_5_hollow_twins_production_intake",
        "engine": unreal.SystemLibrary.get_engine_version(), "status": "PASS",
        "source_archive": str(ARCHIVE), "mesh": mesh.get_path_name(),
        "authored_identities": rows,
    }, indent=2) + "\n", encoding="utf-8")
    print("WP-23.5 Hollow Twins intake: PASS")


main()
