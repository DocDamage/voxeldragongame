import unreal
import json
import os

def main():
    report = {
        "meshes": [],
        "animations": [],
        "skeleton": None,
        "root_mesh": None
    }
    
    skel_path = "/Game/WYRMFALL/Development/Intake/WP00/GreenDragon/Green_Dragon/SkeletalMeshes/Hip-Local_Skeleton.Hip-Local_Skeleton"
    skeleton = unreal.EditorAssetLibrary.load_asset(skel_path)
    if skeleton:
        report["skeleton"] = skel_path
        print(f"Loaded skeleton: {skel_path}")

    # Inspect skeletal meshes
    asset_reg = unreal.AssetRegistryHelpers.get_asset_registry()
    filter = unreal.ARFilter(
        package_paths=["/Game/WYRMFALL/Development/Intake/WP00/GreenDragon/Green_Dragon/SkeletalMeshes"],
        recursive_paths=False,
        include_only_on_disk_assets=True
    )
    
    asset_data_list = asset_reg.get_assets_by_paths(["/Game/WYRMFALL/Development/Intake/WP00/GreenDragon/Green_Dragon/SkeletalMeshes"], False)
    
    for ad in asset_data_list:
        asset_class = str(ad.asset_class_path.asset_name)
        asset_name = str(ad.asset_name)
        obj_path = str(ad.package_name) + "." + asset_name
        if asset_class == "SkeletalMesh":
            mesh_obj = unreal.EditorAssetLibrary.load_asset(obj_path)
            bounds = mesh_obj.get_bounds() if mesh_obj else None
            box_extent = [bounds.box_extent.x, bounds.box_extent.y, bounds.box_extent.z] if bounds else None
            report["meshes"].append({
                "name": asset_name,
                "path": obj_path,
                "box_extent": box_extent
            })
        elif asset_class == "AnimSequence":
            anim_obj = unreal.EditorAssetLibrary.load_asset(obj_path)
            num_frames = anim_obj.get_editor_property("number_of_sampled_keys") if anim_obj else 0
            seq_len = anim_obj.get_play_length() if anim_obj else 0.0
            report["animations"].append({
                "name": asset_name,
                "path": obj_path,
                "length": seq_len
            })

    # Find the largest/central mesh or check Hip-Local
    print(f"Total SkeletalMeshes found: {len(report['meshes'])}")
    print(f"Total AnimSequences found: {len(report['animations'])}")
    
    out_dir = "G:/assets/voxel project/Saved/Diagnostics"
    os.makedirs(out_dir, exist_ok=True)
    out_file = os.path.join(out_dir, "WP09_dragon_asset_inspection.json")
    with open(out_file, "w") as f:
        json.dump(report, f, indent=2)
    print(f"Saved inspection report to {out_file}")

if __name__ == "__main__":
    main()
