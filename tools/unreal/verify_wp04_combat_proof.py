"""
Editor-world commandlet verification for WP-04: First Real Combat Loop
Tests acceptance criteria COM-01 through COM-05:
  COM-01: Canonical damage execution via authoritative GAS pipeline.
  COM-02: Bounds and drain limits: Shield absorption, invulnerability immunity, dead target rejection, overkill clamp.
  COM-03: Two-camera combat: Seamless perspective toggle mid-encounter without state or attack reset.
  COM-04: Focus cost and cooldown commit: Atomic deduction, cooldown gating, double-activation rejection.
  COM-05: Status effect interactions (slow magnitude combining, boss resistance) and enemy defeat.

Run via:
  UnrealEditor-Cmd.exe WYRMFALL.uproject -run=pythonscript -script="tools/unreal/verify_wp04_combat_proof.py" -stdout -FullStdOutLogOutput -unattended -nopause -nosplash -nullrhi
"""

import unreal
import json
import os
import sys

def log(msg):
    unreal.log(f"[WP04_COMBAT_PROOF] {msg}")

def main():
    log("Starting WP-04 Combat Proof Suite...")
    results = {}
    passed = True

    world = unreal.EditorLevelLibrary.get_editor_world()
    if not world:
        log("ERROR: Could not get editor world.")
        sys.exit(1)

    # -------------------------------------------------------------
    # SETUP: Spawn Player Character & Controller
    # -------------------------------------------------------------
    player_loc = unreal.Vector(0.0, 0.0, 100.0)
    player_trans = unreal.Transform(location=player_loc)

    player = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.WyrmCharacter, player_loc)
    if not player:
        log("ERROR: Failed to spawn WyrmCharacter")
        sys.exit(1)

    pc = unreal.WyrmPlayerController.spawn_wyrm_player_controller(world, player_trans)
    if not pc:
        log("ERROR: Failed to spawn WyrmPlayerController")
        player.destroy_actor()
        sys.exit(1)

    pc.possess(player)
    player.grant_combat_abilities()
    player_attrs = player.get_attributes()

    # =============================================================
    # CASE COM-01: Canonical Damage Execution via Authoritative GAS
    # =============================================================
    # Spawn Melee Chaser (Wolf role) in front of player
    enemy1_loc = unreal.Vector(100.0, 0.0, 100.0) # 100 cm ahead, well within 150 cm reach
    enemy1_trans = unreal.Transform(location=enemy1_loc)
    enemy1 = unreal.WyrmEnemyCharacter.spawn_wyrm_enemy(world, unreal.WyrmEnemyRole.MELEE_CHASER, enemy1_trans)
    
    if not enemy1:
        log("ERROR: Failed to spawn WyrmEnemyCharacter")
        results["COM-01"] = {"status": "FAIL", "reason": "Failed to spawn enemy actor"}
        passed = False
    else:
        enemy1_attrs = enemy1.get_attributes()
        initial_hp = enemy1_attrs.get_current_health()
        enemy_armor = enemy1_attrs.get_current_armor() # 10.0
        player_power = player_attrs.get_current_power() # 20.0
        
        # Primary attack: WeaponBase 10.0 + 0.5 * 20.0 = 20.0 Raw
        # Mitigation: Armor 10 / (10 + 50 + 10*1) = 10 / 70 = 0.142857
        # Mitigated: 20.0 * (1 - 10/70) = 17.142857
        # Expected new HP = 60.0 - 17.142857 = ~42.857
        strike_success = player.perform_primary_attack()
        hp_after = enemy1_attrs.get_current_health()
        damage_dealt = initial_hp - hp_after

        log(f"COM-01: Initial HP={initial_hp}, Armor={enemy_armor}, Player Power={player_power}, HP After={hp_after}, Dealt={damage_dealt:.3f}")
        
        if strike_success and 16.5 <= damage_dealt <= 17.5:
            results["COM-01"] = {
                "status": "PASS",
                "initial_health": initial_hp,
                "health_after_strike": hp_after,
                "damage_dealt": damage_dealt,
                "expected_damage": 17.143,
                "mitigation_observed": f"{(1.0 - damage_dealt / 20.0) * 100:.1f}%"
            }
        else:
            results["COM-01"] = {
                "status": "FAIL",
                "strike_success": strike_success,
                "damage_dealt": damage_dealt,
                "initial_hp": initial_hp,
                "hp_after": hp_after
            }
            passed = False

    # =============================================================
    # CASE COM-02: Bounds & Drain Limits (Shield, Invulnerability, Dead)
    # =============================================================
    # Test shield absorption: grant 10.0 Shield to enemy1
    enemy1_attrs.set_current_shield(10.0)
    current_hp_before = enemy1_attrs.get_current_health() # ~42.857
    
    # Strike again: 17.14 damage. Shield (10.0) absorbed completely; remaining 7.14 damage to health
    player.perform_primary_attack()
    shield_after = enemy1_attrs.get_current_shield()
    hp_after_shield_hit = enemy1_attrs.get_current_health()
    shield_absorbed = 10.0 - shield_after
    health_lost = current_hp_before - hp_after_shield_hit

    log(f"COM-02 (Shield): Shield absorbed={shield_absorbed:.2f}, Remaining Shield={shield_after:.2f}, Health Lost={health_lost:.2f}")

    # Test invulnerability: apply State.Combat.Invulnerable
    enemy1.set_invulnerable(True)

    hp_pre_invuln_hit = enemy1_attrs.get_current_health()
    player.perform_primary_attack()
    hp_post_invuln_hit = enemy1_attrs.get_current_health()
    invuln_passed = (hp_pre_invuln_hit == hp_post_invuln_hit)
    log(f"COM-02 (Invulnerability): Pre-hit HP={hp_pre_invuln_hit}, Post-hit HP={hp_post_invuln_hit}, Unharmed={invuln_passed}")
    enemy1.set_invulnerable(False)

    # Test dead target boundary: deal lethal damage until Health <= 0
    max_strikes = 10
    strikes = 0
    while enemy1_attrs.get_current_health() > 0.0 and strikes < max_strikes:
        player.perform_primary_attack()
        strikes += 1

    dead_hp = enemy1_attrs.get_current_health()
    is_defeated = enemy1.is_defeated()
    has_dead_tag = enemy1.has_matching_gameplay_tag("State.Dead")

    # Overkill check: strike dead enemy again
    player.perform_primary_attack()
    post_dead_hp = enemy1_attrs.get_current_health()
    dead_boundary_passed = (dead_hp == 0.0 and post_dead_hp == 0.0 and is_defeated and has_dead_tag)
    log(f"COM-02 (Death/Overkill): Dead HP={dead_hp}, Post-hit Dead HP={post_dead_hp}, HasDeadTag={has_dead_tag}")

    if shield_after == 0.0 and invuln_passed and dead_boundary_passed:
        results["COM-02"] = {
            "status": "PASS",
            "shield_absorbed": shield_absorbed,
            "shield_remaining": shield_after,
            "invulnerable_damage_taken": 0.0,
            "dead_target_damage_rejected": True,
            "health_clamped_at_zero": True
        }
    else:
        results["COM-02"] = {
            "status": "FAIL",
            "shield_after": shield_after,
            "invuln_passed": invuln_passed,
            "dead_boundary_passed": dead_boundary_passed
        }
        passed = False

    # Cleanup enemy1
    enemy1.destroy_actor()

    # =============================================================
    # CASE COM-03: Two-Camera Combat Mid-Encounter Switching
    # =============================================================
    # Spawn fresh enemy in front of player
    enemy2 = unreal.WyrmEnemyCharacter.spawn_wyrm_enemy(world, unreal.WyrmEnemyRole.MELEE_CHASER, enemy1_trans)
    enemy2_attrs = enemy2.get_attributes()

    # Strike 1 in Third-Person mode
    player.set_camera_mode(unreal.WyrmCameraMode.THIRD_PERSON)
    initial_e2_hp = enemy2_attrs.get_current_health()
    player.perform_primary_attack()
    hp_tp = enemy2_attrs.get_current_health()
    dmg_tp = initial_e2_hp - hp_tp

    # Switch mid-encounter to Top-Down mode
    player.set_camera_mode(unreal.WyrmCameraMode.TOP_DOWN)
    top_down_verified = (player.get_camera_mode() == unreal.WyrmCameraMode.TOP_DOWN)
    
    # Strike 2 in Top-Down mode
    player.perform_primary_attack()
    hp_td = enemy2_attrs.get_current_health()
    dmg_td = hp_tp - hp_td

    # Switch back to Third-Person mode
    player.set_camera_mode(unreal.WyrmCameraMode.THIRD_PERSON)
    tp_restored = (player.get_camera_mode() == unreal.WyrmCameraMode.THIRD_PERSON)

    log(f"COM-03: TP Dmg={dmg_tp:.2f}, TD Dmg={dmg_td:.2f}, TopDown={top_down_verified}, TPRestored={tp_restored}")

    if abs(dmg_tp - dmg_td) < 0.1 and top_down_verified and tp_restored:
        results["COM-03"] = {
            "status": "PASS",
            "third_person_damage": dmg_tp,
            "top_down_damage": dmg_td,
            "formula_parity": True,
            "camera_toggle_seamless": True
        }
    else:
        results["COM-03"] = {
            "status": "FAIL",
            "dmg_tp": dmg_tp,
            "dmg_td": dmg_td,
            "top_down_verified": top_down_verified
        }
        passed = False

    enemy2.destroy_actor()

    # =============================================================
    # CASE COM-04: Ability Focus Cost and Cooldown Commit
    # =============================================================
    # Player starts with 100 Focus
    player_attrs.set_current_focus(100.0)
    initial_focus = player_attrs.get_current_focus()

    # Activate Secondary Attack (costs 20 Focus, 5s cooldown)
    sec_activated = player.perform_secondary_attack()
    focus_after_sec = player_attrs.get_current_focus()
    focus_deducted = initial_focus - focus_after_sec

    # Immediate repeat request should fail due to active cooldown
    sec_repeat = player.perform_secondary_attack()
    focus_after_repeat = player_attrs.get_current_focus()

    # Set Focus below cost and try activation
    player_attrs.set_current_focus(10.0)
    sec_low_focus = player.perform_secondary_attack()
    focus_after_low = player_attrs.get_current_focus()

    log(f"COM-04: SecActivated={sec_activated}, Deducted={focus_deducted:.1f}, RepeatBlocked={not sec_repeat}, LowFocusBlocked={not sec_low_focus}")

    if sec_activated and focus_deducted == 20.0 and (not sec_repeat) and (focus_after_repeat == 80.0) and (not sec_low_focus) and (focus_after_low == 10.0):
        results["COM-04"] = {
            "status": "PASS",
            "initial_focus": initial_focus,
            "focus_after_cost": focus_after_sec,
            "cost_deducted": focus_deducted,
            "repeat_activation_blocked_by_cooldown": not sec_repeat,
            "insufficient_focus_activation_blocked": not sec_low_focus
        }
    else:
        results["COM-04"] = {
            "status": "FAIL",
            "sec_activated": sec_activated,
            "focus_deducted": focus_deducted,
            "sec_repeat": sec_repeat,
            "sec_low_focus": sec_low_focus
        }
        passed = False

    # Restore Focus
    player_attrs.set_current_focus(100.0)

    # =============================================================
    # CASE COM-05: Status Effects & Contrasting Enemy Roles
    # =============================================================
    # 1. Contrasting roles: MeleeChaser vs RangedSkirmisher
    chaser = unreal.WyrmEnemyCharacter.spawn_wyrm_enemy(world, unreal.WyrmEnemyRole.MELEE_CHASER, enemy1_trans)
    skirmisher = unreal.WyrmEnemyCharacter.spawn_wyrm_enemy(world, unreal.WyrmEnemyRole.RANGED_SKIRMISHER, unreal.Transform(location=unreal.Vector(250.0, 0.0, 100.0)))

    chaser_hp = chaser.get_attributes().get_current_max_health()
    chaser_armor = chaser.get_attributes().get_current_armor()
    chaser_spd = chaser.get_current_speed()

    skirm_hp = skirmisher.get_attributes().get_current_max_health()
    skirm_armor = skirmisher.get_attributes().get_current_armor()
    skirm_spd = skirmisher.get_current_speed()

    roles_distinct = (chaser_hp == 60.0 and chaser_armor == 10.0 and chaser_spd == 550.0 and
                      skirm_hp == 50.0 and skirm_armor == 5.0 and skirm_spd == 400.0)

    # 2. Slow status effect: magnitude combining rule (highest magnitude, not multiplicative)
    chaser.apply_named_status_effect("State.Combat.Slow", 5.0, 0.3)
    speed_slow1 = chaser.get_current_speed() # 550 * 0.7 = 385.0

    chaser.apply_named_status_effect("State.Combat.Slow", 5.0, 0.5)
    speed_slow2 = chaser.get_current_speed() # 550 * 0.5 = 275.0 (combines by max 0.5, not 0.3*0.5=0.15)

    slow_combining_passed = (380.0 <= speed_slow1 <= 390.0 and 270.0 <= speed_slow2 <= 280.0)
    log(f"COM-05: Chaser Base={chaser_spd}, Slow1={speed_slow1}, Slow2={speed_slow2}, SlowCombinePass={slow_combining_passed}")

    # 3. Boss resistance: boss reduces slow and resists hard stun
    boss = unreal.WyrmEnemyCharacter.spawn_wyrm_enemy(world, unreal.WyrmEnemyRole.MELEE_CHASER, unreal.Transform(location=unreal.Vector(400.0, 0.0, 100.0)))
    boss.set_editor_property("bIsBoss", True)
    boss.apply_named_status_effect("State.Combat.Stun", 2.0, 1.0)
    boss_resisted_stun = (boss.get_current_speed() > 0.0)
    log(f"COM-05: Boss Resisted Stun={boss_resisted_stun} (Speed={boss.get_current_speed()})")

    # Cleanup actors
    chaser.destroy_actor()
    skirmisher.destroy_actor()
    boss.destroy_actor()

    if roles_distinct and slow_combining_passed and boss_resisted_stun:
        results["COM-05"] = {
            "status": "PASS",
            "roles_distinct": True,
            "melee_chaser_profile": {"max_hp": chaser_hp, "armor": chaser_armor, "speed": chaser_spd},
            "ranged_skirmisher_profile": {"max_hp": skirm_hp, "armor": skirm_armor, "speed": skirm_spd},
            "slow_magnitude_combining": True,
            "boss_hard_stun_resisted": True
        }
    else:
        results["COM-05"] = {
            "status": "FAIL",
            "roles_distinct": roles_distinct,
            "slow_combining_passed": slow_combining_passed,
            "boss_resisted_stun": boss_resisted_stun
        }
        passed = False

    # -------------------------------------------------------------
    # CLEANUP
    # -------------------------------------------------------------
    pc.un_possess()
    pc.destroy_actor()
    player.destroy_actor()

    # -------------------------------------------------------------
    # RECORD EVIDENCE
    # -------------------------------------------------------------
    evidence_dir = "G:/assets/voxel project/Saved/Diagnostics"
    os.makedirs(evidence_dir, exist_ok=True)
    evidence_path = os.path.join(evidence_dir, "WP04_combat_proof.json")

    final_report = {
        "suite": "WP-04 First Real Combat Loop Proof",
        "overall_status": "PASS" if passed else "FAIL",
        "cases": results
    }

    with open(evidence_path, "w", encoding="utf-8") as f:
        json.dump(final_report, f, indent=2)

    log(f"Evidence written to {evidence_path}")
    log(f"Overall status: {final_report['overall_status']}")

    if not passed:
        log("ERROR: One or more combat proof cases failed.")
        sys.exit(1)

    log("ALL COMBAT PROOF CASES SUCCEEDED.")
    sys.exit(0)

if __name__ == "__main__":
    main()
