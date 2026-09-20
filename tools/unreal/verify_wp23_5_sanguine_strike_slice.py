"""Exercise the bounded Sanguine Strike Echo in live PIE."""

import json
from pathlib import Path

import unreal


ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
MAP = "/Game/WYRMFALL/World/Regions/L_GloamingMarches"
REPORT = ROOT / "Saved/Diagnostics/WP23_5_sanguine_strike_slice_proof.json"


def close_enough(actual, expected, tolerance=0.05):
    return abs(actual - expected) <= tolerance


def main():
    levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    editor = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
    if not levels.load_level(MAP):
        raise RuntimeError("Could not load " + MAP)

    state = {
        "kind": "wp23_5_sanguine_strike_echo_slice",
        "engine": unreal.SystemLibrary.get_engine_version(),
        "map": MAP,
        "status": "RUNNING",
        "acceptance": {},
        "not_claimed": [
            "Second Turn", "Gloaming travel or save recovery", "regional completion",
            "interactive keyboard or gamepad walkthrough", "new packaged build",
        ],
    }
    REPORT.parent.mkdir(parents=True, exist_ok=True)
    REPORT.write_text(json.dumps(state, indent=2) + "\n", encoding="utf-8")
    wait_ticks = 0
    stage = "wait_pie"

    def tick(_delta):
        nonlocal wait_ticks, stage
        if stage == "ending":
            if not levels.is_in_play_in_editor():
                unreal.unregister_slate_post_tick_callback(handle)
                unreal.SystemLibrary.quit_editor()
            return
        if wait_ticks:
            wait_ticks -= 1
            return

        world = editor.get_game_world()
        if not world or not levels.is_in_play_in_editor():
            return
        terrains = unreal.GameplayStatics.get_all_actors_of_class(world, unreal.GeoForgeInfiniteTerrainActor)
        if len(terrains) != 1:
            return
        stats = terrains[0].get_runtime_render_stats()
        if (stats.get_editor_property("loaded_chunk_count") < 81 or
                stats.get_editor_property("queued_chunk_generation_count") or
                stats.get_editor_property("queued_chunk_rebuild_count") or
                stats.get_editor_property("pending_chunk_apply_count")):
            return

        players = unreal.GameplayStatics.get_all_actors_of_class(world, unreal.WyrmCharacter)
        malvaines = unreal.GameplayStatics.get_all_actors_of_class(world, unreal.WyrmCountMalvaineCharacter)
        twins = unreal.GameplayStatics.get_all_actors_of_class(world, unreal.WyrmHollowTwinCharacter)
        if len(players) != 1 or len(malvaines) != 1 or len(twins) != 2:
            return

        player = players[0]
        malvaine = malvaines[0]
        target = twins[0]
        region = unreal.WyrmGloamingSubsystem.get_gloaming_subsystem(world)
        if not region:
            raise RuntimeError("Gloaming fact owner unavailable in PIE")

        region.reset_gloaming_state()
        malvaine.reset_encounter()
        prerequisites = region.record_arrival() and region.resolve_ashgrave_extraction_seal()
        encounter_started = malvaine.begin_encounter(player)
        parley_resolved = malvaine.resolve_by_parley(player)
        manifested = malvaine.manifest_sanguine_strike(player)
        duplicate_rejected = not malvaine.manifest_sanguine_strike(player)
        unlock_recorded = (
            region.has_fact(unreal.Name("echo.sanguine_strike")) and
            region.has_receipt(unreal.Name("gloaming.malvaine.sanguine_strike_manifested")) and
            player.is_echo_unlocked("SanguineStrike"))

        equipped = player.equip_echo("SanguineStrike")
        player_attrs = player.get_attributes()
        target_attrs = target.get_attributes()
        player_attrs.set_current_max_health(100.0)
        player_attrs.set_current_health(50.0)
        player_attrs.set_current_focus(100.0)
        player_attrs.set_current_power(20.0)
        target_attrs.set_current_max_health(400.0)
        target_attrs.set_current_health(400.0)
        target_attrs.set_current_armor(0.0)

        activated = player.activate_equipped_echo()
        focus_after_activation = player_attrs.get_current_focus()
        initial_window = player.get_sanguine_strike_remaining_window()
        initial_cooldown = player.get_sanguine_strike_remaining_cooldown()
        primed_tag_after_activation = player.has_matching_gameplay_tag("State.Combat.SanguineStrikePrimed")

        ordinary_damage_applied = unreal.WyrmMeleeAttackAbility.apply_damage_effect(
            player.get_ability_system(), target.get_ability_system(), 10.0)
        ordinary_health = target_attrs.get_current_health()
        ordinary_did_not_consume = player.is_sanguine_strike_primed()
        ordinary_did_not_heal = close_enough(player_attrs.get_current_health(), 50.0)

        target_attrs.set_current_health(400.0)
        eligible_damage_applied = unreal.WyrmMeleeAttackAbility.apply_eligible_weapon_damage_effect(
            player.get_ability_system(), target.get_ability_system(), 100.0)
        target_health_after = target_attrs.get_current_health()
        player_health_after = player_attrs.get_current_health()
        prime_consumed = not player.is_sanguine_strike_primed()
        cooldown_rejects = not player.activate_equipped_echo()
        cooldown_before_unequip = player.get_sanguine_strike_remaining_cooldown()
        player.unequip_echo()
        cooldown_after_unequip = player.get_sanguine_strike_remaining_cooldown()
        cooldown_survives_unequip = close_enough(cooldown_before_unequip, cooldown_after_unequip, 0.15)

        excluded_absent = (
            not player.is_echo_unlocked("SecondTurn") and
            not region.has_fact(unreal.Name("echo.second_turn")) and
            not region.has_fact(unreal.Name("gloaming.region_complete")))

        state["acceptance"] = {
            "resolved_malvaine_route_manifests_once": bool(
                prerequisites and encounter_started and parley_resolved and manifested and
                duplicate_rejected and unlock_recorded),
            "gas_activation_costs_25_focus": bool(
                equipped and activated and close_enough(focus_after_activation, 75.0)),
            "four_second_next_hit_window": bool(
                primed_tag_after_activation and 3.8 <= initial_window <= 4.0 and prime_consumed and
                not player.has_matching_gameplay_tag("State.Combat.SanguineStrikePrimed")),
            "twelve_second_cooldown_and_unequip_retention": bool(
                11.8 <= initial_cooldown <= 12.0 and cooldown_rejects and cooldown_survives_unequip),
            "generic_secondary_or_reflected_path_does_not_consume_or_heal": bool(
                ordinary_damage_applied and close_enough(ordinary_health, 390.0) and
                ordinary_did_not_consume and ordinary_did_not_heal),
            "eligible_hit_adds_half_power_and_consumes": bool(
                eligible_damage_applied and close_enough(target_health_after, 290.0) and prime_consumed),
            "heal_uses_actual_damage_and_twelve_percent_cap": close_enough(player_health_after, 62.0),
            "second_turn_and_later_outcomes_absent": excluded_absent,
        }
        state["measurements"] = {
            "focus_before": 100.0,
            "focus_after_activation": focus_after_activation,
            "prime_window_seconds": initial_window,
            "primed_tag_after_activation": primed_tag_after_activation,
            "cooldown_seconds": initial_cooldown,
            "source_power": player_attrs.get_current_power(),
            "eligible_base_raw_damage": 100.0,
            "expected_half_power_bonus": 10.0,
            "target_health_before_eligible_hit": 400.0,
            "target_health_after_eligible_hit": target_health_after,
            "source_health_before_eligible_hit": 50.0,
            "source_health_after_eligible_hit": player_health_after,
        }
        state["status"] = "PASS" if all(state["acceptance"].values()) else "FAIL"
        REPORT.write_text(json.dumps(state, indent=2) + "\n", encoding="utf-8")
        stage = "ending"
        levels.editor_request_end_play()

    handle = unreal.register_slate_post_tick_callback(tick)
    levels.editor_request_begin_play()
    print("WP-23.5 Sanguine Strike PIE proof started")


main()
