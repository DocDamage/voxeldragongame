"""Import the supplied Voxel Palace assets used by the bounded Jade Peaks slice."""

import json
import sys
import zipfile
from pathlib import Path

import unreal

ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
ARCHIVE = ROOT / "assets and old docs/voxel/characters/palace.zip"
STAGING = ROOT / "Saved/Staging/WP22/Palace"
DEST = "/Game/WYRMFALL/Development/Intake/WP22/JadePeaks/Palace"
REPORT = ROOT / "Saved/Diagnostics/WP22_jade_peaks_asset_intake.json"
ASSETS = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
TOOLS = unreal.AssetToolsHelpers.get_asset_tools()

TEXTURES = {
    "TVS_VoxelPalace_Palace": "Voxel Palace/Textures/Environment/TVS_VoxelPalace_Palace_Texture.png",
    "TVS_VoxelPalace_Fountain": "Voxel Palace/Textures/Environment/TVS_VoxelPalace_Fountain_Texture.png",
    "TVS_VoxelPalace_Fence": "Voxel Palace/Textures/Environment/TVS_VoxelPalace_Fence_Texture.png",
    "TVS_VoxelPalace_PottedTree": "Voxel Palace/Textures/Environment/TVS_VoxelPalace_PottedTree_Texture.png",
    "TVS_VoxelPalace_Tree": "Voxel Palace/Textures/Environment/TVS_VoxelPalace_Tree_Texture.png",
    "TVS_VoxelPalace_Guard": "Voxel Palace/Textures/Characters/TVS_VoxelPalace_Guard_Texture.png",
    "TVS_VoxelPalace_Spear": "Voxel Palace/Textures/Props/TVS_VoxelPalace_Spear_Texture.png",
}


def extract_sources():
    wanted = [
        "Voxel Palace/FBX/Environment/TVS_VoxelPalace_Palace.fbx",
        "Voxel Palace/FBX/Environment/TVS_VoxelPalace_Fountain.fbx",
        "Voxel Palace/FBX/Environment/TVS_VoxelPalace_Fence.fbx",
        "Voxel Palace/FBX/Environment/TVS_VoxelPalace_PottedTree.fbx",
        "Voxel Palace/FBX/Environment/TVS_VoxelPalace_Tree.fbx",
        "Voxel Palace/FBX/Characters/TVS_VoxelPalace_Guard.fbx",
        "Voxel Palace/FBX/Props/TVS_VoxelPalace_Spear.fbx",
    ]
    STAGING.mkdir(parents=True, exist_ok=True)
    with zipfile.ZipFile(ARCHIVE) as archive:
        names = set(archive.namelist())
        for member in wanted:
            if member not in names:
                raise RuntimeError(f"Missing supplied palace member: {member}")
            target = STAGING / Path(member).name
            target.write_bytes(archive.read(member))
        for member in TEXTURES.values():
            if member not in names:
                raise RuntimeError(f"Missing supplied palace texture: {member}")
            (STAGING / Path(member).name).write_bytes(archive.read(member))
    return [STAGING / Path(member).name for member in wanted]


def options(skeletal=False):
    result = unreal.FbxImportUI()
    result.set_editor_property("automated_import_should_detect_type", True)
    result.set_editor_property("import_mesh", True)
    result.set_editor_property("import_as_skeletal", skeletal)
    result.set_editor_property("import_animations", False)
    result.set_editor_property("import_materials", True)
    result.set_editor_property("import_textures", True)
    return result


def import_one(source, folder, skeletal=False):
    name = "TVS_VoxelPalace_Guard" if source.stem.endswith("_Guard_Normalized") else source.stem
    destination = f"{DEST}/{folder}"
    object_path = f"{destination}/{name}"
    if ASSETS.does_asset_exist(object_path) and not skeletal:
        asset = ASSETS.load_asset(object_path)
        if asset:
            return asset
    task = unreal.AssetImportTask()
    task.filename = source.as_posix()
    task.destination_path = destination
    task.destination_name = name
    task.automated = True
    task.save = True
    task.replace_existing = True
    task.async_ = False
    if source.suffix.lower() == ".fbx":
        task.factory = unreal.FbxFactory()
    task.options = options(skeletal)
    TOOLS.import_asset_tasks([task])
    asset = ASSETS.load_asset(object_path)
    if not asset:
        raise RuntimeError(f"Import did not produce {object_path}")
    return asset


