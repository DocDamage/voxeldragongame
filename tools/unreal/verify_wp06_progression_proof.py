"""WP-06 progression and ranged-build proof in a real PIE world."""
import json
from pathlib import Path
import time
import traceback
import unreal


ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
REPORT_PATH = ROOT / "Saved/Diagnostics/WP06_progression_proof.json"
report = {
    "kind": "wp06_progression_ranged_build_pie",
    "engine": unreal.SystemLibrary.get_engine_version(),
    "status": "INITIALIZING",
    "tests": {
        "ASSET.RangerWeapons": {"status": "NOT_RUN"},
        "COM-08.Progression": {"status": "NOT_RUN"},
        "COM-08.KitSwitching": {"status": "NOT_RUN"},
        "COM-08.RangedCombat": {"status": "NOT_RUN"},
        "COM-08.EvadeAndCamera": {"status": "NOT_RUN"},
        "SAVE-01.Progression": {"status": "NOT_RUN"},
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
    REPORT_PATH.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")


def close_enough(a, b, tolerance=0.01):
    return abs(a - b) <= tolerance


def find_fixture_actor(world, actor_class, target_x):
    actors = unreal.GameplayStatics.get_all_actors_of_class(world, actor_class)
    if not actors:
        return None
    return min(actors, key=lambda actor: abs(actor.get_actor_location().x - target_x))


def finish_test(name, passed, details):
    report["tests"][name]["status"] = "PASS" if passed else "FAIL"
    report["details"][name] = details
    if not passed:
        raise RuntimeError(f"{name} failed: {details}")


write_report()

# Build an unsaved editor fixture that will be duplicated into PIE.
unreal.EditorLoadingAndSavingUtils.new_blank_map(False)
editor_actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
fixture_player = editor_actors.spawn_actor_from_class(
    unreal.WyrmCharacter, unreal.Vector(-1000.0, 0.0, 1000.0))
fixture_enemy = editor_actors.spawn_actor_from_class(
    unreal.WyrmEnemyCharacter, unreal.Vector(-400.0, 0.0, 1000.0))
fixture_player.set_actor_rotation(unreal.Rotator(0.0, 0.0, 0.0), False)
fixture_enemy.set_actor_rotation(unreal.Rotator(0.0, 180.0, 0.0), False)

level_editor = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
started_at = time.monotonic()
stage_started_at = started_at
stage = "init"
finished = False
context = {}


def pie_tick(_delta_seconds):
    global stage, stage_started_at, finished
    try:
        if finished:
            if not level_editor.is_in_play_in_editor():
                unreal.unregister_slate_post_tick_callback(tick_handle)
                log("PIE closed cleanly; exiting editor.")
                unreal.SystemLibrary.quit_editor()
            return

        if time.monotonic() - started_at > 60:
            raise RuntimeError(f"Timed out in PIE stage: {stage}")

        world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
        if not world or not level_editor.is_in_play_in_editor():
            return

        if stage == "init":
            player = find_fixture_actor(world, unreal.WyrmCharacter, -1000.0)
            enemy = find_fixture_actor(world, unreal.WyrmEnemyCharacter, -400.0)
            if not player or not enemy:
                return

            player.set_actor_location(unreal.Vector(-1000.0, 0.0, 1000.0), False, False)
            player.set_actor_rotation(unreal.Rotator(0.0, 0.0, 0.0), False)
            enemy.set_actor_location(unreal.Vector(-400.0, 0.0, 1000.0), False, False)
            enemy.configure_for_role(unreal.WyrmEnemyRole.MELEE_CHASER)
            for actor in (player, enemy):
                movement = actor.get_component_by_class(unreal.CharacterMovementComponent)
                if movement:
                    movement.set_editor_property("gravity_scale", 0.0)

            inventory = player.get_inventory()
            attributes = player.get_attributes()
            if not inventory or not attributes or not player.get_ability_system():
                raise RuntimeError("PIE character is missing inventory or GAS components")

            context.update({
                "world": world,
                "player": player,
                "enemy": enemy,
                "inventory": inventory,
                "attributes": attributes,
            })
            stage = "progression"
            log("PIE fixture initialized.")
            return

        player = context["player"]
        enemy = context["enemy"]
        inventory = context["inventory"]
        attributes = context["attributes"]

        if stage == "progression":
            level_1 = player.get_character_level()
            required_1 = player.get_xp_for_next_level()
            partial_leveled = player.add_experience(60.0)
            remaining_after_60 = player.get_xp_to_next_level()
            threshold_leveled = player.add_experience(50.0)
            progression_ok = (
                close_enough(level_1, 1.0)
                and close_enough(required_1, 100.0)
                and not partial_leveled
                and close_enough(remaining_after_60, 40.0)
                and threshold_leveled
                and close_enough(player.get_character_level(), 2.0)
                and close_enough(player.get_current_xp(), 10.0)
                and close_enough(player.get_xp_for_next_level(), 150.0)
                and close_enough(player.get_xp_to_next_level(), 140.0)
                and close_enough(attributes.get_current_power(), 23.0)
                and close_enough(attributes.get_current_max_health(), 108.0)
            )
            finish_test("COM-08.Progression", progression_ok, {
                "level": player.get_character_level(),
                "current_xp": player.get_current_xp(),
                "required_xp": player.get_xp_for_next_level(),
                "remaining_xp": player.get_xp_to_next_level(),
                "power": attributes.get_current_power(),
                "max_health": attributes.get_current_max_health(),
            })

            sword = unreal.WyrmInventoryComponent.roll_random_item(
                "ForgedBlade", unreal.WyrmItemType.WEAPON, 1,
                unreal.WyrmEquipSlot.MAIN_HAND)
            bow = unreal.WyrmInventoryComponent.roll_random_item(
                "RangerBow", unreal.WyrmItemType.WEAPON, 1,
                unreal.WyrmEquipSlot.MAIN_HAND)
            inventory.add_item(sword)
            inventory.add_item(bow)

            sword_equipped = inventory.equip_item(
                sword.instance_id, unreal.WyrmEquipSlot.MAIN_HAND)
            melee_family = player.get_active_weapon_family()
            initial_enemy_health = enemy.get_attributes().get_current_health()
            melee_activated = player.perform_primary_attack()
            health_after_melee = enemy.get_attributes().get_current_health()

            bow_equipped = inventory.equip_item(
                bow.instance_id, unreal.WyrmEquipSlot.MAIN_HAND)
            ranged_family = player.get_active_weapon_family()
            finish_test("COM-08.KitSwitching", (
                sword_equipped and bow_equipped
                and melee_family == unreal.WyrmWeaponFamily.MELEE1H
                and ranged_family == unreal.WyrmWeaponFamily.RANGED_BOW
            ), {
                "melee_family": str(melee_family),
                "ranged_family": str(ranged_family),
            })

            ranged_activated = player.perform_primary_attack()
            projectiles = unreal.GameplayStatics.get_all_actors_of_class(
                world, unreal.WyrmProjectile)
            bow_asset = unreal.load_asset("/Game/WYRMFALL/Items/Weapons/SM_Bow")
            arrow_asset = unreal.load_asset("/Game/WYRMFALL/Items/Weapons/SM_Arrow")
            bow_texture = unreal.load_asset("/Game/WYRMFALL/Items/Weapons/T_Bow")
            arrow_texture = unreal.load_asset("/Game/WYRMFALL/Items/Weapons/T_Arrow")
            projectile_mesh = None
            if projectiles:
                mesh_component = projectiles[0].get_editor_property("mesh_comp")
                if mesh_component:
                    projectile_mesh = mesh_component.get_editor_property("static_mesh")
            finish_test("ASSET.RangerWeapons", (
                bow_asset is not None and arrow_asset is not None
                and bow_texture is not None and arrow_texture is not None
                and projectile_mesh == arrow_asset
            ), {
                "bow_mesh": bow_asset.get_path_name() if bow_asset else None,
                "arrow_mesh": arrow_asset.get_path_name() if arrow_asset else None,
                "bow_texture": bow_texture.get_path_name() if bow_texture else None,
                "arrow_texture": arrow_texture.get_path_name() if arrow_texture else None,
                "projectile_uses_arrow_mesh": projectile_mesh == arrow_asset,
            })
            context.update({
                "initial_enemy_health": initial_enemy_health,
                "health_after_melee": health_after_melee,
                "melee_activated": melee_activated,
                "ranged_activated": ranged_activated,
                "projectile_spawned": len(projectiles) > 0,
            })
            stage = "wait_projectile"
            stage_started_at = time.monotonic()
            return

        if stage == "wait_projectile":
            enemy_health = enemy.get_attributes().get_current_health()
            if close_enough(enemy_health, context["initial_enemy_health"]):
                if time.monotonic() - stage_started_at > 3.0:
                    raise RuntimeError("Ranged projectile did not damage the hostile target")
                return

            finish_test("COM-08.RangedCombat", (
                context["melee_activated"]
                and close_enough(context["health_after_melee"], context["initial_enemy_health"])
                and context["ranged_activated"]
                and context["projectile_spawned"]
                and enemy_health < context["initial_enemy_health"]
            ), {
                "distance_cm": 600.0,
                "melee_activated": context["melee_activated"],
                "melee_health_delta": context["initial_enemy_health"] - context["health_after_melee"],
                "ranged_activated": context["ranged_activated"],
                "projectile_spawned": context["projectile_spawned"],
                "ranged_health_delta": context["initial_enemy_health"] - enemy_health,
            })

            player.set_camera_mode(unreal.WyrmCameraMode.TOP_DOWN)
            top_down_ok = player.get_camera_mode() == unreal.WyrmCameraMode.TOP_DOWN
            player.set_camera_mode(unreal.WyrmCameraMode.THIRD_PERSON)
            third_person_ok = player.get_camera_mode() == unreal.WyrmCameraMode.THIRD_PERSON
            context.update({
                "top_down_ok": top_down_ok,
                "third_person_ok": third_person_ok,
                "evade_activated": player.perform_evade(),
                "evade_start": player.get_actor_location(),
            })
            stage = "wait_evade"
            stage_started_at = time.monotonic()
            return

        if stage == "wait_evade":
            evade_distance = (player.get_actor_location() - context["evade_start"]).length()
            if evade_distance < 1.0 and time.monotonic() - stage_started_at <= 1.0:
                return
            finish_test("COM-08.EvadeAndCamera", (
                context["top_down_ok"] and context["third_person_ok"]
                and context["evade_activated"] and evade_distance >= 1.0
            ), {
                "top_down_selected": context["top_down_ok"],
                "third_person_selected": context["third_person_ok"],
                "evade_activated": context["evade_activated"],
                "evade_distance_cm": evade_distance,
            })

            snapshot = unreal.WyrmSaveSubsystem.create_snapshot_object(
                "WP06_PIE_Memory", player, None)
            if not snapshot:
                raise RuntimeError("Failed to create WP-06 in-memory snapshot")
            saved_level = player.get_character_level()
            saved_xp = player.get_current_xp()
            player.set_character_level(1.0)
            player.set_current_xp(0.0)
            restored = unreal.WyrmSaveSubsystem.apply_snapshot_object(
                snapshot, player, None)
            finish_test("SAVE-01.Progression", (
                restored
                and close_enough(player.get_character_level(), saved_level)
                and close_enough(player.get_current_xp(), saved_xp)
            ), {
                "restored": restored,
                "level": player.get_character_level(),
                "current_xp": player.get_current_xp(),
            })

            log("ALL WP-06 PIE TESTS PASSED!")
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
