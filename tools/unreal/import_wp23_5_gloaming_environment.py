"""Import the bounded supplied environment subset for L_GloamingMarches."""

import io
import json
from pathlib import Path
import zipfile

import unreal


ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
OUT = ROOT / "Saved/Diagnostics/WP23_5_gloaming_environment_intake.json"
STAGING = ROOT / "Saved/AssetIntake/WP23_5/GloamingEnvironment"
CEMETERY_ARCHIVE = ROOT / "assets and old docs/voxel/cemetary and church voxel set.zip"
CATHEDRAL_ROOT = ROOT / "Saved/Diagnostics/WP23_5_Source/Cathedral/Voxel Cathedral"
CEMETERY_DEST = "/Game/WYRMFALL/Development/Intake/WP23_5/Gloaming/Cemetery"
MATERIAL_DEST = "/Game/WYRMFALL/Development/Intake/WP23_5/Gloaming/EnvironmentMaterials"
TEXTURE_DEST = "/Game/WYRMFALL/Development/Intake/WP23_5/Gloaming/EnvironmentTextures"
CATHEDRAL_DEST = "/Game/WYRMFALL/Development/Intake/WP23_5/Gloaming/Environment"

ASSETS = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
TOOLS = unreal.AssetToolsHelpers.get_asset_tools()

CEMETERY = (
    ("Gate", "church-44-gy_gatedbl"),
    ("Fence", "church-30-gy_fencemetal2"),
    ("Tombstone", "church-58-gy_tombstone1"),
    ("Coffin", "church-55-coffin"),
    ("Crypt", "church-67-crypt"),
    ("DirtGrave", "church-74-gy_dirtgrave"),
    ("Gargoyle", "church-79-gargoyle"),
)

CATHEDRAL = (
    ("Cathedral", "Cathedral", "TVS_VoxelCathedral_Cathedral", "TVS_VoxelCathedral_Cathedral_Texture.png"),
    ("CrossGrave", "CrossGrave", "TVS_VoxelCathedral_CrossGrave", "TVS_VoxelCathedral_CrossGrave_Texture.png"),
    ("Grave", "Grave", "TVS_VoxelCathedral_Grave", "TVS_VoxelCathedral_Grave_Texture.png"),
    ("Statue", "Statue", "TVS_VoxelCathedral_Statue", "TVS_VoxelCathedral_Statues_Texture.png"),
    ("Tree", "Tree", "TVS_VoxelCathedral_Tree", "TVS_VoxelCathedral_Tree_Texture.png"),
)


def import_texture(source, name):
    object_path = f"{TEXTURE_DEST}/T_{name}"
    existing = ASSETS.load_asset(object_path) if ASSETS.does_asset_exist(object_path) else None
    if existing is not None:
        return existing
    factory = unreal.TextureFactory()
    factory.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_EDITOR_ICON)
    factory.set_editor_property("mip_gen_settings", unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS)
    task = unreal.AssetImportTask()
    task.filename = source.as_posix()
    task.destination_path = TEXTURE_DEST
    task.destination_name = "T_" + name
    task.automated = True
    task.save = True
    task.replace_existing = False
    task.async_ = False
    task.factory = factory
    TOOLS.import_asset_tasks([task])
    texture = ASSETS.load_asset(object_path)
    if texture is None or not isinstance(texture, unreal.Texture2D):
        raise RuntimeError("Texture import failed: " + source.as_posix())
    return texture