def create_material(asset_name, texture):
    material_name = f"M_{asset_name}"
    material_path = f"{DEST}/Materials/{material_name}"
    material = ASSETS.load_asset(material_path) if ASSETS.does_asset_exist(material_path) else None
    if not material:
        material = TOOLS.create_asset(material_name, f"{DEST}/Materials", unreal.Material, unreal.MaterialFactoryNew())
        sample = unreal.MaterialEditingLibrary.create_material_expression(
            material, unreal.MaterialExpressionTextureSampleParameter2D, -300, 0)
        sample.set_editor_property("parameter_name", "BaseTexture")
        sample.set_editor_property("texture", texture)
        unreal.MaterialEditingLibrary.connect_material_property(sample, "RGB", unreal.MaterialProperty.MP_BASE_COLOR)
        if asset_name == "TVS_VoxelPalace_Guard":
            material.set_editor_property("used_with_skeletal_mesh", True)
        unreal.MaterialEditingLibrary.recompile_material(material)
        unreal.EditorAssetLibrary.save_loaded_asset(material)
    elif asset_name == "TVS_VoxelPalace_Guard":
        material.set_editor_property("used_with_skeletal_mesh", True)
        unreal.MaterialEditingLibrary.recompile_material(material)
        unreal.EditorAssetLibrary.save_loaded_asset(material)
    return material


def main():
    report = {"status": "ERROR", "source": str(ARCHIVE), "assets": []}
    try:
        sources = extract_sources()
        normalized_guard = STAGING / "TVS_VoxelPalace_Guard_Normalized.fbx"
        if not normalized_guard.is_file():
            raise RuntimeError("Run tools/normalize_wp22_palace_guard.py with Blender before Unreal intake")
        sources = [normalized_guard if source.stem.endswith("_Guard") else source for source in sources]
        imported = {}
        for source in sources:
            skeletal = source.stem.endswith("_Guard_Normalized")
            folder = "Characters" if skeletal else ("Props" if source.stem.endswith("_Spear") else "Environment")
            asset = import_one(source, folder, skeletal)
            canonical_name = "TVS_VoxelPalace_Guard" if skeletal else source.stem
            imported[canonical_name] = asset
            report["assets"].append({"path": asset.get_path_name(), "class": asset.get_class().get_name()})
        for asset_name, member in TEXTURES.items():
            texture = import_one(STAGING / Path(member).name, "Textures")
            texture.set_editor_property("filter", unreal.TextureFilter.TF_NEAREST)
            unreal.EditorAssetLibrary.save_loaded_asset(texture)
            material = create_material(asset_name, texture)
            # StaticMesh exposes set_material in the editor API. The normalized
            # guard is a SkeletalMesh and receives this same supplied material
            # from AWyrmJadeDiscipleCharacter at runtime.
            if isinstance(imported[asset_name], unreal.StaticMesh):
                imported[asset_name].set_material(0, material)
            unreal.EditorAssetLibrary.save_loaded_asset(imported[asset_name])
            report["assets"].append({"path": texture.get_path_name(), "class": texture.get_class().get_name()})
            report["assets"].append({"path": material.get_path_name(), "class": material.get_class().get_name()})
        report["status"] = "PASS"
        # Skeletal import creates dependent Skeleton and PhysicsAsset packages
        # that are not guaranteed to be included in the task's returned paths.
        unreal.EditorAssetLibrary.save_directory(DEST, only_if_is_dirty=False, recursive=True)
    except Exception as exc:
        report["error"] = repr(exc)
        raise
    finally:
        REPORT.parent.mkdir(parents=True, exist_ok=True)
        REPORT.write_text(json.dumps(report, indent=2), encoding="utf-8")


if __name__ == "__main__":
    try:
        main()
    except Exception:
        sys.exit(1)
