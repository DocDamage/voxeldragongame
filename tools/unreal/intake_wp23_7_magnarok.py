"""Import the bounded rigged Magnarok fine candidate and supplied locomotion clips."""

import hashlib
import json
import traceback
from pathlib import Path

import unreal


ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
SOURCE = Path(r"G:\3d assets\king_demon_vulture")
OUT = ROOT / "Saved/Diagnostics/WP23_7_MagnarokIntake"
REPORT_PATH = OUT / "unreal_intake.json"
BASE_FBX = OUT / "Magnarok_VoxelFine_Rigged.fbx"
WALK_FBX = SOURCE / "Animation_Walking_withSkin.fbx"
RUN_FBX = SOURCE / "Animation_Running_withSkin.fbx"
DEST = "/Game/WYRMFALL/Development/Intake/WP23_7/Selected/Magnarok"
MESH_DEST = DEST + "/Mesh"
ANIM_DEST = DEST + "/Animations"
TEXTURE_DEST = DEST + "/Textures"
MATERIAL_DEST = DEST + "/Materials"
TEXTURES = {
    "Albedo": SOURCE / "King_Demon_Vulture_0120150621_texture.png",
    "Normal": SOURCE / "King_Demon_Vulture_0120150621_texture_normal.png",
    "Roughness": SOURCE / "King_Demon_Vulture_0120150621_texture_roughness.png",
    "Metallic": SOURCE / "King_Demon_Vulture_0120150621_texture_metallic.png",
}

ASSETS = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
TOOLS = unreal.AssetToolsHelpers.get_asset_tools()


def sha256(path):
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest().upper()


def load(path, expected):
    asset = ASSETS.load_asset(path)
    if asset is None or not isinstance(asset, expected):
        raise RuntimeError("Missing or wrong-class asset: " + path)
    return asset


def import_skeletal_mesh(source):
    options = unreal.FbxImportUI()
    options.set_editor_property("automated_import_should_detect_type", False)
    options.set_editor_property("mesh_type_to_import", unreal.FBXImportType.FBXIT_SKELETAL_MESH)
    options.set_editor_property("import_mesh", True)
    options.set_editor_property("import_as_skeletal", True)
    options.set_editor_property("import_materials", False)
    options.set_editor_property("import_textures", False)
    options.set_editor_property("import_animations", False)
    options.set_editor_property("create_physics_asset", False)
    skeletal_data = options.get_editor_property("skeletal_mesh_import_data")
    skeletal_data.set_editor_property("import_uniform_scale", 100.0)
    task = unreal.AssetImportTask()
    task.filename = source.as_posix()
    task.destination_path = MESH_DEST
    task.destination_name = "SK_Magnarok_VoxelFine"
    task.automated = True
    task.save = True
    task.replace_existing = True
    task.async_ = False
    task.factory = unreal.FbxFactory()
    task.options = options
    TOOLS.import_asset_tasks([task])
    for path in task.imported_object_paths:
        asset = ASSETS.load_asset(path)
        if isinstance(asset, unreal.SkeletalMesh):
            return asset, list(task.imported_object_paths)
    raise RuntimeError("Magnarok FBX did not produce a SkeletalMesh")


def import_animation(label, source, skeleton):
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
    task.filename = source.as_posix()
    task.destination_path = ANIM_DEST
    task.destination_name = "A_Magnarok_" + label
    task.automated = True
    task.save = True
    task.replace_existing = True
    task.async_ = False
    task.factory = unreal.FbxFactory()
    task.options = options
    TOOLS.import_asset_tasks([task])
    for path in task.imported_object_paths:
        asset = ASSETS.load_asset(path)
        if isinstance(asset, unreal.AnimSequence):
            return asset, list(task.imported_object_paths)
    object_path = ANIM_DEST + "/A_Magnarok_" + label
    return load(object_path, unreal.AnimSequence), list(task.imported_object_paths)


def import_texture(label, source):
    object_path = f"{TEXTURE_DEST}/T_Magnarok_{label}"
    task = unreal.AssetImportTask()
    task.filename = source.as_posix()
    task.destination_path = TEXTURE_DEST
    task.destination_name = "T_Magnarok_" + label
    task.automated = True
    task.save = True
    task.replace_existing = True
    task.async_ = False
    task.factory = unreal.TextureFactory()
    TOOLS.import_asset_tasks([task])
    texture = load(object_path, unreal.Texture2D)
    if label == "Normal":
        texture.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_NORMALMAP)
        texture.set_editor_property("srgb", False)
    elif label in {"Roughness", "Metallic"}:
        texture.set_editor_property("srgb", False)
    ASSETS.save_loaded_asset(texture)
    return texture


def pbr_material(textures):
    object_path = MATERIAL_DEST + "/M_Magnarok_SuppliedPBR"
    if ASSETS.does_asset_exist(object_path):
        ASSETS.delete_asset(object_path)
    material = TOOLS.create_asset("M_Magnarok_SuppliedPBR", MATERIAL_DEST, unreal.Material, unreal.MaterialFactoryNew())
    positions = {"Albedo": (-420, -180), "Normal": (-420, 0), "Roughness": (-420, 180), "Metallic": (-420, 340)}
    properties = {
        "Albedo": ("RGB", unreal.MaterialProperty.MP_BASE_COLOR),
        "Normal": ("RGB", unreal.MaterialProperty.MP_NORMAL),
        "Roughness": ("R", unreal.MaterialProperty.MP_ROUGHNESS),
        "Metallic": ("R", unreal.MaterialProperty.MP_METALLIC),
    }
    for label, texture in textures.items():
        x, y = positions[label]
        sample = unreal.MaterialEditingLibrary.create_material_expression(material, unreal.MaterialExpressionTextureSample, x, y)
        sample.set_editor_property("texture", texture)
        output_name, material_property = properties[label]
        unreal.MaterialEditingLibrary.connect_material_property(sample, output_name, material_property)
    unreal.MaterialEditingLibrary.recompile_material(material)
    ASSETS.save_loaded_asset(material)
    return material


