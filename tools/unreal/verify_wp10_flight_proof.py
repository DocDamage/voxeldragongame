"""WP-10 Green Dragon Riding, Flight Locomotion, Obstacle Collision & Mounted Persistence PIE Proof."""
import json
from pathlib import Path
import time
import traceback
import unreal


ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
REPORT_PATH = ROOT / "Saved/Diagnostics/WP10_flight_proof.json"
report = {
    "kind": "wp10_dragon_flight_mounted_persistence_pie",
    "engine": unreal.SystemLibrary.get_engine_version(),
    "status": "INITIALIZING",
    "tests": {
        "DRG-05.MountAndDismount": {"status": "NOT_RUN"},
        "DRG-06.FlightLocomotionAndCamera": {"status": "NOT_RUN"},
        "DRG-07.MountedDefeatEmergencyRecovery": {"status": "NOT_RUN"},
        "DRG-14.HubCompanionRecovery": {"status": "NOT_RUN"},
        "SAVE-09.MountedFlightSaveAndRecovery": {"status": "NOT_RUN"},
    },
    "details": {},
    "logs": [],
}


def log(message):
    line = f"[{time.strftime('%H:%M:%S')}] {message}"
    print(line)
    report["logs"].append(line)


def write_report():
    REPORT_PATH.parent.mkdir(parents=True, exist_ok=True)
    REPORT_PATH.write_text(json.dumps(report, indent=2, default=str) + "\n", encoding="utf-8")


def finish_test(name, passed, details):
    report["tests"][name]["status"] = "PASS" if passed else "FAIL"
    report["details"][name] = details
    log(f"Test {name}: {'PASS' if passed else 'FAIL'}")
    if not passed:
        raise RuntimeError(f"{name} failed: {details}")


def eval_ue_result(res):
    """Evaluates Unreal Engine Python return value for methods with out-parameters or bools."""
    if res is None:
        return False
    if isinstance(res, bool):
        return res
    # If not None and not bool, UE Python returned out-param(s) indicating success (true)
    return True


write_report()

# Build blank map fixture with diagnostic ground plane
unreal.EditorLoadingAndSavingUtils.new_blank_map(False)
editor_actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
cube_mesh = unreal.load_asset("/Engine/BasicShapes/Cube.Cube")
floor_actor = editor_actor_subsystem.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(0.0, 0.0, -10.0))
if floor_actor and cube_mesh:
    mesh_comp = floor_actor.get_component_by_class(unreal.StaticMeshComponent)
    if mesh_comp:
        mesh_comp.set_static_mesh(cube_mesh)
        mesh_comp.set_collision_profile_name("BlockAll")
    floor_actor.set_actor_scale3d(unreal.Vector(100.0, 100.0, 0.2))

level_editor = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
started_at = time.monotonic()
stage = "init"
finished = False


