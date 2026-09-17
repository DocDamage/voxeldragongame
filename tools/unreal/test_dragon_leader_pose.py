import unreal

def test_leader_pose():
    world = unreal.EditorLevelLibrary.get_editor_world()
    skel_path = "/Game/WYRMFALL/Development/Intake/WP00/GreenDragon/Green_Dragon/SkeletalMeshes/Hip-Local_Skeleton.Hip-Local_Skeleton"
    skeleton = unreal.EditorAssetLibrary.load_asset(skel_path)
    
    # Load all 44 meshes
    mesh_names = [
        "Chest-Local", "Claw-Local", "Detail3-1-Local", "Detail4-1-Local", "Detail5-1-Local", "Detail6-1-Local",
        "ear_L-Local", "ear_R-Local", "Head-Local", "Hip-Local", "horn1_ear1-2-Local", "horn1_ear1-3-Local",
        "horn2_ear1-2-Local", "horn2_ear1-3-Local", "Jaw-Local", "L_Wing1-Local", "L_Wing2-Local", "L_Wing3-Local",
        "LB_Claw-Local", "LB_Foot-Local", "LB_Leg1-Local", "LB_Leg2-Local", "LB_Thigh-Local", "LF_Claw-Local",
        "LF_Foot-Local", "LF_Leg-Local", "LF_Thigh-Local", "Mini2-1-Local", "Neck-Local", "R_Wing1-Local",
        "R_Wing2-Local", "R_Wing3-Local", "RB_Foot-Local", "RB_Leg1-Local", "RB_Leg2-Local", "RB_Thigh-Local",
        "RF_Claw-Local", "RF_Foot-Local", "RF_Leg-Local", "RF_Thigh-Local", "Tail_1-Local", "Tail_2-Local",
        "Tail_3-Local", "Tail_4-Local"
    ]
    
    base_path = "/Game/WYRMFALL/Development/Intake/WP00/GreenDragon/Green_Dragon/SkeletalMeshes/"
    meshes = {}
    for name in mesh_names:
        p = base_path + name + "." + name
        m = unreal.EditorAssetLibrary.load_asset(p)
        if m:
            meshes[name] = m
    print(f"Loaded {len(meshes)} meshes.")
    
    # Spawn actor
    actor = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.Actor, unreal.Vector(0, 0, 100))
    root_comp = unreal.SceneComponent(actor)
    actor.set_editor_property("root_component", root_comp)
    
    # Leader mesh (Hip-Local)
    leader_comp = unreal.SkeletalMeshComponent(actor)
    leader_comp.set_skeletal_mesh_asset(meshes["Hip-Local"])
    leader_comp.attach_to_component(root_comp, unreal.Name(), unreal.AttachmentRule.KEEP_RELATIVE, unreal.AttachmentRule.KEEP_RELATIVE, unreal.AttachmentRule.KEEP_RELATIVE, False)
    leader_comp.set_relative_scale3d(unreal.Vector(0.02, 0.02, 0.02))
    
    follower_comps = []
    for name, m in meshes.items():
        if name == "Hip-Local":
            continue
        comp = unreal.SkeletalMeshComponent(actor)
        comp.set_skeletal_mesh_asset(m)
        comp.attach_to_component(leader_comp, unreal.Name(), unreal.AttachmentRule.KEEP_RELATIVE, unreal.AttachmentRule.KEEP_RELATIVE, unreal.AttachmentRule.KEEP_RELATIVE, False)
        comp.set_leader_pose_component(leader_comp)
        follower_comps.append(comp)
        
    print(f"Created leader mesh and {len(follower_comps)} follower components with leader pose set.")
    
    # Play anim
    anim = unreal.EditorAssetLibrary.load_asset("/Game/WYRMFALL/Development/Intake/WP00/GreenDragon/Green_Dragon/SkeletalMeshes/Green_DragonIdle_01.Green_DragonIdle_01")
    if anim:
        leader_comp.play_animation(anim, True)
        print("Successfully played Green_DragonIdle_01 on leader pose component.")
        
    # Clean up test actor
    actor.destroy_actor()
    print("Destroyed test actor cleanly.")

if __name__ == "__main__":
    test_leader_pose()
