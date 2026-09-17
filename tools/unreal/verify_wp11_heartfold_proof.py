"""WP-11 Green Dragon Heartfold and Compact Behavior PIE Proof."""
import json
from pathlib import Path
import time
import traceback
import unreal


ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
REPORT_PATH = ROOT / "Saved/Diagnostics/WP11_heartfold_proof.json"
report = {
    "kind": "wp11_dragon_heartfold_compact_behavior_pie",
    "engine": unreal.SystemLibrary.get_engine_version(),
    "status": "INITIALIZING",
    "tests": {
        "DRG-08.CompactSizeAndDoorway": {"status": "NOT_RUN"},
        "DRG-09.CompactCombatAndDirectControl": {"status": "NOT_RUN"},
        "DRG-10.FormStateConservation": {"status": "NOT_RUN"},
        "DRG-11.BlockedGrowth": {"status": "NOT_RUN"},
        "DRG-12.InterruptedTransition": {"status": "NOT_RUN"},
        "DRG-13.TownHomecomingBehavior": {"status": "NOT_RUN"},
        "SAVE-08.CompactSaveRoundtrip": {"status": "NOT_RUN"},
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
    if isinstance(res, (tuple, list)):
        return bool(res[0])
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

# Pre-spawn low ceiling obstacle for DRG-11 at X=1000, Y=1000, Z=200 before PIE begins
ceiling_actor = editor_actor_subsystem.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(1000.0, 1000.0, 200.0))
if ceiling_actor and cube_mesh:
    c_mesh_comp = ceiling_actor.get_component_by_class(unreal.StaticMeshComponent)
    if c_mesh_comp:
        c_mesh_comp.set_static_mesh(cube_mesh)
        c_mesh_comp.set_collision_profile_name("BlockAll")
    ceiling_actor.set_actor_scale3d(unreal.Vector(6.0, 6.0, 0.4))

level_editor = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
started_at = time.monotonic()
stage = "init"
finished = False

# References across stages
player_ref = None
dragon_ref = None
wyrm_pc_ref = None
dummy_ref = None
parity_state = {}


def pie_tick(_delta_seconds):
    global stage, finished, player_ref, dragon_ref, wyrm_pc_ref, dummy_ref, parity_state
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

            # Spawn allied dragon
            dragon_spawn_tf = unreal.Transform(
                location=unreal.Vector(200.0, 0.0, 100.0),
                rotation=unreal.Rotator(0.0, 0.0, 0.0),
                scale=unreal.Vector(1.0, 1.0, 1.0)
            )
            dragon = unreal.WyrmDragonCharacter.spawn_wyrm_dragon(
                world, unreal.WyrmDragonRole.ALLIED_COMPANION, dragon_spawn_tf)
            if not dragon:
                raise RuntimeError("Failed to spawn WyrmDragonCharacter in PIE")

            dragon.bond_with_humanoid(player)
            dragon.set_dragon_form(unreal.WyrmDragonForm.COMPANION_FORM)

            player_ref = player
            dragon_ref = dragon
            wyrm_pc_ref = wyrm_pc

            log("WP-11 PIE fixture initialized with Player and Allied Companion Dragon.")
            stage = "test_drg08_doorfit"
            return

        # -------------------------------------------------------------
        # Stage: DRG-08.CompactSizeAndDoorway
        # -------------------------------------------------------------
        if stage == "test_drg08_doorfit":
            capsule = dragon_ref.capsule_component if hasattr(dragon_ref, "capsule_component") else dragon_ref.get_component_by_class(unreal.CapsuleComponent)
            comp_radius = capsule.capsule_radius if hasattr(capsule, "capsule_radius") else capsule.get_unscaled_capsule_radius()
            comp_height = capsule.capsule_half_height if hasattr(capsule, "capsule_half_height") else capsule.get_unscaled_capsule_half_height()
            comp_scale = getattr(dragon_ref.mesh, "relative_scale3d", unreal.Vector(0.009, 0.009, 0.009)).x if dragon_ref.mesh else 0.009

            # Standard doorframe opening: 100cm width x 210cm height (SM_Stylized_Wood_Doorframe)
            door_width = 100.0
            door_height = 210.0
            comp_diam = comp_radius * 2.0
            comp_full_height = comp_height * 2.0

            comp_fits = (comp_diam < door_width) and (comp_full_height < door_height)
            if not comp_fits:
                raise RuntimeError(f"Companion dimensions (diam={comp_diam}, height={comp_full_height}) do not fit door ({door_width}x{door_height})")

            # Switch to True Form
            dragon_ref.set_dragon_form(unreal.WyrmDragonForm.TRUE_FORM)
            tf_radius = capsule.capsule_radius if hasattr(capsule, "capsule_radius") else capsule.get_unscaled_capsule_radius()
            tf_height = capsule.capsule_half_height if hasattr(capsule, "capsule_half_height") else capsule.get_unscaled_capsule_half_height()
            tf_diam = tf_radius * 2.0
            tf_full_height = tf_height * 2.0
            tf_scale = getattr(dragon_ref.mesh, "relative_scale3d", unreal.Vector(0.035, 0.035, 0.035)).x if dragon_ref.mesh else 0.035

            tf_blocked = (tf_diam > door_width) and (tf_full_height > door_height)
            if not tf_blocked:
                raise RuntimeError(f"True Form dimensions (diam={tf_diam}, height={tf_full_height}) not blocked by door ({door_width}x{door_height})")

            # Check locomotion speeds
            comp_speed = dragon_ref.get_companion_ground_speed()
            comp_catchup = dragon_ref.get_companion_catch_up_speed()
            tf_speed = dragon_ref.get_true_form_ground_speed()

            finish_test("DRG-08.CompactSizeAndDoorway", True, {
                "companion_radius": comp_radius,
                "companion_half_height": comp_height,
                "companion_mesh_scale": comp_scale,
                "companion_diameter_vs_door": f"{comp_diam}cm < {door_width}cm (FITS)",
                "companion_height_vs_door": f"{comp_full_height}cm < {door_height}cm (FITS)",
                "trueform_radius": tf_radius,
                "trueform_half_height": tf_height,
                "trueform_mesh_scale": tf_scale,
                "trueform_diameter_vs_door": f"{tf_diam}cm > {door_width}cm (BLOCKED)",
                "trueform_height_vs_door": f"{tf_full_height}cm > {door_height}cm (BLOCKED)",
                "companion_ground_speed": comp_speed,
                "companion_catchup_speed": comp_catchup,
                "trueform_ground_speed": tf_speed,
            })

            stage = "test_drg09_combat"
            return

        # -------------------------------------------------------------
        # Stage: DRG-09.CompactCombatAndDirectControl
        # -------------------------------------------------------------
        if stage == "test_drg09_combat":
            dragon_ref.set_dragon_form(unreal.WyrmDragonForm.COMPANION_FORM)

            # Direct control test
            transferred = wyrm_pc_ref.transfer_control_to_dragon(dragon_ref)
            if not transferred or not dragon_ref.is_directly_controlled():
                raise RuntimeError("Failed to transfer direct control to compact dragon")

            # Spawn enemy dummy target at X=420 (distance 220cm: clear of TrueForm 120cm capsule, within 250cm area attack radius)
            dummy_spawn_tf = unreal.Transform(
                location=unreal.Vector(420.0, 0.0, 100.0),
                rotation=unreal.Rotator(0.0, 0.0, 0.0),
                scale=unreal.Vector(1.0, 1.0, 1.0)
            )
            dummy = unreal.WyrmEnemyCharacter.spawn_wyrm_enemy(
                world, unreal.WyrmEnemyRole.MELEE_CHASER, dummy_spawn_tf)
            if not dummy:
                raise RuntimeError("Failed to spawn enemy dummy in PIE")

            dummy.attributes.set_current_max_health(100.0)
            dummy.attributes.set_current_health(100.0)
            dummy.attributes.set_current_armor(0.0)
            dummy_ref = dummy

            # Perform Primary Attack in CompanionForm: 9 damage
            pri_hit = dragon_ref.perform_primary_attack(dummy)
            pri_hp = dummy.attributes.get_current_health()
            if not pri_hit or abs(pri_hp - 91.0) > 0.1:
                raise RuntimeError(f"Companion primary attack failed: hit={pri_hit}, HP={pri_hp} (expected 91.0)")

            # Reset dummy HP and perform Secondary Area Attack: 6 damage, 6s cooldown
            dummy.attributes.set_current_health(100.0)
            sec_hit = dragon_ref.perform_secondary_attack(dummy)
            sec_hp = dummy.attributes.get_current_health()
            cooldown_rem = dragon_ref.get_area_attack_cooldown_remaining()
            if not sec_hit or abs(sec_hp - 94.0) > 0.1 or cooldown_rem <= 0.0:
                raise RuntimeError(f"Companion secondary attack failed: hit={sec_hit}, HP={sec_hp}, cooldown={cooldown_rem}")

            # Return control to humanoid
            returned = wyrm_pc_ref.return_control_to_humanoid()
            if not returned or dragon_ref.is_directly_controlled():
                raise RuntimeError("Failed to return direct control to humanoid")

            finish_test("DRG-09.CompactCombatAndDirectControl", True, {
                "direct_control_transfer": True,
                "companion_primary_damage": 9.0,
                "dummy_hp_after_primary": pri_hp,
                "companion_secondary_damage": 6.0,
                "dummy_hp_after_secondary": sec_hp,
                "area_attack_cooldown_committed": cooldown_rem,
                "direct_control_returned": True,
            })

            stage = "test_drg10_parity"
            return

        # -------------------------------------------------------------
        # Stage: DRG-10.FormStateConservation
        # -------------------------------------------------------------
        if stage == "test_drg10_parity":
            # Set dragon to TrueForm with injury and depleted focus
            dragon_ref.set_dragon_form(unreal.WyrmDragonForm.TRUE_FORM)
            dragon_ref.attributes.set_current_health(175.0)
            dragon_ref.attributes.set_current_focus(80.0)

            orig_hp = dragon_ref.attributes.get_current_health()
            orig_max_hp = dragon_ref.attributes.get_current_max_health()
            orig_focus = dragon_ref.attributes.get_current_focus()
            orig_cd = dragon_ref.get_area_attack_cooldown_remaining()

            # Exercise the real timed form-transition path. The follow-up stages
            # wait for its normal one-second presentation and shared cooldown.
            folded = eval_ue_result(dragon_ref.request_form_change(unreal.WyrmDragonForm.COMPANION_FORM))
            if not folded or not dragon_ref.is_transitioning_form():
                raise RuntimeError("Timed compact-form transition did not start")
            parity_state = {
                "initial_injured_health": orig_hp,
                "initial_max_health": orig_max_hp,
                "initial_focus": orig_focus,
                "initial_area_cooldown": orig_cd,
            }
            stage = "await_drg10_fold"
            return

        if stage == "await_drg10_fold":
            if dragon_ref.is_transitioning_form():
                return

            folded_hp = dragon_ref.attributes.get_current_health()
            folded_max_hp = dragon_ref.attributes.get_current_max_health()
            folded_focus = dragon_ref.attributes.get_current_focus()
            if (dragon_ref.get_dragon_form() != unreal.WyrmDragonForm.COMPANION_FORM
                    or folded_hp != parity_state["initial_injured_health"]
                    or folded_max_hp != parity_state["initial_max_health"]
                    or folded_focus != parity_state["initial_focus"]
                    or dragon_ref.get_area_attack_cooldown_remaining() <= 0.0):
                raise RuntimeError(
                    f"Parity violated on timed fold: form={dragon_ref.get_dragon_form()}, "
                    f"HP={folded_hp}, Focus={folded_focus}")
            parity_state.update({
                "folded_health_conserved": folded_hp,
                "folded_max_health_conserved": folded_max_hp,
                "folded_focus_conserved": folded_focus,
            })
            stage = "await_drg10_cooldown"
            return

        if stage == "await_drg10_cooldown":
            # Match the runtime gate exactly.  A small positive remainder is
            # still a cooldown, so do not issue the reverse request until it
            # has reached zero.
            if dragon_ref.get_form_transition_cooldown_remaining() > 0.0:
                return
            can_unfold = dragon_ref.can_change_form(unreal.WyrmDragonForm.TRUE_FORM)
            if not eval_ue_result(can_unfold):
                reason = str(can_unfold[1]) if isinstance(can_unfold, (tuple, list)) and len(can_unfold) > 1 else "unknown reason"
                raise RuntimeError(f"Timed true-form transition remained invalid after cooldown: {reason}")
            unfolded = eval_ue_result(dragon_ref.request_form_change(unreal.WyrmDragonForm.TRUE_FORM))
            if not unfolded or not dragon_ref.is_transitioning_form():
                raise RuntimeError("Timed true-form transition did not start after cooldown")
            stage = "await_drg10_unfold"
            return

        if stage == "await_drg10_unfold":
            if dragon_ref.is_transitioning_form():
                return
            unfolded_hp = dragon_ref.attributes.get_current_health()
            unfolded_focus = dragon_ref.attributes.get_current_focus()
            if (dragon_ref.get_dragon_form() != unreal.WyrmDragonForm.TRUE_FORM
                    or unfolded_hp != parity_state["initial_injured_health"]
                    or unfolded_focus != parity_state["initial_focus"]):
                raise RuntimeError(
                    f"Parity violated on timed unfold: form={dragon_ref.get_dragon_form()}, "
                    f"HP={unfolded_hp}, Focus={unfolded_focus}")

            finish_test("DRG-10.FormStateConservation", True, {
                "initial_injured_health": parity_state["initial_injured_health"],
                "initial_focus": parity_state["initial_focus"],
                "folded_health_conserved": parity_state["folded_health_conserved"],
                "folded_max_health_conserved": parity_state["folded_max_health_conserved"],
                "folded_focus_conserved": parity_state["folded_focus_conserved"],
                "unfolded_health_conserved": unfolded_hp,
                "unfolded_focus_conserved": unfolded_focus,
                "zero_free_healing_verified": True,
                "timed_transitions_used": True,
            })

            # The reverse timed transition has its own normal recovery
            # cooldown.  Let it expire before putting the fixture into the
            # compact form used by the separate clearance test.
            stage = "await_drg11_cooldown"
            return

        if stage == "await_drg11_cooldown":
            if dragon_ref.get_form_transition_cooldown_remaining() > 0.0:
                return
            stage = "test_drg11_blocked_growth"
            return

        # -------------------------------------------------------------
        # Stage: DRG-11.BlockedGrowth
        # -------------------------------------------------------------
        if stage == "test_drg11_blocked_growth":
            dragon_ref.set_dragon_form(unreal.WyrmDragonForm.COMPANION_FORM)

            # In open space (X=200, Y=0, Z=100), growth is permitted
            dragon_ref.set_actor_location(unreal.Vector(200.0, 0.0, 100.0), False, False)
            can_grow_open = eval_ue_result(dragon_ref.can_change_form(unreal.WyrmDragonForm.TRUE_FORM))
            if not can_grow_open:
                raise RuntimeError("CanChangeForm to TrueForm rejected in open space")

            # Move directly under pre-spawned low ceiling fixture at X=1000, Y=1000, Z=50 (ceiling at Z=200, TrueForm needs 320cm)
            dragon_ref.set_actor_location(unreal.Vector(1000.0, 1000.0, 50.0), False, False)

            # Blocked growth check
            grow_res = dragon_ref.can_change_form(unreal.WyrmDragonForm.TRUE_FORM)
            can_grow_blocked = eval_ue_result(grow_res)
            reason = str(grow_res[1]) if isinstance(grow_res, (tuple, list)) and len(grow_res) > 1 else ""

            if can_grow_blocked:
                raise RuntimeError("CanChangeForm succeeded under low ceiling obstacle (should be blocked)")

            req_blocked = eval_ue_result(dragon_ref.request_form_change(unreal.WyrmDragonForm.TRUE_FORM))
            if req_blocked:
                raise RuntimeError("RequestFormChange succeeded under low ceiling obstacle")

            remains_compact = (dragon_ref.get_dragon_form() == unreal.WyrmDragonForm.COMPANION_FORM)

            # Move back to open space
            dragon_ref.set_actor_location(unreal.Vector(200.0, 0.0, 100.0), False, False)

            can_grow_cleared = eval_ue_result(dragon_ref.can_change_form(unreal.WyrmDragonForm.TRUE_FORM))
            if not can_grow_cleared:
                raise RuntimeError("CanChangeForm failed after returning to open space")

            finish_test("DRG-11.BlockedGrowth", True, {
                "open_space_can_grow": can_grow_open,
                "ceiling_height": 200.0,
                "blocked_growth_rejected": not can_grow_blocked,
                "rejection_reason": reason,
                "dragon_remained_compact": remains_compact,
                "post_clearance_growth_allowed": can_grow_cleared,
            })

            stage = "test_drg12_interrupted_transition"
            return

        # -------------------------------------------------------------
        # Stage: DRG-12.InterruptedTransition
        # -------------------------------------------------------------
        if stage == "test_drg12_interrupted_transition":
            dragon_ref.set_dragon_form(unreal.WyrmDragonForm.COMPANION_FORM)
            dragon_ref.attributes.set_current_max_health(420.0)
            dragon_ref.attributes.set_current_health(420.0)

            # Start transition (non-immediate)
            started = eval_ue_result(dragon_ref.request_form_change(unreal.WyrmDragonForm.TRUE_FORM))
            if not started or not dragon_ref.is_transitioning_form():
                raise RuntimeError("Failed to begin form transition")

            # Apply incoming damage to interrupt
            instigator = player_ref.get_controller() if player_ref else None
            unreal.GameplayStatics.apply_damage(dragon_ref, 30.0, instigator, player_ref, unreal.DamageType)

            is_trans = dragon_ref.is_transitioning_form()
            curr_form = dragon_ref.get_dragon_form()
            cd_rem = dragon_ref.get_form_transition_cooldown_remaining()
            curr_hp = dragon_ref.attributes.get_current_health()

            if is_trans or curr_form != unreal.WyrmDragonForm.COMPANION_FORM or cd_rem <= 0.0 or curr_hp != 390.0:
                raise RuntimeError(
                    f"Transition interruption failed: is_trans={is_trans}, form={curr_form}, cd={cd_rem}, HP={curr_hp}")

            finish_test("DRG-12.InterruptedTransition", True, {
                "transition_started": True,
                "interrupted_by_damage": not is_trans,
                "form_rolled_back_to_compact": (curr_form == unreal.WyrmDragonForm.COMPANION_FORM),
                "recovery_cooldown_committed": cd_rem,
                "damage_conserved_without_duplication": curr_hp,
            })

            stage = "test_drg13_town_mode"
            return

        # -------------------------------------------------------------
        # Stage: DRG-13.TownHomecomingBehavior
        # -------------------------------------------------------------
        if stage == "test_drg13_town_mode":
            dragon_ref.set_town_mode_enabled(True)
            is_town = dragon_ref.is_town_mode_enabled()
            if not is_town:
                raise RuntimeError("Failed to enable town mode on dragon")

            # Issue attack order against dummy (immediately suppressed to FOLLOW by town mode)
            dragon_ref.issue_order(unreal.WyrmCompanionOrder.ATTACK_TARGET, dummy_ref)

            order = dragon_ref.get_companion_order()
            target = dragon_ref.get_combat_target()

            if order != unreal.WyrmCompanionOrder.FOLLOW or target is not None:
                raise RuntimeError(f"Town mode suppression failed: order={order}, target={target}")

            finish_test("DRG-13.TownHomecomingBehavior", True, {
                "town_mode_enabled": is_town,
                "combat_order_suppressed_to_follow": (order == unreal.WyrmCompanionOrder.FOLLOW),
                "combat_target_cleared": (target is None),
            })

            stage = "test_save08_compact_save"
            return

        # -------------------------------------------------------------
        # Stage: SAVE-08.CompactSaveRoundtrip
        # -------------------------------------------------------------
        if stage == "test_save08_compact_save":
            # Dragon is currently in CompanionForm, damaged (390.0 HP), town mode enabled
            slot_name = "WyrmSlot_Heartfold_Proof"
            snapshot = unreal.WyrmSaveSubsystem.create_snapshot_object(slot_name, player_ref, None, world)
            if not snapshot:
                raise RuntimeError("Failed to create snapshot save object")

            saved_form = snapshot.dragon_record.form
            saved_hp = snapshot.dragon_record.health
            saved_town = getattr(snapshot.dragon_record, 'town_mode_enabled', getattr(snapshot.dragon_record, 'b_town_mode_enabled', False))

            if saved_form != unreal.WyrmDragonForm.COMPANION_FORM or saved_hp != 390.0 or not saved_town:
                raise RuntimeError(f"Snapshot record corrupted: form={saved_form}, HP={saved_hp}, town={saved_town}")

            # Mutate live dragon state
            dragon_ref.set_dragon_form(unreal.WyrmDragonForm.TRUE_FORM)
            dragon_ref.attributes.set_current_health(420.0)
            dragon_ref.set_town_mode_enabled(False)

            # Apply snapshot restore
            restored = unreal.WyrmSaveSubsystem.apply_snapshot_object(snapshot, player_ref, None, world)
            if not restored:
                raise RuntimeError("Failed to apply snapshot restore")

            restored_form = dragon_ref.get_dragon_form()
            restored_hp = dragon_ref.attributes.get_current_health()
            restored_town = dragon_ref.is_town_mode_enabled()
            capsule = dragon_ref.capsule_component if hasattr(dragon_ref, "capsule_component") else dragon_ref.get_component_by_class(unreal.CapsuleComponent)
            restored_radius = capsule.capsule_radius if hasattr(capsule, "capsule_radius") else capsule.get_unscaled_capsule_radius()
            restored_height = capsule.capsule_half_height if hasattr(capsule, "capsule_half_height") else capsule.get_unscaled_capsule_half_height()

            if restored_form != unreal.WyrmDragonForm.COMPANION_FORM or restored_hp != 390.0 or not restored_town:
                raise RuntimeError(f"Restored dragon state mismatch: form={restored_form}, HP={restored_hp}, town={restored_town}")

            finish_test("SAVE-08.CompactSaveRoundtrip", True, {
                "saved_form": str(saved_form),
                "saved_health": saved_hp,
                "saved_town_mode": saved_town,
                "restored_form": str(restored_form),
                "restored_health": restored_hp,
                "restored_town_mode": restored_town,
                "restored_capsule_radius": restored_radius,
                "restored_capsule_half_height": restored_height,
            })

            # All tests passed!
            report["status"] = "PASS"
            log("ALL WP-11 PIE PROOF TESTS PASSED SUCCESSFULLY!")
            write_report()

            # Clean up
            if dummy_ref:
                dummy_ref.destroy_actor()
            if dragon_ref:
                dragon_ref.destroy_actor()
            finished = True
            level_editor.editor_request_end_play()
            return

    except Exception as exc:
        report["status"] = "FAIL"
        report["error"] = str(exc)
        report["traceback"] = traceback.format_exc()
        log(f"PIE exception in stage {stage}: {exc}")
        write_report()
        finished = True
        level_editor.editor_request_end_play()


tick_handle = unreal.register_slate_post_tick_callback(pie_tick)
level_editor.editor_request_begin_play()
log("PIE session started for WP-11 Heartfold verification.")