def pie_tick(_delta_seconds):
    global stage, finished
    try:
        if finished:
            if not level_editor.is_in_play_in_editor():
                unreal.unregister_slate_post_tick_callback(tick_handle)
                log("PIE closed cleanly; exiting editor.")
                unreal.SystemLibrary.quit_editor()
            return

        if time.monotonic() - started_at > 90:
            raise RuntimeError(f"Timed out in PIE stage: {stage}")

        world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
        if not world or not level_editor.is_in_play_in_editor():
            return

        if stage == "init":
            pc = unreal.GameplayStatics.get_player_controller(world, 0)
            if not pc:
                return
            wyrm_pc = unreal.WyrmPlayerController.cast(pc)
            if not wyrm_pc:
                return

            pawn = wyrm_pc.get_controlled_pawn()
            if not pawn:
                return
            player = unreal.WyrmCharacter.cast(pawn)
            if not player:
                return

            player.set_actor_location(unreal.Vector(0.0, 0.0, 100.0), False, False)

            # Spawn allied dragon in True Form
            dragon_spawn_tf = unreal.Transform(
                location=unreal.Vector(0.0, 0.0, 100.0),
                rotation=unreal.Rotator(0.0, 0.0, 0.0),
                scale=unreal.Vector(1.0, 1.0, 1.0)
            )
            dragon = unreal.WyrmDragonCharacter.spawn_wyrm_dragon(
                world, unreal.WyrmDragonRole.ALLIED_COMPANION, dragon_spawn_tf)
            if not dragon:
                raise RuntimeError("Failed to spawn WyrmDragonCharacter in PIE")

            dragon.bond_with_humanoid(player)

            # -------------------------------------------------------------
            # TEST 1: DRG-05.MountAndDismount
            # -------------------------------------------------------------
            dragon.set_dragon_form(unreal.WyrmDragonForm.COMPANION_FORM)
            compact_mount_res = dragon.can_mount(player)
            compact_can_mount = eval_ue_result(compact_mount_res)

            # Switch to True Form and mount
            dragon.set_dragon_form(unreal.WyrmDragonForm.TRUE_FORM)
            true_mount_res = dragon.can_mount(player)
            true_can_mount = eval_ue_result(true_mount_res)

            mount_success = dragon.mount_humanoid(player)
            rider_mounted = dragon.is_rider_mounted()
            mounted_rider = dragon.get_mounted_rider()
            humanoid_movement_locked = player.is_movement_locked()
            pc_possesses_dragon = (wyrm_pc.get_controlled_pawn() == dragon)

            # Ground dismount
            dismount_can_res = dragon.can_dismount()
            can_dismount_ground = eval_ue_result(dismount_can_res)
            dismount_res = dragon.dismount_humanoid()
            dismount_success = eval_ue_result(dismount_res)
            rider_unmounted = not dragon.is_rider_mounted()
            humanoid_movement_unlocked = not player.is_movement_locked()
            pc_possesses_player = (wyrm_pc.get_controlled_pawn() == player)

            mount_dismount_passed = (
                not compact_can_mount
                and true_can_mount
                and mount_success
                and rider_mounted
                and mounted_rider == player
                and humanoid_movement_locked
                and pc_possesses_dragon
                and can_dismount_ground
                and dismount_success
                and rider_unmounted
                and humanoid_movement_unlocked
                and pc_possesses_player
            )

            finish_test("DRG-05.MountAndDismount", mount_dismount_passed, {
                "compact_can_mount": compact_can_mount,
                "true_can_mount": true_can_mount,
                "mount_success": mount_success,
                "rider_mounted": rider_mounted,
                "humanoid_movement_locked": humanoid_movement_locked,
                "pc_possesses_dragon": pc_possesses_dragon,
                "can_dismount_ground": can_dismount_ground,
                "dismount_success": dismount_success,
                "pc_possesses_player": pc_possesses_player,
            })

            # -------------------------------------------------------------
            # TEST 2: DRG-06.FlightLocomotionAndCamera
            # -------------------------------------------------------------
            dragon.mount_humanoid(player)
            takeoff_can_res = dragon.can_take_off()
            can_take_off = eval_ue_result(takeoff_can_res)
            takeoff_success = dragon.take_off()

            flight_state_flying = (dragon.get_flight_state() == unreal.WyrmDragonFlightState.FLYING)
            is_in_flight = dragon.is_in_flight()
            movement_mode_flying = (dragon.character_movement.movement_mode == unreal.MovementMode.MOVE_FLYING)
            max_fly_speed = dragon.character_movement.max_fly_speed

            # In-flight dismount must be rejected
            inflight_dismount_res = dragon.can_dismount()
            can_dismount_inflight = eval_ue_result(inflight_dismount_res)

            # Dual camera toggle
            boom = dragon.get_component_by_class(unreal.SpringArmComponent)
            dragon.set_flight_camera_mode(True)
            topdown_camera_active = bool(boom and boom.target_arm_length == 1800.0)
            dragon.set_flight_camera_mode(False)
            thirdperson_camera_active = bool(boom and boom.target_arm_length == 1100.0)

            # Landing
            dragon.set_actor_location(unreal.Vector(0.0, 0.0, 300.0), False, False)
            land_can_res = dragon.can_land()
            can_land = eval_ue_result(land_can_res)
            land_success = dragon.land()
            flight_state_grounded = (dragon.get_flight_state() == unreal.WyrmDragonFlightState.GROUNDED)
            movement_mode_walking = (dragon.character_movement.movement_mode == unreal.MovementMode.MOVE_WALKING)

            flight_passed = (
                can_take_off
                and takeoff_success
                and flight_state_flying
                and is_in_flight
                and movement_mode_flying
                and max_fly_speed == 1600.0
                and not can_dismount_inflight
                and topdown_camera_active
                and thirdperson_camera_active
                and can_land
                and land_success
                and flight_state_grounded
                and movement_mode_walking
            )

            finish_test("DRG-06.FlightLocomotionAndCamera", flight_passed, {
                "can_take_off": can_take_off,
                "takeoff_success": takeoff_success,
                "flight_state_flying": flight_state_flying,
                "max_fly_speed": max_fly_speed,
                "can_dismount_inflight": can_dismount_inflight,
                "topdown_camera_active": topdown_camera_active,
                "thirdperson_camera_active": thirdperson_camera_active,
                "can_land": can_land,
                "land_success": land_success,
            })

            # -------------------------------------------------------------
            # TEST 3: DRG-07.MountedDefeatEmergencyRecovery
            # -------------------------------------------------------------
            dragon.take_off()
            dragon.set_actor_location(unreal.Vector(0.0, 0.0, 1500.0), False, False)
            dragon_altitude_before_defeat = dragon.get_actor_location().z

            # Trigger mounted defeat emergency recovery
            dragon.handle_mounted_defeat()

            defeat_rider_unmounted = not dragon.is_rider_mounted()
            defeat_pc_possesses_player = (wyrm_pc.get_controlled_pawn() == player)
            defeat_movement_unlocked = not player.is_movement_locked()
            defeat_role_recovering = (dragon.get_dragon_role() == unreal.WyrmDragonRole.RECOVERING)
            defeat_flight_grounded = (dragon.get_flight_state() == unreal.WyrmDragonFlightState.GROUNDED)

            # Rider must be on safe ground, not stranded in midair
            player_loc_after_defeat = player.get_actor_location()
            rider_safely_on_ground = player_loc_after_defeat.z < 250.0

            # Mounting while recovering must be rejected
            mount_recovering_res = dragon.can_mount(player)
            can_mount_recovering = eval_ue_result(mount_recovering_res)

            defeat_recovery_passed = (
                defeat_rider_unmounted
                and defeat_pc_possesses_player
                and defeat_movement_unlocked
                and defeat_role_recovering
                and defeat_flight_grounded
                and rider_safely_on_ground
                and not can_mount_recovering
            )

            finish_test("DRG-07.MountedDefeatEmergencyRecovery", defeat_recovery_passed, {
                "dragon_altitude_before_defeat": dragon_altitude_before_defeat,
                "player_z_after_defeat": player_loc_after_defeat.z,
                "rider_safely_on_ground": rider_safely_on_ground,
                "defeat_role_recovering": defeat_role_recovering,
                "can_mount_recovering": can_mount_recovering,
            })

            # -------------------------------------------------------------
            # TEST 4: DRG-14.HubCompanionRecovery
            # -------------------------------------------------------------
            recover_success = dragon.recover_companion()
            role_allied = (dragon.get_dragon_role() == unreal.WyrmDragonRole.ALLIED_COMPANION)
            restored_hp = dragon.get_attributes().get_current_health()
            restored_focus = dragon.get_attributes().get_current_focus()
            flight_state_after_recover = (dragon.get_flight_state() == unreal.WyrmDragonFlightState.GROUNDED)

            # Can mount again after recovery
            post_recover_mount_res = dragon.can_mount(player)
            can_mount_after_recovery = eval_ue_result(post_recover_mount_res)

            hub_recovery_passed = (
                recover_success
                and role_allied
                and restored_hp == 420.0
                and restored_focus == 100.0
                and flight_state_after_recover
                and can_mount_after_recovery
            )

            finish_test("DRG-14.HubCompanionRecovery", hub_recovery_passed, {
                "recover_success": recover_success,
                "role_allied": role_allied,
                "restored_hp": restored_hp,
                "restored_focus": restored_focus,
                "can_mount_after_recovery": can_mount_after_recovery,
            })

            # -------------------------------------------------------------
            # TEST 5: SAVE-09.MountedFlightSaveAndRecovery
            # -------------------------------------------------------------
            dragon.mount_humanoid(player)
            dragon.take_off()
            dragon.set_actor_location(unreal.Vector(400.0, 500.0, 800.0), False, False)

            save_slot = "WyrmSlot_Flight_PIE_Proof"
            snapshot = unreal.WyrmSaveSubsystem.create_snapshot_object(
                save_slot, player, None, world)
            if not snapshot:
                raise RuntimeError("Failed to create snapshot object in PIE")

            rec = snapshot.dragon_record
            saved_mounted = getattr(rec, "is_rider_mounted", getattr(rec, "b_is_rider_mounted", False))
            saved_flight_state = rec.flight_state
            saved_loc = rec.world_location

            # Alter dragon state
            dragon.dismount_humanoid()
            dragon.set_actor_location(unreal.Vector(0.0, 0.0, 100.0), False, False)

            # Apply snapshot
            apply_success = unreal.WyrmSaveSubsystem.apply_snapshot_object(
                snapshot, player, None, world)

            restored_flight_state = dragon.get_flight_state()
            restored_loc = dragon.get_actor_location()
            restored_mounted = dragon.is_rider_mounted()
            restored_pawn = wyrm_pc.get_controlled_pawn()

            # Clean dismount and actor cleanup
            dismount_clean_res = dragon.dismount_humanoid()
            dragon.destroy_actor()

            save_recovery_passed = (
                saved_mounted
                and saved_flight_state == unreal.WyrmDragonFlightState.FLYING
                and saved_loc.z == 800.0
                and apply_success
                and restored_flight_state == unreal.WyrmDragonFlightState.FLYING
                and abs(restored_loc.z - 800.0) < 5.0
                and restored_mounted
                and restored_pawn == dragon
            )

            finish_test("SAVE-09.MountedFlightSaveAndRecovery", save_recovery_passed, {
                "saved_mounted": saved_mounted,
                "saved_flight_state": str(saved_flight_state),
                "saved_z": saved_loc.z,
                "apply_success": apply_success,
                "restored_flight_state": str(restored_flight_state),
                "restored_z": restored_loc.z,
                "restored_mounted": restored_mounted,
                "restored_pawn_is_dragon": (restored_pawn == dragon),
            })

            log("ALL WP-10 GREEN DRAGON FLIGHT & PERSISTENCE PIE TESTS PASSED!")
            report["status"] = "PASS"
            write_report()
            finished = True
            level_editor.editor_request_end_play()
            return

    except Exception:
        report["status"] = "ERROR"
        report["error"] = traceback.format_exc()
        log("PIE ERROR: " + report["error"])
        write_report()
        finished = True
        level_editor.editor_request_end_play()


tick_handle = unreal.register_slate_post_tick_callback(pie_tick)
level_editor.editor_request_begin_play()
