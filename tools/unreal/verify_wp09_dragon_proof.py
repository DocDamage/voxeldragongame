"""WP-09 Green Dragon Locomotion, Combat, Direct Control & Save Roundtrip PIE Proof."""
import json
from pathlib import Path
import time
import traceback
import unreal


ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
REPORT_PATH = ROOT / "Saved/Diagnostics/WP09_dragon_proof.json"
report = {
    "kind": "wp09_dragon_locomotion_combat_direct_control_pie",
    "engine": unreal.SystemLibrary.get_engine_version(),
    "status": "INITIALIZING",
    "tests": {
        "ASSET.DragonAssets": {"status": "NOT_RUN"},
        "DRG-01.LivingDefeatAndBond": {"status": "NOT_RUN"},
        "DRG-02.CompanionCombat": {"status": "NOT_RUN"},
        "DRG-03_DRG-04.DirectControlAndTether": {"status": "NOT_RUN"},
        "SAVE-08.DragonSaveRoundtrip": {"status": "NOT_RUN"},
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


write_report()

# Build blank map fixture
unreal.EditorLoadingAndSavingUtils.new_blank_map(False)
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

            # -------------------------------------------------------------
            # TEST 1: ASSET.DragonAssets
            # -------------------------------------------------------------
            leader_path = "/Game/WYRMFALL/Development/Intake/WP00/GreenDragon/Green_Dragon/SkeletalMeshes/Hip-Local"
            leader_mesh = unreal.load_asset(leader_path)
            if not leader_mesh:
                raise RuntimeError(f"Leader mesh missing: {leader_path}")

            skeleton_path = "/Game/WYRMFALL/Development/Intake/WP00/GreenDragon/Green_Dragon/SkeletalMeshes/Hip-Local_Skeleton"
            skeleton = unreal.load_asset(skeleton_path)
            if not skeleton:
                raise RuntimeError(f"Skeleton missing: {skeleton_path}")

            sample_anims = [
                "/Game/WYRMFALL/Development/Intake/WP00/GreenDragon/Green_Dragon/SkeletalMeshes/Green_DragonFlying_01",
                "/Game/WYRMFALL/Development/Intake/WP00/GreenDragon/Green_Dragon/SkeletalMeshes/Green_DragonAttack_01",
                "/Game/WYRMFALL/Development/Intake/WP00/GreenDragon/Green_Dragon/SkeletalMeshes/Green_DragonCombo_Attack_01",
                "/Game/WYRMFALL/Development/Intake/WP00/GreenDragon/Green_Dragon/SkeletalMeshes/Green_DragonIdle_01",
            ]
            loaded_anims = []
            for a in sample_anims:
                obj = unreal.load_asset(a)
                if obj:
                    loaded_anims.append(a)

            follower_parts = [
                "Chest-Local", "Claw-Local", "Head-Local", "Jaw-Local",
                "L_Wing1-Local", "R_Wing1-Local", "Tail_1-Local"
            ]
            loaded_parts = []
            for p in follower_parts:
                mesh = unreal.load_asset(f"/Game/WYRMFALL/Development/Intake/WP00/GreenDragon/Green_Dragon/SkeletalMeshes/{p}")
                if mesh:
                    loaded_parts.append(p)

            finish_test("ASSET.DragonAssets", bool(leader_mesh and skeleton and len(loaded_anims) >= 4 and len(loaded_parts) >= 7), {
                "leader_mesh": leader_path,
                "skeleton": skeleton_path,
                "verified_anims": len(loaded_anims),
                "verified_parts": len(loaded_parts),
            })

            # -------------------------------------------------------------
            # TEST 2: DRG-01.LivingDefeatAndBond
            # -------------------------------------------------------------
            dragon_spawn_tf = unreal.Transform(
                location=unreal.Vector(200.0, 0.0, 100.0),
                rotation=unreal.Rotator(0.0, 0.0, 0.0),
                scale=unreal.Vector(1.0, 1.0, 1.0)
            )
            dragon = unreal.WyrmDragonCharacter.spawn_wyrm_dragon(
                world, unreal.WyrmDragonRole.HOSTILE_BOSS, dragon_spawn_tf)
            if not dragon:
                raise RuntimeError("Failed to spawn WyrmDragonCharacter in PIE")

            initial_role = dragon.get_dragon_role()
            initial_max_hp = dragon.get_attributes().get_current_max_health()
            initial_hp = dragon.get_attributes().get_current_health()
            has_bond_init = dragon.has_bond_receipt()

            # Trigger living defeat
            defeat_ok = dragon.perform_boss_defeat()
            defeated_role = dragon.get_dragon_role()
            defeated_hp = dragon.get_attributes().get_current_health()
            dragon_still_alive_actor = unreal.SystemLibrary.is_valid(dragon)

            # Perform one-way bond conversion
            bond_ok = dragon.bond_with_humanoid(player)
            allied_role = dragon.get_dragon_role()
            bonded_has_receipt = dragon.has_bond_receipt()
            allied_max_hp = dragon.get_attributes().get_current_max_health()
            allied_hp = dragon.get_attributes().get_current_health()
            allied_focus = dragon.get_attributes().get_current_max_focus()

            # Repeat bond attempt should be rejected with zero change
            repeat_bond = dragon.bond_with_humanoid(player)
            hp_after_repeat = dragon.get_attributes().get_current_health()

            living_bond_passed = (
                initial_max_hp == 1800.0
                and initial_hp == 1800.0
                and not has_bond_init
                and defeat_ok
                and defeated_role == unreal.WyrmDragonRole.DEFEATED_ALIVE
                and defeated_hp == 0.0
                and dragon_still_alive_actor
                and bond_ok
                and allied_role == unreal.WyrmDragonRole.ALLIED_COMPANION
                and bonded_has_receipt
                and allied_max_hp == 420.0
                and allied_hp == 210.0
                and allied_focus == 100.0
                and not repeat_bond
                and hp_after_repeat == 210.0
            )

            finish_test("DRG-01.LivingDefeatAndBond", living_bond_passed, {
                "initial_hp": initial_hp,
                "defeated_role": str(defeated_role),
                "defeated_hp": defeated_hp,
                "allied_role": str(allied_role),
                "allied_hp": allied_hp,
                "allied_max_hp": allied_max_hp,
                "repeat_bond_rejected": not repeat_bond,
            })

            # -------------------------------------------------------------
            # TEST 3: DRG-02.CompanionCombat
            # -------------------------------------------------------------
            dragon.issue_order(unreal.WyrmCompanionOrder.HOLD, None)
            order_hold = dragon.get_companion_order()

            dragon.issue_order(unreal.WyrmCompanionOrder.FOLLOW, None)
            order_follow = dragon.get_companion_order()

            # Spawn enemy
            enemy_tf = unreal.Transform(
                location=unreal.Vector(350.0, 0.0, 100.0),
                rotation=unreal.Rotator(0.0, 0.0, 0.0),
                scale=unreal.Vector(1.0, 1.0, 1.0)
            )
            enemy = unreal.WyrmEnemyCharacter.spawn_wyrm_enemy(
                world, unreal.WyrmEnemyRole.MELEE_CHASER, enemy_tf)
            if not enemy:
                raise RuntimeError("Failed to spawn enemy in PIE")

            enemy.get_attributes().set_current_armor(0.0)
            initial_enemy_hp = enemy.get_attributes().get_current_health()

            # Primary attack (24 damage)
            hit_primary = dragon.perform_primary_attack(enemy)
            enemy_hp_after_primary = enemy.get_attributes().get_current_health()

            # Secondary area attack (18 damage, 6s cooldown)
            hit_area = dragon.perform_secondary_attack(enemy)
            enemy_hp_after_area = enemy.get_attributes().get_current_health()
            cooldown_committed = dragon.get_area_attack_cooldown_remaining()

            # Repeat secondary attack should fail due to cooldown
            blocked_area = dragon.perform_secondary_attack(enemy)

            enemy.destroy_actor()

            combat_passed = (
                order_hold == unreal.WyrmCompanionOrder.HOLD
                and order_follow == unreal.WyrmCompanionOrder.FOLLOW
                and initial_enemy_hp == 60.0
                and hit_primary
                and enemy_hp_after_primary == 36.0
                and hit_area
                and enemy_hp_after_area == 18.0
                and cooldown_committed > 5.0
                and not blocked_area
            )

            finish_test("DRG-02.CompanionCombat", combat_passed, {
                "initial_enemy_hp": initial_enemy_hp,
                "enemy_hp_after_primary": enemy_hp_after_primary,
                "enemy_hp_after_area": enemy_hp_after_area,
                "cooldown_remaining": cooldown_committed,
                "second_attack_blocked": not blocked_area,
            })

            # -------------------------------------------------------------
            # TEST 4: DRG-03_DRG-04.DirectControlAndTether
            # -------------------------------------------------------------
            wyrm_pc = unreal.WyrmPlayerController.cast(pc)
            transfer_ok = wyrm_pc.transfer_control_to_dragon(dragon)
            pc_possesses_dragon = (wyrm_pc.get_controlled_pawn() == dragon)
            dragon_marked_controlled = dragon.is_directly_controlled()
            humanoid_movement_locked = player.is_movement_locked()
            humanoid_still_valid = unreal.SystemLibrary.is_valid(player)

            # Tether warning check at 125m (12,500 cm)
            dragon.set_actor_location(unreal.Vector(12500.0, 0.0, 100.0), False, False)
            dragon.check_tether_status()
            tether_warning_status = dragon.get_tether_status()
            still_controlling_during_warning = (wyrm_pc.get_controlled_pawn() == dragon)

            # Tether limit check at 151m (15,100 cm) -> stops and returns control
            dragon.set_actor_location(unreal.Vector(15100.0, 0.0, 100.0), False, False)
            dragon.check_tether_status()
            tether_limit_status = dragon.get_tether_status()
            control_returned_at_limit = (wyrm_pc.get_controlled_pawn() == player)

            # Re-transfer control for remote risk test
            dragon.set_actor_location(unreal.Vector(500.0, 0.0, 100.0), False, False)
            retransfer_ok = wyrm_pc.transfer_control_to_dragon(dragon)
            possessing_again = (wyrm_pc.get_controlled_pawn() == dragon)

            # Simulate damage to waiting humanoid body -> immediate return of control
            dragon.handle_waiting_body_damaged(10.0)
            control_returned_on_body_damage = (wyrm_pc.get_controlled_pawn() == player)
            humanoid_unlocked_on_return = not player.is_movement_locked()

            tether_and_risk_passed = (
                transfer_ok
                and pc_possesses_dragon
                and dragon_marked_controlled
                and humanoid_movement_locked
                and humanoid_still_valid
                and tether_warning_status == unreal.WyrmTetherStatus.WARNING
                and still_controlling_during_warning
                and tether_limit_status == unreal.WyrmTetherStatus.LIMIT_REACHED
                and control_returned_at_limit
                and retransfer_ok
                and possessing_again
                and control_returned_on_body_damage
                and humanoid_unlocked_on_return
            )

            finish_test("DRG-03_DRG-04.DirectControlAndTether", tether_and_risk_passed, {
                "transfer_ok": transfer_ok,
                "pc_possesses_dragon": pc_possesses_dragon,
                "humanoid_movement_locked": humanoid_movement_locked,
                "tether_warning_status": str(tether_warning_status),
                "tether_limit_status": str(tether_limit_status),
                "control_returned_at_limit": control_returned_at_limit,
                "control_returned_on_body_damage": control_returned_on_body_damage,
            })

            # -------------------------------------------------------------
            # TEST 5: SAVE-08.DragonSaveRoundtrip
            # -------------------------------------------------------------
            dragon.set_actor_location(unreal.Vector(400.0, 500.0, 100.0), False, False)
            dragon.get_attributes().set_current_health(310.0)
            wyrm_pc.transfer_control_to_dragon(dragon)

            save_slot = "WyrmSlot_Dragon_PIE_Proof"
            snapshot = unreal.WyrmSaveSubsystem.create_snapshot_object(
                save_slot, player, None, world)
            if not snapshot:
                raise RuntimeError("Failed to create snapshot object in PIE")

            saved_has_bond = getattr(snapshot.dragon_record, "has_bond_receipt", getattr(snapshot.dragon_record, "b_has_bond_receipt", False))
            saved_health = snapshot.dragon_record.health
            saved_controlled = getattr(snapshot.dragon_record, "is_directly_controlled", getattr(snapshot.dragon_record, "b_is_directly_controlled", False))
            saved_loc = snapshot.dragon_record.world_location

            # Alter dragon state
            dragon.set_actor_location(unreal.Vector(0.0, 0.0, 0.0), False, False)
            dragon.get_attributes().set_current_health(100.0)

            # Apply snapshot
            apply_ok = unreal.WyrmSaveSubsystem.apply_snapshot_object(
                snapshot, player, None, world)
            restored_hp = dragon.get_attributes().get_current_health()
            restored_loc = dragon.get_actor_location()
            restored_controlled = dragon.is_directly_controlled()
            restored_pawn = wyrm_pc.get_controlled_pawn()

            # Clean return
            wyrm_pc.return_control_to_humanoid()
            dragon.destroy_actor()

            save_roundtrip_passed = (
                saved_has_bond
                and saved_health == 310.0
                and saved_controlled
                and apply_ok
                and restored_hp == 310.0
                and abs(restored_loc.x - 400.0) < 5.0
                and abs(restored_loc.y - 500.0) < 5.0
                and restored_controlled
                and restored_pawn == dragon
            )

            finish_test("SAVE-08.DragonSaveRoundtrip", save_roundtrip_passed, {
                "saved_health": saved_health,
                "saved_controlled": saved_controlled,
                "apply_ok": apply_ok,
                "restored_hp": restored_hp,
                "restored_loc": f"({restored_loc.x:.1f}, {restored_loc.y:.1f}, {restored_loc.z:.1f})",
                "restored_controlled": restored_controlled,
            })

            log("ALL WP-09 DRAGON LOCOMOTION, COMBAT & CONTROL PIE TESTS PASSED!")
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
