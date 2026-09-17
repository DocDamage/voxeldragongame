"""Selective WP-12 import and inspection of supplied Region 01 NPC art.

The source FBX/texture files are extracted from user-supplied archives into
Saved/AssetIntake/WP12/NPC.  This script imports only the six selected role
samples into the ignored Development/Intake mount, measures their real Unreal
objects, and writes a receipt. Source idle/walk clips are imported through UE
5.8's legacy ``FbxFactory`` with an explicit target skeleton. The default
Interchange path classified the initial supplied idle sample as a StaticMesh,
so it is intentionally bypassed for these source clips. It does not create a
map, spawn gameplay NPCs, or assert a production acceptance result.
"""

import hashlib
import json
import traceback
from pathlib import Path

import unreal


ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
STAGING = ROOT / "Saved" / "AssetIntake" / "WP12" / "NPC"
DEST_ROOT = "/Game/WYRMFALL/Development/Intake/WP12/NPC"
OUTPUT = ROOT / "Saved" / "Diagnostics" / "WP12_region01_npc_intake.json"


def role(role_id, source_pack, mesh, texture, idle, walk):
    return {
        "role": role_id,
        "source_pack": source_pack,
        "mesh": STAGING / mesh,
        "texture": STAGING / texture,
        "idle": STAGING / idle,
        "walk": STAGING / walk,
    }


# These are role-fit candidates only. Their visual treatment, material
# completion, dialogue, interaction behavior, staging, and animation use must
# be validated separately before production placement is claimed.
ROLES = (
    role(
        "Tamsin",
        "Voxel Rangers / Captain",
        "Voxel Rangers/FBX/Character/TVS_VoxelRangers_Captain.fbx",
        "Voxel Rangers/Textures/Characters/TVS_VoxelRangers_Captain_Texture.png",
        "Voxel Rangers/Animations/Human_Idle_Anim.fbx",
        "Voxel Rangers/Animations/Human_Walk_Anim.fbx",
    ),
    role(
        "Mara",
        "Voxel Village / MarketWoman",
        "Voxel Village/FBX/Characters/TVS_VoxelVillage_MarketWoman.fbx",
        "Voxel Village/Textures/Characters/TVS_VoxelVillage_MarketWoman_Texture.png",
        "Voxel Village/Animations/Human_Idle_Anim.fbx",
        "Voxel Village/Animations/Human_Walk_Anim.fbx",
    ),
    role(
        "Sella",
        "Voxel Farm / FarmersDaughter",
        "Voxel Farm/FBX/Characters/TVS_VoxelFarm_FarmersDaughter.fbx",
        "Voxel Farm/Textures/Characters/TVS_VoxelFarm_FarmersDaughter_Texture.png",
        "Voxel Farm/Animations/Human/Human_Idle_Anim.fbx",
        "Voxel Farm/Animations/Human/Human_Walk_Anim.fbx",
    ),
    role(
        "Pell",
        "Voxel Farm / FarmHand",
        "Voxel Farm/FBX/Characters/TVS_VoxelFarm_FarmHand.fbx",
        "Voxel Farm/Textures/Characters/TVS_VoxelFarm_Farmhand_Texture.png",
        "Voxel Farm/Animations/Human/Human_Idle_Anim.fbx",
        "Voxel Farm/Animations/Human/Human_Walk_Anim.fbx",
    ),
    role(
        "Iven",
        "Voxel Village / YoungMan",
        "Voxel Village/FBX/Characters/TVS_VoxelVillage_YoungMan.fbx",
        "Voxel Village/Textures/Characters/TVS_VoxelVillage_YoungMan_Texture.png",
        "Voxel Village/Animations/Human_Idle_Anim.fbx",
        "Voxel Village/Animations/Human_Walk_Anim.fbx",
    ),
    role(
        "Rusk",
        "Voxel Knights / Commander",
        "Voxel Knights/FBX/Characters/TVS_VoxelKnights_Commander.fbx",
        "Voxel Knights/Textures/Characters/TVS_VoxelKnights_Commander_Texture.png",
        "Voxel Knights/Animations/Humans/Human_Idle_Anim.fbx",
        "Voxel Knights/Animations/Humans/Human_Walk_Anim.fbx",
    ),
)


ASSETS = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
TOOLS = unreal.AssetToolsHelpers.get_asset_tools()


def sha256(path):
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for chunk in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def assert_source_file(path):
    if not path.is_file():
        raise RuntimeError("Missing supplied intake file: " + path.as_posix())


def load_or_import(source, destination, name, options=None, factory=None):
    object_path = destination + "/" + name
    if ASSETS.does_asset_exist(object_path):
        loaded = ASSETS.load_asset(object_path)
        if loaded is None:
            raise RuntimeError("Existing intake object would not load: " + object_path)
        return loaded, False

    task = unreal.AssetImportTask()
    task.filename = source.as_posix()
    task.destination_path = destination
    task.destination_name = name
    task.automated = True
    task.save = True
    task.replace_existing = False
    task.async_ = False
    if factory is not None:
        task.factory = factory
    if options is not None:
        task.options = options
    TOOLS.import_asset_tasks([task])
    loaded = ASSETS.load_asset(object_path)
    if loaded is None:
        raise RuntimeError("Import did not produce the expected object: " + object_path)
    return loaded, True


def skeletal_import_options(skeleton=None):
    options = unreal.FbxImportUI()
    options.set_editor_property("import_mesh", True)
    options.set_editor_property("import_as_skeletal", True)
    options.set_editor_property("import_materials", False)
    options.set_editor_property("import_textures", False)
    options.set_editor_property("import_animations", False)
    if skeleton is not None:
        options.set_editor_property("skeleton", skeleton)
    return options


