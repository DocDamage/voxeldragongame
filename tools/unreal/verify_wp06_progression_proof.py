"""
Headless PIE Verification Suite for WP-06: Second Build and Progression Fixture
Tests acceptance criteria COM-08:
  COM-08: Second distinct combat build (Ranged Skirmisher vs Melee Knight),
          authoritative projectile combat, weapon family gating, dynamic kit switching,
          GAS-authoritative Level/XP progression, and dual-camera parity.

Run via:
  UnrealEditor-Cmd.exe WYRMFALL.uproject -run=pythonscript -script="G:/assets/voxel project/tools/unreal/verify_wp06_progression_proof.py" -stdout -FullStdOutLogOutput -unattended -nopause -nosplash -nullrhi
"""

import unreal
import json
import os
import sys

def log(msg):
    unreal.log(f"[WP06_PROGRESSION_PROOF] {msg}")

def main():
    log("Starting WP-06 Progression & Archetype Proof Suite...")
    results = {}
    passed = True

    editor_subsystem = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
    world = editor_subsystem.get_editor_world() if editor_subsystem else unreal.EditorLevelLibrary.get_editor_world()
    if not world:
        log("ERROR: Could not get editor world.")
        sys.exit(1)

    # -------------------------------------------------------------
    # SETUP: Spawn Player Character
    # -------------------------------------------------------------
    player_loc = unreal.Vector(0.0, 0.0, 100.0)
    player = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.WyrmCharacter, player_loc)
    if not player:
        log("ERROR: Failed to spawn WyrmCharacter")
        sys.exit(1)

    player.grant_combat_abilities()
    inv = player.get_inventory()
    attrs = player.get_attributes()
    asc = player.get_ability_system()
    if not inv or not attrs or not asc:
        log("ERROR: Missing core components on WyrmCharacter")
        player.destroy_actor()
        sys.exit(1)

    # =============================================================
    # CASE 1: Progression Formula & Level-Up Fixture
    # =============================================================
    log("Running Case 1: GAS-Authoritative Progression & Attribute Scaling...")
    # Initial state: Level 1, 0 XP
    lvl1 = player.get_character_level()
    xp1 = player.get_current_xp()
    req1 = player.get_xp_to_next_level()
    pwr1 = attrs.get_current_power()
    hp1 = attrs.get_current_max_health()

    log(f"Level 1: Level={lvl1}, XP={xp1}/{req1}, Power={pwr1}, MaxHealth={hp1}")
    
    # Verify baseline level 1 formulas: XPNeeded=100, Power=20, MaxHealth=100
    lvl1_ok = (lvl1 == 1.0 and xp1 == 0.0 and req1 == 100.0 and abs(pwr1 - 20.0) < 0.01 and abs(hp1 - 100.0) < 0.01)

    # Add 60 XP: Should stay level 1, CurrentXP=60
    player.add_experience(60.0)
    lvl_after_60 = player.get_character_level()
    xp_after_60 = player.get_current_xp()
    log(f"After +60 XP: Level={lvl_after_60}, XP={xp_after_60}")
    xp_partial_ok = (lvl_after_60 == 1.0 and abs(xp_after_60 - 60.0) < 0.01)

    # Add 50 XP: Total 110 XP -> Crosses 100 threshold!
    # Should level up to Level 2 with 10 leftover XP
    # Level 2 XPNeeded = 100 + 50*(2-1) = 150.
    # Level 2 Power = 20 + 3*1 = 23. MaxHealth = 100 + 8*1 = 108.
    player.add_experience(50.0)
    lvl2 = player.get_character_level()
    xp2 = player.get_current_xp()
    req2 = player.get_xp_for_next_level()
    to_next2 = player.get_xp_to_next_level()
    pwr2 = attrs.get_current_power()
    hp2 = attrs.get_current_max_health()
    log(f"After +50 XP (Total 110): Level={lvl2}, XP={xp2}/{req2} (Remaining={to_next2}), Power={pwr2}, MaxHealth={hp2}")

    lvl2_ok = (
        lvl2 == 2.0 and abs(xp2 - 10.0) < 0.01 and req2 == 150.0 and to_next2 == 140.0 and
        abs(pwr2 - 23.0) < 0.01 and abs(hp2 - 108.0) < 0.01
    )

    progression_pass = lvl1_ok and xp_partial_ok and lvl2_ok
    results["COM-08_Progression"] = {
        "status": "PASS" if progression_pass else "FAIL",
        "lvl1_ok": lvl1_ok,
        "xp_partial_ok": xp_partial_ok,
        "lvl2_ok": lvl2_ok,
        "power_level2": pwr2,
        "max_health_level2": hp2
    }
    if not progression_pass:
        passed = False

    # =============================================================
    # CASE 2: Weapon Family Gating & Dynamic Kit Switching
    # =============================================================
    log("Running Case 2: Weapon Family Gating & Dynamic Kit Switching...")
    # Roll a Melee 1H Sword and a Ranged Bow
    sword = unreal.WyrmInventoryComponent.roll_random_item("KnightBlade", unreal.WyrmItemType.WEAPON, 1, unreal.WyrmEquipSlot.MAIN_HAND)
    sword.weapon_family = unreal.WyrmWeaponFamily.MELEE1H

    bow = unreal.WyrmInventoryComponent.roll_random_item("RangerBow", unreal.WyrmItemType.WEAPON, 1, unreal.WyrmEquipSlot.MAIN_HAND)
    bow.weapon_family = unreal.WyrmWeaponFamily.RANGED_BOW

    inv.add_item(sword)
    inv.add_item(bow)

    # Initial kit (unarmed):
    init_kit = player.get_active_weapon_family()
    log(f"Initial Weapon Family: {init_kit}")

    # Equip Melee Sword -> Kit should become Melee1H
    inv.equip_item(sword.instance_id, unreal.WyrmEquipSlot.MAIN_HAND)
    melee_kit = player.get_active_weapon_family()
    log(f"Equipped Sword -> Weapon Family: {melee_kit}")
    melee_kit_ok = (melee_kit == unreal.WyrmWeaponFamily.MELEE1H)

    # Equip Ranged Bow -> Kit should become RangedBow
    inv.equip_item(bow.instance_id, unreal.WyrmEquipSlot.MAIN_HAND)
    ranged_kit = player.get_active_weapon_family()
    log(f"Equipped Bow -> Weapon Family: {ranged_kit}")
    ranged_kit_ok = (ranged_kit == unreal.WyrmWeaponFamily.RANGED_BOW)

    # Unequip -> Kit returns to Unarmed
    inv.unequip_item(unreal.WyrmEquipSlot.MAIN_HAND)
    unequip_kit = player.get_active_weapon_family()
    log(f"Unequipped -> Weapon Family: {unequip_kit}")
    unequip_kit_ok = (unequip_kit == unreal.WyrmWeaponFamily.UNARMED)

    kit_switch_pass = melee_kit_ok and ranged_kit_ok and unequip_kit_ok
    results["COM-08_KitSwitching"] = {
        "status": "PASS" if kit_switch_pass else "FAIL",
        "melee_kit_ok": melee_kit_ok,
        "ranged_kit_ok": ranged_kit_ok,
        "unequip_kit_ok": unequip_kit_ok
    }
    if not kit_switch_pass:
        passed = False

    # =============================================================
    # CASE 3: Melee vs Ranged Combat Encounters & Projectile Execution
    # =============================================================
    log("Running Case 3: Melee vs Ranged Engagement Range Tests...")
    
    # Spawn enemy at (800, 0, 100) — outside melee reach (220cm)
    enemy_xform = unreal.Transform(location=unreal.Vector(800.0, 0.0, 100.0), rotation=unreal.Rotator(0.0, 0.0, 0.0), scale=unreal.Vector(1.0, 1.0, 1.0))
    enemy = unreal.WyrmEnemyCharacter.spawn_wyrm_enemy(world, unreal.WyrmEnemyRole.MELEE_CHASER, enemy_xform)
    enemy_attrs = enemy.get_attributes()
    enemy_init_hp = enemy_attrs.get_current_health()

    # Step 3A: Equip Melee Sword and attempt attack at 800cm
    inv.equip_item(sword.instance_id, unreal.WyrmEquipSlot.MAIN_HAND)
    player.perform_primary_attack()
    dummy_hp_after_melee_miss = enemy_attrs.get_current_health()
    log(f"Melee attack at 800cm: Enemy HP={dummy_hp_after_melee_miss} (Initial={enemy_init_hp})")
    melee_range_gated = (dummy_hp_after_melee_miss == enemy_init_hp)

    # Step 3B: Equip Bow and fire ranged projectile at 800cm
    inv.equip_item(bow.instance_id, unreal.WyrmEquipSlot.MAIN_HAND)
    player.perform_primary_attack()

    all_projectiles = unreal.GameplayStatics.get_all_actors_of_class(world, unreal.WyrmProjectile)
    projectile_spawned = len(all_projectiles) > 0
    log(f"Ranged attack fired: Spawned Projectiles Count={len(all_projectiles)}")

    # Step 3C: Tactical mobility / Evade ability
    evade_success = player.perform_evade()
    log(f"Evade performed: Success={evade_success}")

    combat_pass = melee_range_gated and projectile_spawned and evade_success
    results["COM-08_CombatArchetypes"] = {
        "status": "PASS" if combat_pass else "FAIL",
        "melee_range_gated": melee_range_gated,
        "projectile_spawned": projectile_spawned,
        "evade_success": evade_success
    }
    if not combat_pass:
        passed = False

    # =============================================================
    # CASE 4: Dual-Camera Perspective Parity
    # =============================================================
    log("Running Case 4: Dual-Camera Perspective Parity...")
    player.set_camera_mode(unreal.WyrmCameraMode.TOP_DOWN)
    topdown_mode = player.get_camera_mode()
    player.perform_primary_attack() # Fires ranged in TopDown
    topdown_projectiles = unreal.GameplayStatics.get_all_actors_of_class(world, unreal.WyrmProjectile)

    player.set_camera_mode(unreal.WyrmCameraMode.THIRD_PERSON)
    tp_mode = player.get_camera_mode()

    camera_pass = (topdown_mode == unreal.WyrmCameraMode.TOP_DOWN and
                   tp_mode == unreal.WyrmCameraMode.THIRD_PERSON and
                   len(topdown_projectiles) >= 1)

    results["COM-08_CameraParity"] = {
        "status": "PASS" if camera_pass else "FAIL",
        "topdown_ok": topdown_mode == unreal.WyrmCameraMode.TOP_DOWN,
        "third_person_ok": tp_mode == unreal.WyrmCameraMode.THIRD_PERSON
    }
    if not camera_pass:
        passed = False

    # Cleanup actors
    for proj in all_projectiles:
        if proj:
            proj.destroy_actor()
    enemy.destroy_actor()
    player.destroy_actor()

    # Output results
    receipt_path = "G:/assets/voxel project/Saved/Diagnostics/WP06_progression_proof.json"
    os.makedirs(os.path.dirname(receipt_path), exist_ok=True)
    with open(receipt_path, "w") as f:
        json.dump(results, f, indent=2)

    log(f"Proof complete. Overall Passed: {passed}. Receipt written to {receipt_path}")
    sys.exit(0 if passed else 1)

if __name__ == "__main__":
    main()
