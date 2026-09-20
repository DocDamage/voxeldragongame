"""Focused live-PIE proof for the WP-23.6 Cogfang rig/profile gate."""

import json
from pathlib import Path
import time
import traceback
import unreal


ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
REPORT_PATH = ROOT / "Saved/Diagnostics/WP23_6_cogfang_profile_proof.json"
ASSET_BASE = "/Game/WYRMFALL/Development/Intake/WP23_6/Cogfang/Steampunk_Dragon/SkeletalMeshes"
MAP_PACKAGE = "/Game/WYRMFALL/Development/Maps/L_DEV_Bootstrap"

report = {
    "kind": "wp23_6_cogfang_profile_proof",
    "engine": unreal.SystemLibrary.get_engine_version(),
    "status": "INITIALIZING",
    "tests": {
        "COG-01.AssetsProfileAndBond": {"status": "NOT_RUN"},
        "COG-02.HeartfoldAndClearance": {"status": "NOT_RUN"},
        "COG-03.MountAndFlight": {"status": "NOT_RUN"},
        "COG-04.GASCombatAndDirectControl": {"status": "NOT_RUN"},
        "COG-05.SaveIdentity": {"status": "NOT_RUN"},
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


def finish(name, details):
    report["tests"][name]["status"] = "PASS"
    report["details"][name] = details
    log(f"Test {name}: PASS")


def bool_result(value):
    if value is None:
        return False
    if isinstance(value, bool):
        return value
    if isinstance(value, (tuple, list)):
        return bool(value[0])
    return True


def reason_result(value):
    if isinstance(value, (tuple, list)):
        return next((item for item in value if isinstance(item, str)), "")
    return value if isinstance(value, str) else str(value or "")


def capsule_size(dragon):
    capsule = dragon.get_component_by_class(unreal.CapsuleComponent)
    radius = getattr(capsule, "capsule_radius", capsule.get_unscaled_capsule_radius())
    half_height = getattr(capsule, "capsule_half_height", capsule.get_unscaled_capsule_half_height())
    return float(radius), float(half_height)


write_report()
level_editor = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
editor_world_sub = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
current_world = editor_world_sub.get_editor_world()
if MAP_PACKAGE not in (current_world.get_path_name() if current_world else ""):
    level_editor.load_level(MAP_PACKAGE)

actor_sub = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
cube = unreal.load_asset("/Engine/BasicShapes/Cube.Cube")
ceiling = actor_sub.spawn_actor_from_class(
    unreal.StaticMeshActor, unreal.Vector(1200.0, 1200.0, 210.0))
if ceiling and cube:
    comp = ceiling.get_component_by_class(unreal.StaticMeshComponent)
    comp.set_static_mesh(cube)
    comp.set_collision_profile_name("BlockAll")
    ceiling.set_actor_scale3d(unreal.Vector(6.0, 6.0, 0.4))

stage = "init"
stage_started = time.monotonic()
finished = False
player = None
controller = None
dragon = None


def advance(next_stage):
    global stage, stage_started
    stage = next_stage
    stage_started = time.monotonic()


def pie_tick(_delta):
    global finished, player, controller, dragon
    try:
        if finished:
            if not level_editor.is_in_play_in_editor():
                unreal.unregister_slate_post_tick_callback(tick_handle)
                unreal.SystemLibrary.quit_editor()
            return

        if time.monotonic() - stage_started > 45:
            raise RuntimeError(f"Timed out in PIE stage {stage}")

        world = editor_world_sub.get_game_world()
        if not world or not level_editor.is_in_play_in_editor():
            return

        if stage == "init":
            raw_pc = unreal.GameplayStatics.get_player_controller(world, 0)
            if not raw_pc:
                return
            controller = unreal.WyrmPlayerController.cast(raw_pc)
            if not controller:
                return
            player = unreal.WyrmCharacter.cast(controller.get_controlled_pawn())
            if not player:
                chars = unreal.GameplayStatics.get_all_actors_of_class(world, unreal.WyrmCharacter)
                if not chars:
                    return
                player = chars[0]
                controller.possess(player)
            player.set_actor_location(unreal.Vector(0.0, 0.0, 120.0), False, False)
            advance("assets_profile")
            return

        if stage == "assets_profile":
            required = [
                f"{ASSET_BASE}/Hip-Local.Hip-Local",
                f"{ASSET_BASE}/Hip-Local_Skeleton.Hip-Local_Skeleton",
                f"{ASSET_BASE}/Chest-Local.Chest-Local",
                f"{ASSET_BASE}/Head-Local.Head-Local",
                f"{ASSET_BASE}/Tail_4-Local.Tail_4-Local",
                f"{ASSET_BASE}/Steampunk_DragonIdle_01.Steampunk_DragonIdle_01",
                f"{ASSET_BASE}/Steampunk_DragonFlying_01.Steampunk_DragonFlying_01",
                f"{ASSET_BASE}/Steampunk_DragonAttack_01.Steampunk_DragonAttack_01",
            ]
            missing = [path for path in required if not unreal.load_asset(path)]
            if missing:
                raise RuntimeError(f"Missing Cogfang assets: {missing}")

            transform = unreal.Transform(
                location=unreal.Vector(250.0, 0.0, 120.0),
                rotation=unreal.Rotator(0.0, 0.0, 0.0),
                scale=unreal.Vector(1.0, 1.0, 1.0))
            dragon = unreal.WyrmDragonCharacter.spawn_wyrm_dragon(
                world, unreal.WyrmDragonRole.HOSTILE_BOSS, transform)
            if not dragon:
                raise RuntimeError("Could not spawn Cogfang fixture")
            dragon.set_dragon_id("Cogfang")
            if not dragon.has_supported_rig_profile():
                raise RuntimeError("Cogfang profile is not supported")
            if len(dragon.follower_mesh_components) != 34:
                raise RuntimeError(f"Expected 34 followers, got {len(dragon.follower_mesh_components)}")
            dragon.perform_boss_defeat()
            if dragon.get_dragon_role() != unreal.WyrmDragonRole.DEFEATED_ALIVE:
                raise RuntimeError("Cogfang did not enter living-defeat state")
            if not dragon.bond_with_humanoid(player):
                raise RuntimeError("Cogfang bond conversion failed")
            if dragon.get_dragon_role() != unreal.WyrmDragonRole.ALLIED_COMPANION:
                raise RuntimeError("Cogfang did not become an allied companion")
            companion = capsule_size(dragon)
            if any(abs(value - expected) > 1.0 for value, expected in zip(companion, (32.0, 38.0))):
                raise RuntimeError(f"Unexpected Companion envelope: {companion}")
            finish("COG-01.AssetsProfileAndBond", {
                "dragon_id": str(dragon.dragon_id),
                "follower_components": len(dragon.follower_mesh_components),
                "companion_envelope": companion,
                "living_defeat_and_bond": True,
            })
            advance("heartfold_grow")
            return

        if stage == "heartfold_grow":
            if not dragon.request_form_change(unreal.WyrmDragonForm.TRUE_FORM):
                raise RuntimeError("Open-space TrueForm request failed")
            advance("heartfold_wait_grow")
            return

        if stage == "heartfold_wait_grow":
            if dragon.is_transitioning_form() or dragon.get_form_transition_cooldown_remaining() > 0.0:
                return
            if dragon.get_dragon_form() != unreal.WyrmDragonForm.TRUE_FORM:
                raise RuntimeError("Cogfang did not reach TrueForm")
            true_form = capsule_size(dragon)
            if any(abs(value - expected) > 1.0 for value, expected in zip(true_form, (125.0, 165.0))):
                raise RuntimeError(f"Unexpected TrueForm envelope: {true_form}")
            if not dragon.request_form_change(unreal.WyrmDragonForm.COMPANION_FORM):
                raise RuntimeError("Companion shrink request failed")
            report["details"]["true_form_envelope"] = true_form
            advance("heartfold_wait_shrink")
            return

        if stage == "heartfold_wait_shrink":
            if dragon.is_transitioning_form() or dragon.get_form_transition_cooldown_remaining() > 0.0:
                return
            dragon.set_actor_location(unreal.Vector(1200.0, 1200.0, 60.0), False, False)
            advance("heartfold_blocked")
            return

        if stage == "heartfold_blocked":
            result = dragon.can_change_form(unreal.WyrmDragonForm.TRUE_FORM)
            if bool_result(result):
                raise RuntimeError("Low ceiling did not block Cogfang growth")
            reason = reason_result(result)
            dragon.set_actor_location(unreal.Vector(250.0, 0.0, 120.0), False, False)
            finish("COG-02.HeartfoldAndClearance", {
                "growth": True,
                "shrink": True,
                "blocked_growth": True,
                "blocked_reason": reason,
                "true_form_envelope": report["details"].pop("true_form_envelope"),
            })
            advance("mount")
            return

        if stage == "mount":
            dragon.set_dragon_form(unreal.WyrmDragonForm.TRUE_FORM)
            if not bool_result(dragon.can_mount(player)) or not dragon.mount_humanoid(player):
                raise RuntimeError("Cogfang mount failed")
            if not bool_result(dragon.can_take_off()) or not dragon.take_off():
                raise RuntimeError("Cogfang takeoff failed")
            if not dragon.is_in_flight():
                raise RuntimeError("Cogfang did not enter flight")
            advance("land")
            return

        if stage == "land":
            if not dragon.land() or dragon.is_in_flight():
                raise RuntimeError("Cogfang landing failed")
            if not bool_result(dragon.dismount_humanoid()):
                raise RuntimeError("Cogfang dismount failed")
            finish("COG-03.MountAndFlight", {
                "mount": True, "takeoff": True, "landing": True, "dismount": True,
                "mount_offset": [0.0, 0.0, 170.0], "flight_speed": 1500.0,
            })
            advance("control_combat")
            return

        if stage == "control_combat":
            if not dragon.start_direct_control(controller, player):
                raise RuntimeError("Cogfang direct control failed")
            dummy_transform = unreal.Transform(
                location=unreal.Vector(400.0, 0.0, 120.0),
                rotation=unreal.Rotator(0.0, 0.0, 0.0),
                scale=unreal.Vector(1.0, 1.0, 1.0))
            dummy = unreal.WyrmEnemyCharacter.spawn_wyrm_enemy(
                world, unreal.WyrmEnemyRole.MELEE_CHASER, dummy_transform)
            if not dummy:
                raise RuntimeError("Combat dummy spawn failed")
            dummy.attributes.set_current_max_health(100.0)
            dummy.attributes.set_current_health(100.0)
            dummy.attributes.set_current_armor(0.0)
            primary = dragon.perform_primary_attack(dummy)
            primary_damage = 100.0 - dummy.attributes.get_current_health()
            dummy.attributes.set_current_health(100.0)
            secondary = dragon.perform_secondary_attack(dummy)
            secondary_damage = 100.0 - dummy.attributes.get_current_health()
            dummy.destroy_actor()
            if not primary or not secondary or primary_damage <= 0.0 or secondary_damage <= 0.0:
                raise RuntimeError(
                    f"GAS attacks failed: primary={primary}/{primary_damage}, secondary={secondary}/{secondary_damage}")
            if not dragon.end_direct_control(controller) or controller.get_controlled_pawn() != player:
                raise RuntimeError("Humanoid possession was not restored")
            finish("COG-04.GASCombatAndDirectControl", {
                "primary_damage": primary_damage,
                "secondary_damage": secondary_damage,
                "humanoid_repossessed": True,
            })
            advance("save")
            return

        if stage == "save":
            record = dragon.build_save_record()
            if str(record.dragon_id) != "Cogfang":
                raise RuntimeError(f"Wrong saved DragonId: {record.dragon_id}")
            restore_transform = unreal.Transform(
                location=unreal.Vector(550.0, 0.0, 120.0),
                rotation=unreal.Rotator(0.0, 0.0, 0.0),
                scale=unreal.Vector(1.0, 1.0, 1.0))
            restored = unreal.WyrmDragonCharacter.spawn_wyrm_dragon(
                world, unreal.WyrmDragonRole.ALLIED_COMPANION, restore_transform)
            restored.restore_from_save_record(record, player)
            if str(restored.dragon_id) != "Cogfang" or not restored.has_supported_rig_profile():
                raise RuntimeError("Cogfang identity/profile did not restore")
            if len(restored.follower_mesh_components) != 34:
                raise RuntimeError("Cogfang follower assembly did not restore")
            finish("COG-05.SaveIdentity", {
                "saved_dragon_id": str(record.dragon_id),
                "restored_dragon_id": str(restored.dragon_id),
                "restored_followers": len(restored.follower_mesh_components),
            })
            restored.destroy_actor()
            report["status"] = "PASS"
            write_report()
            finished = True
            level_editor.editor_request_end_play()
            return

    except Exception as exc:
        log(f"PIE proof failed in {stage}: {exc}\n{traceback.format_exc()}")
        report["status"] = "FAIL"
        report["failed_stage"] = stage
        write_report()
        finished = True
        level_editor.editor_request_end_play()


tick_handle = unreal.register_slate_post_tick_callback(pie_tick)
log("Starting focused Cogfang profile PIE proof")
level_editor.editor_request_begin_play()