def legacy_animation_import_options(skeleton):
    """Use the installed legacy FBX factory, not default Interchange, for clips."""
    options = unreal.FbxImportUI()
    options.set_editor_property("automated_import_should_detect_type", False)
    options.set_editor_property("mesh_type_to_import", unreal.FBXImportType.FBXIT_ANIMATION)
    options.set_editor_property("import_mesh", False)
    options.set_editor_property("import_as_skeletal", True)
    options.set_editor_property("import_animations", True)
    options.set_editor_property("import_materials", False)
    options.set_editor_property("import_textures", False)
    options.set_editor_property("skeleton", skeleton)
    return options


def skeletal_measurement(mesh):
    bounds = mesh.get_imported_bounds()
    component = unreal.SkeletalMeshComponent()
    component.set_skinned_asset_and_update(mesh)
    skeleton = mesh.get_editor_property("skeleton")
    return {
        "object_path": mesh.get_path_name(),
        "class": mesh.get_class().get_name(),
        "skeleton": skeleton.get_path_name() if skeleton else None,
        "bone_count": component.get_num_bones(),
        "bounds_origin": list(bounds.origin.to_tuple()),
        "bounds_extent": list(bounds.box_extent.to_tuple()),
        "material_slots": len(mesh.get_editor_property("materials")),
    }


def animation_measurement(animation):
    skeleton = animation.get_editor_property("skeleton")
    return {
        "object_path": animation.get_path_name(),
        "class": animation.get_class().get_name(),
        "skeleton": skeleton.get_path_name() if skeleton else None,
        "play_length_seconds": animation.get_play_length(),
        "number_of_sampled_keys": int(animation.get_editor_property("number_of_sampled_keys")),
    }


def main():
    report = {
        "engine": unreal.SystemLibrary.get_engine_version(),
        "kind": "supplied_npc_mesh_texture_animation_intake_and_measurement_not_production_placement",
        "roles": [],
        "status": "ERROR",
    }

    try:
        for candidate in ROLES:
            for source in (candidate["mesh"], candidate["texture"], candidate["idle"], candidate["walk"]):
                assert_source_file(source)

            role_name = candidate["role"]
            destination = DEST_ROOT + "/" + role_name
            row = {
                "role": role_name,
                "source_pack": candidate["source_pack"],
                "source_files": {
                    label: {"path": candidate[label].as_posix(), "sha256": sha256(candidate[label])}
                    for label in ("mesh", "texture", "idle", "walk")
                },
            }

            texture, texture_imported = load_or_import(candidate["texture"], destination, "T_" + role_name)
            if not isinstance(texture, unreal.Texture2D):
                raise RuntimeError("Expected Texture2D for " + role_name)
            row["texture"] = {
                "object_path": texture.get_path_name(),
                "class": texture.get_class().get_name(),
                "imported_this_run": texture_imported,
            }

            mesh, mesh_imported = load_or_import(
                candidate["mesh"], destination, "SK_" + role_name, skeletal_import_options()
            )
            if not isinstance(mesh, unreal.SkeletalMesh):
                raise RuntimeError("Expected SkeletalMesh for " + role_name)
            row["mesh"] = skeletal_measurement(mesh)
            row["mesh"]["imported_this_run"] = mesh_imported

            if mesh.get_editor_property("skeleton") is None:
                raise RuntimeError("SkeletalMesh missing a skeleton for " + role_name)

            row["animations"] = {}
            for label, source in (("idle", candidate["idle"]), ("walk", candidate["walk"])):
                animation, animation_imported = load_or_import(
                    source,
                    destination,
                    "A_{}_{}".format(role_name, label.title()),
                    legacy_animation_import_options(mesh.get_editor_property("skeleton")),
                    unreal.FbxFactory(),
                )
                if not isinstance(animation, unreal.AnimSequence):
                    raise RuntimeError("Expected AnimSequence for {} {}".format(role_name, label))
                measurement = animation_measurement(animation)
                measurement["imported_this_run"] = animation_imported
                if measurement["skeleton"] != mesh.get_editor_property("skeleton").get_path_name():
                    raise RuntimeError("Animation skeleton mismatch for {} {}".format(role_name, label))
                if measurement["play_length_seconds"] <= 0.0 or measurement["number_of_sampled_keys"] <= 0:
                    raise RuntimeError("Animation has no sampled playback data for {} {}".format(role_name, label))
                row["animations"][label] = measurement

            row["source_animation_files"] = [
                {
                    "kind": label,
                    "path": source.as_posix(),
                    "sha256": sha256(source),
                    "native_animation_import": "PASS_LEGACY_FBX_FACTORY_ANIM_SEQUENCE",
                }
                for label, source in (("idle", candidate["idle"]), ("walk", candidate["walk"]))
            ]

            report["roles"].append(row)
            unreal.log("[WP12_NPC_INTAKE] {} mesh={} bones={} texture=PASS idle_walk=AnimSequence".format(
                role_name, row["mesh"]["object_path"], row["mesh"]["bone_count"]
            ))

        report["status"] = "PASS_SCOPED_NPC_MESH_TEXTURE_ANIMATION_INTAKE_AND_MEASUREMENT"
    except Exception:
        report["error"] = traceback.format_exc()
        raise
    finally:
        OUTPUT.parent.mkdir(parents=True, exist_ok=True)
        OUTPUT.write_text(json.dumps(report, indent=2), encoding="utf-8")


if __name__ == "__main__":
    main()