def main():
    report = {
        "kind": "wp23_7_magnarok_unreal_intake",
        "engine": unreal.SystemLibrary.get_engine_version(),
        "scope": "isolated Magnarok presentation and supplied locomotion intake only; no gameplay, map, facts, Echo, travel, save, or regional completion",
        "status": "ERROR",
    }
    try:
        required = [BASE_FBX, WALK_FBX, RUN_FBX, *TEXTURES.values()]
        missing = [str(path) for path in required if not path.is_file()]
        if missing:
            raise RuntimeError(f"Missing Magnarok inputs: {missing}")
        mesh, mesh_imports = import_skeletal_mesh(BASE_FBX)
        skeleton = mesh.get_editor_property("skeleton")
        if skeleton is None:
            raise RuntimeError("Imported Magnarok mesh has no Skeleton")
        # Auto-created Skeleton packages are separate from the SkeletalMesh package.
        # Save it explicitly so locomotion assets remain valid after an editor restart.
        if not ASSETS.save_loaded_asset(skeleton):
            raise RuntimeError("Failed to persist imported Magnarok Skeleton")
        imported_textures = {label: import_texture(label, path) for label, path in TEXTURES.items()}
        material = pbr_material(imported_textures)
        material_slot = unreal.SkeletalMaterial()
        material_slot.set_editor_property("material_interface", material)
        material_slot.set_editor_property("material_slot_name", "Magnarok_SuppliedPBR")
        mesh.set_editor_property("materials", [material_slot])
        ASSETS.save_loaded_asset(mesh)
        walk, walk_imports = import_animation("Walk", WALK_FBX, skeleton)
        run, run_imports = import_animation("Run", RUN_FBX, skeleton)
        if not ASSETS.save_loaded_asset(walk) or not ASSETS.save_loaded_asset(run):
            raise RuntimeError("Failed to persist imported Magnarok locomotion assets")
        component = unreal.SkeletalMeshComponent()
        component.set_skinned_asset_and_update(mesh)
        animations = {}
        for label, animation in (("walk", walk), ("run", run)):
            animations[label] = {
                "object_path": animation.get_path_name(),
                "skeleton": animation.get_editor_property("skeleton").get_path_name(),
                "play_length_seconds": animation.get_play_length(),
                "number_of_sampled_keys": int(animation.get_editor_property("number_of_sampled_keys")),
            }
        bounds = mesh.get_imported_bounds()
        checks = {
            "mesh_is_skeletal": isinstance(mesh, unreal.SkeletalMesh),
            "bone_count": component.get_num_bones(),
            "expected_bone_count": 24,
            "material_populated": bool(mesh.get_editor_property("materials") and mesh.get_editor_property("materials")[0].material_interface),
            "height_cm": float(bounds.box_extent.z * 2.0),
            "height_in_expected_range": 150.0 <= float(bounds.box_extent.z * 2.0) <= 250.0,
            "walk_uses_imported_skeleton": animations["walk"]["skeleton"] == skeleton.get_path_name(),
            "run_uses_imported_skeleton": animations["run"]["skeleton"] == skeleton.get_path_name(),
            "walk_has_samples": animations["walk"]["number_of_sampled_keys"] > 0,
            "run_has_samples": animations["run"]["number_of_sampled_keys"] > 0,
        }
        passed = (
            checks["mesh_is_skeletal"] and checks["bone_count"] == checks["expected_bone_count"] and
            checks["material_populated"] and checks["height_in_expected_range"] and checks["walk_uses_imported_skeleton"] and
            checks["run_uses_imported_skeleton"] and checks["walk_has_samples"] and checks["run_has_samples"]
        )
        report.update({
            "status": "PASS_INTAKE" if passed else "PARTIAL",
            "sources": {path.name: {"path": str(path), "sha256": sha256(path)} for path in required},
            "skeletal_mesh": {
                "object_path": mesh.get_path_name(), "skeleton": skeleton.get_path_name(),
                "bounds_origin": list(bounds.origin.to_tuple()), "bounds_extent": list(bounds.box_extent.to_tuple()),
                "materials": [slot.material_interface.get_path_name() if slot.material_interface else None for slot in mesh.get_editor_property("materials")],
            },
            "animations": animations,
            "checks": checks,
            "imports": {"mesh": mesh_imports, "walk": walk_imports, "run": run_imports},
            "presentation_boundary": "Selected rigged fine candidate plus supplied walk/run only; no idle, attack, hit, defeat, facial, or authored boss performance coverage.",
        })
    except Exception:
        report["status"] = "ERROR"
        report["error"] = traceback.format_exc()
    finally:
        OUT.mkdir(parents=True, exist_ok=True)
        REPORT_PATH.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(f"WP-23.7 Magnarok Unreal intake: {report['status']}")
    print(f"Receipt: {REPORT_PATH}")
    if report["status"] == "ERROR":
        raise RuntimeError(report["error"])


main()
