"""Exercise the optional House Mark investigation and Deathmark in live PIE."""

import json
from pathlib import Path

import unreal


ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
MAP = "/Game/WYRMFALL/World/Regions/L_CogspireHarbor"
REPORT = ROOT / "Saved/Diagnostics/WP23_6_house_mark_deathmark_proof.json"


def close_enough(actual, expected, tolerance=0.1):
    return abs(actual - expected) <= tolerance


def main():
    levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    editor = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
    editor_actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    if not levels.load_level(MAP):
        raise RuntimeError("Could not load " + MAP)

    target = editor_actors.spawn_actor_from_class(
        unreal.WyrmEnemyCharacter, unreal.Vector(700, -700, 320), unreal.Rotator())
    target.set_actor_label("DIAG_WP23_6_DeathmarkTarget")
    state = {
        "kind": "wp23_6_house_mark_deathmark_optional_slice",
        "engine": unreal.SystemLibrary.get_engine_version(),
        "map": MAP,
        "status": "RUNNING",
        "acceptance": {},
        "not_claimed": [
            "Chef Aurelio investigation", "Carver's Precision",
            "interactive keyboard or gamepad walkthrough", "new packaged build",
        ],
    }
    REPORT.parent.mkdir(parents=True, exist_ok=True)
    REPORT.write_text(json.dumps(state, indent=2) + "\n", encoding="utf-8")
    stage = "wait_pie"
    settled_ticks = 0

    def tick(_delta):
        nonlocal stage, settled_ticks
        if stage == "ending":
            if not levels.is_in_play_in_editor():
                unreal.unregister_slate_post_tick_callback(handle)
                editor_actors.destroy_actor(target)
                unreal.SystemLibrary.quit_editor()
            return

        world = editor.get_game_world()
        if not world or not levels.is_in_play_in_editor():
            return
        settled_ticks += 1
        if settled_ticks < 60:
            return

        players = unreal.GameplayStatics.get_all_actors_of_class(world, unreal.WyrmCharacter)
        house_marks = unreal.GameplayStatics.get_all_actors_of_class(world, unreal.WyrmHouseMarkCharacter)
        targets = [actor for actor in unreal.GameplayStatics.get_all_actors_of_class(
            world, unreal.WyrmEnemyCharacter)
            if actor.get_actor_label() == "DIAG_WP23_6_DeathmarkTarget"]
        adapters = unreal.GameplayStatics.get_all_actors_of_class(world, unreal.WyrmGeoForgeAdapter)
        if len(players) != 1 or len(house_marks) != 1 or len(targets) != 1 or len(adapters) != 1:
            return

        player = players[0]
        house_mark = house_marks[0]
        marked_target = targets[0]
        adapter = adapters[0]
        region = unreal.WyrmCogspireSubsystem.get_cogspire_subsystem(world)
        if not region:
            raise RuntimeError("Cogspire owner unavailable in PIE")

        player.set_actor_location(unreal.Vector(600, -700, 320), False, False)
        marked_target.set_actor_location(unreal.Vector(760, -700, 320), False, False)
        region.reset_cogspire_state()
        investigation = (
            region.record_arrival() and
            house_mark.examine_victim_evidence(player) and
            house_mark.examine_relic_trade_evidence(player) and
            house_mark.identify_house_mark(player) and
            house_mark.begin_confrontation(player))
        gas_defeat = unreal.WyrmMeleeAttackAbility.apply_damage_effect(
            player.get_ability_system(), house_mark.get_ability_system(), 2000.0)
        resolved = (
            house_mark.is_defeated() and
            region.has_fact("cogspire.house_mark.defeated") and
            region.has_receipt("cogspire.house_mark.defeat_committed"))
        manifested = house_mark.manifest_deathmark(player)
        duplicate_rejected = not house_mark.manifest_deathmark(player)

        equipped = player.equip_echo("Deathmark")
        player_attrs = player.get_attributes()
        target_attrs = marked_target.get_attributes()
        player_attrs.set_current_focus(100.0)
        player_attrs.set_current_power(20.0)
        target_attrs.set_current_max_health(400.0)
        target_attrs.set_current_health(400.0)
        target_attrs.set_current_armor(0.0)
        activated = player.activate_deathmark(marked_target)
        focus_after = player_attrs.get_current_focus()
        mark_duration = player.get_deathmark_remaining_duration()
        cooldown = player.get_deathmark_remaining_cooldown()

        ordinary_hit = unreal.WyrmMeleeAttackAbility.apply_damage_effect(
            player.get_ability_system(), marked_target.get_ability_system(), 10.0)
        ordinary_health = target_attrs.get_current_health()
        ordinary_preserves_mark = player.get_marked_deathmark_target() == marked_target

        target_attrs.set_current_health(400.0)
        eligible_hit = unreal.WyrmMeleeAttackAbility.apply_eligible_weapon_damage_effect(
            player.get_ability_system(), marked_target.get_ability_system(), 100.0)
        target_health_after = target_attrs.get_current_health()
        consumed = player.get_marked_deathmark_target() is None
        cooldown_rejects = not player.activate_deathmark(marked_target)

        snapshot = unreal.WyrmSaveSubsystem.create_snapshot_object(
            "WP23_6_HouseMarkMemory", player, adapter, world)
        saved_facts = [str(item) for item in snapshot.cogspire_record.known_facts]
        saved_cooldown = snapshot.character_record.deathmark_remaining_cooldown
        region.reset_cogspire_state()
        player.restore_echo_state([], unreal.Name("None"), False, 0.0, 0.0)
        player.restore_deathmark_state(0.0)
        cleared = not player.is_echo_unlocked("Deathmark") and not region.has_fact("echo.deathmark")
        restored = unreal.WyrmSaveSubsystem.apply_snapshot_object(snapshot, player, adapter, world)
        recovered = (
            restored and player.is_echo_unlocked("Deathmark") and
            region.has_fact("echo.deathmark") and
            region.has_receipt("cogspire.house_mark.deathmark_manifested") and
            close_enough(player.get_deathmark_remaining_cooldown(), saved_cooldown, 0.2))

        state["acceptance"] = {
            "supplied_house_mark_presentation_loaded": bool(
                house_mark.get_presentation_mesh().skeletal_mesh is not None),
            "ordered_optional_investigation_commits_once": bool(
                investigation and gas_defeat and resolved and manifested and duplicate_rejected),
            "gas_activation_costs_20_focus": bool(
                equipped and activated and close_enough(focus_after, 80.0)),
            "visible_hostile_mark_lasts_six_seconds": bool(
                5.8 <= mark_duration <= 6.0 and
                player.has_matching_gameplay_tag("Cooldown.Echo.Deathmark")),
            "ordinary_damage_does_not_consume_mark": bool(
                ordinary_hit and close_enough(ordinary_health, 390.0) and ordinary_preserves_mark),
            "eligible_direct_hit_adds_one_power_and_consumes": bool(
                eligible_hit and close_enough(target_health_after, 280.0) and consumed),
            "twelve_second_cooldown_rejects_reactivation": bool(
                11.8 <= cooldown <= 12.0 and cooldown_rejects),
            "schema8_restores_unlock_facts_and_cooldown": bool(
                snapshot and snapshot.schema_version == 8 and "echo.deathmark" in saved_facts and
                cleared and recovered),
            "optional_route_does_not_complete_cogspire": bool(
                not region.has_fact("cogspire.region_complete")),
        }
        state["measurements"] = {
            "focus_after_activation": focus_after,
            "mark_duration_seconds": mark_duration,
            "cooldown_seconds": cooldown,
            "source_power": player_attrs.get_current_power(),
            "eligible_base_raw_damage": 100.0,
            "target_health_after_eligible_hit": target_health_after,
            "saved_cooldown_seconds": saved_cooldown,
            "schema": snapshot.schema_version,
        }
        state["status"] = "PASS" if all(state["acceptance"].values()) else "FAIL"
        REPORT.write_text(json.dumps(state, indent=2) + "\n", encoding="utf-8")
        stage = "ending"
        levels.editor_request_end_play()

    handle = unreal.register_slate_post_tick_callback(tick)
    levels.editor_request_begin_play()
    print("WP-23.6 House Mark / Deathmark PIE proof started")


main()
