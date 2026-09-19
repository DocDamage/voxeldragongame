"""Import only the supplied ranger assets used by the bounded Verdant Reach closure."""

import json
import zipfile
from pathlib import Path
import unreal

ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
ARCHIVE = ROOT / "assets and old docs/voxel/characters/rangers.zip"
STAGING = ROOT / "Saved/Staging/WP23_1/Rangers"
DEST = "/Game/WYRMFALL/Development/Intake/WP23_1/VerdantReach"
REPORT = ROOT / "Saved/Diagnostics/WP23_1_verdant_asset_intake.json"
ASSETS = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
TOOLS = unreal.AssetToolsHelpers.get_asset_tools()

SOURCES = {
    "SK_Meridess": "Voxel Rangers/FBX/Character/TVS_VoxelRangers_LongHair.fbx",
    "SK_CanopyHunter": "Voxel Rangers/FBX/Character/TVS_VoxelRangers_Hooded.fbx",
    "SM_RangerBanner": "Voxel Rangers/FBX/Environment/TVS_VoxelRangers_Banner.fbx",
    "SM_RangerBush": "Voxel Rangers/FBX/Environment/TVS_VoxelRangers_Bush.fbx",
    "SM_RangerCampfire": "Voxel Rangers/FBX/Environment/TVS_VoxelRangers_Campfire_Frame1.fbx",
    "SM_RangerTent": "Voxel Rangers/FBX/Environment/TVS_VoxelRangers_Tent.fbx",
    "SM_RangerTower": "Voxel Rangers/FBX/Environment/TVS_VoxelRangers_TowerWithDoor.fbx",
    "SM_RangerWall": "Voxel Rangers/FBX/Environment/TVS_VoxelRangers_Wall.fbx",
}
TEXTURES = {
    "T_Meridess": "Voxel Rangers/Textures/Characters/TVS_VoxelRangers_Ranger3_Texture.png",
    "T_CanopyHunter": "Voxel Rangers/Textures/Characters/TVS_VoxelRangers_Ranger1_Texture.png",
    "T_RangerBanner": "Voxel Rangers/Textures/Environment/TVS_VoxelRangers_Banner_Texture.png",
    "T_RangerBush": "Voxel Rangers/Textures/Environment/TVS_VoxelRangers_Bush_Texture.png",
    "T_RangerCampfire": "Voxel Rangers/Textures/Environment/TVS_VoxelRangers_Campfire_Texture.png",
    "T_RangerTent": "Voxel Rangers/Textures/Environment/TVS_VoxelRangers_Tent_Texture.png",
    "T_RangerTower": "Voxel Rangers/Textures/Environment/TVS_VoxelRangers_TowerWithDoor_Texture.png",
    "T_RangerWall": "Voxel Rangers/Textures/Environment/TVS_VoxelRangers_Wall_Texture.png",
}


def extract():
    STAGING.mkdir(parents=True, exist_ok=True)
    with zipfile.ZipFile(ARCHIVE) as archive:
        for member in list(SOURCES.values()) + list(TEXTURES.values()):
            if member not in archive.namelist():
                raise RuntimeError(f"Missing supplied ranger member: {member}")
            (STAGING / Path(member).name).write_bytes(archive.read(member))


def import_task(source, destination, name, skeletal=False):
    task = unreal.AssetImportTask()
    task.filename = source.as_posix()
    task.destination_path = destination
    task.destination_name = name
    task.automated = True
    task.save = True
    task.replace_existing = True
    task.async_ = False
    if source.suffix.lower() == ".fbx":
        options = unreal.FbxImportUI()
        options.set_editor_property("automated_import_should_detect_type", False)
        options.set_editor_property("mesh_type_to_import", unreal.FBXImportType.FBXIT_SKELETAL_MESH if skeletal else unreal.FBXImportType.FBXIT_STATIC_MESH)
        options.set_editor_property("import_mesh", True)
        options.set_editor_property("import_as_skeletal", skeletal)
        options.set_editor_property("import_animations", False)
        options.set_editor_property("import_materials", False)
        options.set_editor_property("import_textures", False)
        task.options = options
    TOOLS.import_asset_tasks([task])
    object_path = f"{destination}/{name}"
    asset = ASSETS.load_asset(object_path)
    if not asset:
        raise RuntimeError(f"Import did not produce {object_path}")
    return asset


def material(name, texture, skeletal=False):
    path = f"{DEST}/Materials/{name}"
    result = ASSETS.load_asset(path) if ASSETS.does_asset_exist(path) else None
    if not result:
        result = TOOLS.create_asset(name, f"{DEST}/Materials", unreal.Material, unreal.MaterialFactoryNew())
        sample = unreal.MaterialEditingLibrary.create_material_expression(result, unreal.MaterialExpressionTextureSample, -250, 0)
        sample.set_editor_property("texture", texture)
        unreal.MaterialEditingLibrary.connect_material_property(sample, "RGB", unreal.MaterialProperty.MP_BASE_COLOR)
    if skeletal:
        result.set_editor_property("used_with_skeletal_mesh", True)
    unreal.MaterialEditingLibrary.recompile_material(result)
    unreal.EditorAssetLibrary.save_loaded_asset(result)
    return result


def main():
    extract()
    imported = {}
    for name, member in TEXTURES.items():
        imported[name] = import_task(STAGING / Path(member).name, f"{DEST}/Textures", name)
    for name, member in SOURCES.items():
        skeletal = name.startswith("SK_")
        folder = "Characters" if skeletal else "Environment"
        imported[name] = import_task(STAGING / Path(member).name, f"{DEST}/{folder}", name, skeletal)

    assignments = {
        "SK_Meridess": material("M_Meridess", imported["T_Meridess"], True),
        "SK_CanopyHunter": material("M_CanopyHunter", imported["T_CanopyHunter"], True),
        "SM_RangerBanner": material("M_RangerBanner", imported["T_RangerBanner"]),
        "SM_RangerBush": material("M_RangerBush", imported["T_RangerBush"]),
        "SM_RangerCampfire": material("M_RangerCampfire", imported["T_RangerCampfire"]),
        "SM_RangerTent": material("M_RangerTent", imported["T_RangerTent"]),
        "SM_RangerTower": material("M_RangerTower", imported["T_RangerTower"]),
        "SM_RangerWall": material("M_RangerWall", imported["T_RangerWall"]),
    }
    for mesh_name, mat in assignments.items():
        if mesh_name.startswith("SM_"):
            imported[mesh_name].set_material(0, mat)
            unreal.EditorAssetLibrary.save_loaded_asset(imported[mesh_name])

    rows = [{"name": name, "path": asset.get_path_name(), "class": asset.get_class().get_name()}
            for name, asset in imported.items()]
    REPORT.parent.mkdir(parents=True, exist_ok=True)
    REPORT.write_text(json.dumps({"status": "PASS", "source": str(ARCHIVE), "assets": rows}, indent=2), encoding="utf-8")


if __name__ == "__main__":
    main()