def material_for(name, texture):
    path = f"{MATERIAL_DEST}/M_{name}_Supplied"
    material = ASSETS.load_asset(path) if ASSETS.does_asset_exist(path) else None
    if material is not None:
        return material
    material = TOOLS.create_asset(
        "M_" + name + "_Supplied", MATERIAL_DEST,
        unreal.Material, unreal.MaterialFactoryNew())
    if material is None:
        raise RuntimeError("Could not create material for " + name)
    sample = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionTextureSample, -260, 0)
    sample.set_editor_property("texture", texture)
    unreal.MaterialEditingLibrary.connect_material_property(
        sample, "RGB", unreal.MaterialProperty.MP_BASE_COLOR)
    roughness = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionConstant, -260, 180)
    roughness.set_editor_property("r", 0.92)
    unreal.MaterialEditingLibrary.connect_material_property(
        roughness, "", unreal.MaterialProperty.MP_ROUGHNESS)
    unreal.MaterialEditingLibrary.recompile_material(material)
    ASSETS.save_loaded_asset(material)
    return material


def import_obj(name, stem):
    source_dir = STAGING / stem
    source = source_dir / (stem + ".obj")
    destination = f"{CEMETERY_DEST}/{name}"
    existing = [
        ASSETS.load_asset(path) for path in ASSETS.list_assets(destination, recursive=False, include_folder=False)
    ] if ASSETS.does_directory_exist(destination) else []
    mesh = next((asset for asset in existing if isinstance(asset, unreal.StaticMesh)), None)
    if mesh is None:
        task = unreal.AssetImportTask()
        task.filename = source.as_posix()
        task.destination_path = destination
        task.destination_name = "SM_Gloaming" + name
        task.automated = True
        task.save = True
        task.replace_existing = False
        task.async_ = False
        TOOLS.import_asset_tasks([task])
        imported = [ASSETS.load_asset(path) for path in task.imported_object_paths]
        mesh = next((asset for asset in imported if isinstance(asset, unreal.StaticMesh)), None)
    if mesh is None:
        raise RuntimeError("OBJ did not produce a StaticMesh for " + name)
    texture = import_texture(source_dir / (stem + ".png"), "Cemetery" + name)
    return mesh, material_for("Cemetery" + name, texture)


def main():
    if not CEMETERY_ARCHIVE.is_file():
        raise RuntimeError("Missing supplied cemetery archive")
    STAGING.mkdir(parents=True, exist_ok=True)
    with zipfile.ZipFile(CEMETERY_ARCHIVE) as outer:
        nested_name = outer.namelist()[0]
        with zipfile.ZipFile(io.BytesIO(outer.read(nested_name))) as nested:
            for _, stem in CEMETERY:
                destination = STAGING / stem
                destination.mkdir(parents=True, exist_ok=True)
                for suffix in (".obj", ".mtl", ".png"):
                    member = "obj/" + stem + suffix
                    (destination / (stem + suffix)).write_bytes(nested.read(member))

    report = {
        "kind": "wp23_5_gloaming_environment_intake",
        "engine": unreal.SystemLibrary.get_engine_version(),
        "status": "STARTING",
        "cemetery": [],
        "cathedral": [],
    }
    for name, stem in CEMETERY:
        mesh, material = import_obj(name, stem)
        report["cemetery"].append({
            "name": name, "source": stem + ".obj",
            "mesh": mesh.get_path_name(), "material": material.get_path_name(),
        })
    for name, folder, asset_name, texture_name in CATHEDRAL:
        mesh_path = f"{CATHEDRAL_DEST}/{folder}/{asset_name}.{asset_name}"
        mesh = ASSETS.load_asset(mesh_path)
        if mesh is None or not isinstance(mesh, unreal.StaticMesh):
            raise RuntimeError("Missing canonical Cathedral environment mesh: " + mesh_path)
        texture_source = CATHEDRAL_ROOT / "Textures/Environment" / texture_name
        if not texture_source.is_file():
            raise RuntimeError("Missing supplied Cathedral environment texture: " + texture_source.as_posix())
        texture = import_texture(texture_source, "Cathedral" + name)
        material = material_for("Cathedral" + name, texture)
        report["cathedral"].append({
            "name": name, "mesh": mesh.get_path_name(), "material": material.get_path_name(),
        })
    report["status"] = "PASS"
    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print("WP-23.5 Gloaming environment intake: PASS")


main()
