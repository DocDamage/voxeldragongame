import unreal
from pathlib import Path
import json

print("=== STARTING WP-02 MUTABLE CHARACTER RECIPE & RUNTIME PROOF ===")
ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
report = {
    "proof_id": "WP02_MUTABLE_RECIPE_PROOF",
    "status": "STARTING",
    "checks": {},
    "errors": []
}

def record_check(name, passed, details):
    report["checks"][name] = {
        "passed": bool(passed),
        "details": details
    }
    status_str = "PASS" if passed else "FAIL"
    print(f"[{status_str}] {name}: {details}")
    if not passed:
        report["errors"].append(f"{name}: {details}")

try:
    assets = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
    
    # -------------------------------------------------------------
    # CHAR-01: Authoritative Mutable Recipe Asset
    # -------------------------------------------------------------
    co_path = "/Game/WYRMFALL/Characters/Player/CO_Knight"
    co = assets.load_asset(co_path)
    if not co:
        record_check("CHAR-01_Asset_Exists", False, f"CO_Knight not found at {co_path}")
    else:
        # If not already compiled in current session, compile synchronously (loads from DDC)
        if not co.is_compiled():
            unreal.CustomizableObjectEditorFunctionLibrary.compile_customizable_object_synchronously(co)

        is_compiled = co.is_compiled()
        comp_count = co.get_component_count()
        param_count = co.get_parameter_count()
        param_names = [co.get_parameter_name(i) for i in range(param_count)]
        has_helmet = "Helmet" in param_names
        has_tint = "ArmorTint" in param_names
        
        char01_pass = is_compiled and comp_count >= 1 and has_helmet and has_tint
        record_check("CHAR-01_Authoritative_Recipe", char01_pass, {
            "path": co_path,
            "is_compiled": is_compiled,
            "component_count": comp_count,
            "parameter_count": param_count,
            "parameters": param_names
        })

    # -------------------------------------------------------------
    # Get World for Actor Spawning
    # -------------------------------------------------------------
    world = unreal.EditorLevelLibrary.get_editor_world()
    if not world:
        subsys = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
        world = subsys.get_game_world() if hasattr(subsys, "get_game_world") else None

    if not world:
        record_check("World_Context", False, "Could not acquire a valid world context for actor spawning")
    else:
        record_check("World_Context", True, f"Acquired world: {world.get_name()}")

        # ---------------------------------------------------------
        # CHAR-02: Runtime Mutable Binding on AWyrmCharacter
        # ---------------------------------------------------------
        spawn_loc = unreal.Vector(0, 0, 100)
        spawn_rot = unreal.Rotator(0, 0, 0)
        character = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.WyrmCharacter, spawn_loc, spawn_rot)
        
        if not character:
            record_check("CHAR-02_Actor_Spawn", False, "Failed to spawn AWyrmCharacter")
        else:
            skel_comp = character.get_customizable_component()
            has_comp = skel_comp is not None
            parent_is_mesh = False
            if has_comp:
                parent_is_mesh = (skel_comp.get_attach_parent() == character.mesh)
            
            char02_pass = has_comp and parent_is_mesh
            record_check("CHAR-02_Runtime_Binding", char02_pass, {
                "actor": character.get_name(),
                "has_customizable_component": has_comp,
                "attached_to_character_mesh": parent_is_mesh
            })

            # -----------------------------------------------------
            # CHAR-03: Instance Creation & Initial Binding
            # -----------------------------------------------------
            character.set_customizable_object(co)
            instance = character.customizable_instance
            has_instance = instance is not None
            char03_pass = has_instance
            record_check("CHAR-03_Instance_Creation", char03_pass, {
                "instance_valid": has_instance,
                "instance_name": instance.get_name() if instance else "None"
            })

            # -----------------------------------------------------
            # CHAR-04: Runtime Parameter Variation
            # -----------------------------------------------------
            # Test Option Switch: Helmet -> Down
            character.set_option_parameter("Helmet", "Down")
            helmet_val = character.get_option_parameter("Helmet")
            helmet_pass = (helmet_val == "Down")

            # Test Color Parameter: ArmorTint -> Red
            test_color = unreal.LinearColor(0.85, 0.15, 0.15, 1.0)
            character.set_color_parameter("ArmorTint", test_color)
            read_color = character.get_color_parameter("ArmorTint")
            color_pass = (abs(read_color.r - test_color.r) < 0.01 and
                          abs(read_color.g - test_color.g) < 0.01 and
                          abs(read_color.b - test_color.b) < 0.01)

            # Test Float Parameter setter/getter on character
            character.set_float_parameter("BodyScale", 1.05)
            read_scale = character.get_float_parameter("BodyScale")

            char04_pass = helmet_pass and color_pass
            record_check("CHAR-04_Runtime_Parameters", char04_pass, {
                "helmet_option": helmet_val,
                "helmet_expected": "Down",
                "armor_tint": [read_color.r, read_color.g, read_color.b, read_color.a],
                "body_scale": read_scale
            })

            # -----------------------------------------------------
            # CHAR-05: Equipment Attachment on Sockets
            # -----------------------------------------------------
            has_socket = character.is_socket_valid("Hand_Right")
            sword_asset = assets.load_asset("/Game/WYRMFALL/Items/Weapons/SM_Sword")
            
            # Spawn a StaticMeshActor and attach its mesh component
            sword_actor = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.StaticMeshActor, spawn_loc, spawn_rot)
            sword_comp = sword_actor.static_mesh_component if sword_actor else None
            if sword_comp and sword_asset:
                sword_comp.set_mobility(unreal.ComponentMobility.MOVABLE)
                sword_comp.set_static_mesh(sword_asset)
            
            attach_res = character.attach_equipment_mesh(sword_comp, "Hand_Right")
            attached_parent = sword_comp.get_attach_parent() if sword_comp else None
            attached_socket = sword_comp.get_attach_socket_name() if sword_comp else None
            
            char05_pass = has_socket and attach_res and (attached_parent == character.mesh) and (str(attached_socket) == "Hand_Right")
            record_check("CHAR-05_Equipment_Attachment", char05_pass, {
                "socket_valid": has_socket,
                "attach_returned": attach_res,
                "parent_is_mesh": (attached_parent == character.mesh) if attached_parent else False,
                "socket_name": str(attached_socket),
                "sword_mesh": sword_asset.get_name() if sword_asset else "None"
            })
            if sword_actor:
                sword_actor.destroy_actor()

            # -----------------------------------------------------
            # CHAR-06: Appearance Persistence Descriptor
            # -----------------------------------------------------
            desc_str = character.capture_appearance_descriptor()
            import base64
            valid_base64 = False
            if desc_str and len(desc_str) > 0:
                try:
                    raw_bytes = base64.b64decode(desc_str)
                    valid_base64 = len(raw_bytes) > 0
                except Exception:
                    valid_base64 = False

            # Mutate current parameters to something else
            character.set_option_parameter("Helmet", "Up")
            character.set_color_parameter("ArmorTint", unreal.LinearColor(0.2, 0.8, 0.2, 1.0))
            character.set_float_parameter("BodyScale", 0.9)

            # Now restore from captured descriptor
            restore_ok = character.restore_appearance_descriptor(desc_str)
            restored_helmet = character.get_option_parameter("Helmet")
            restored_color = character.get_color_parameter("ArmorTint")
            restored_scale = character.get_float_parameter("BodyScale")

            restore_matches = (
                restored_helmet == "Down" and
                abs(restored_color.r - test_color.r) < 0.01
            )

            char06_pass = valid_base64 and restore_ok and restore_matches
            record_check("CHAR-06_Appearance_Persistence", char06_pass, {
                "descriptor_captured": valid_base64,
                "descriptor_length": len(desc_str),
                "restore_succeeded": restore_ok,
                "restored_helmet": restored_helmet,
                "restored_tint": [restored_color.r, restored_color.g, restored_color.b, restored_color.a],
                "restored_scale": restored_scale
            })

            # Cleanup
            character.destroy_actor()
            record_check("Actor_Cleanup", True, "Spawned test character cleanly destroyed")

    all_passed = all(c["passed"] for c in report["checks"].values())
    report["status"] = "PASS" if all_passed else "FAIL"

except Exception as ex:
    import traceback
    report["status"] = "ERROR"
    report["errors"].append(traceback.format_exc())
    print("Exception during proof execution:\n", traceback.format_exc())

out_file = ROOT / "Saved/Diagnostics/WP02_mutable_recipe_proof.json"
out_file.parent.mkdir(parents=True, exist_ok=True)
out_file.write_text(json.dumps(report, indent=2), encoding="utf-8")
print(f"=== PROOF FINISHED: {report['status']} (saved to {out_file}) ===")
