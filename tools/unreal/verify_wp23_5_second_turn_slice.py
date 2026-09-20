"""Exercise the bounded Second Turn Echo in live PIE."""

import json
from pathlib import Path

import unreal


ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
MAP = "/Game/WYRMFALL/World/Regions/L_GloamingMarches"
REPORT = ROOT / "Saved/Diagnostics/WP23_5_second_turn_slice_proof.json"


def close_enough(actual, expected, tolerance=0.1):
    return abs(actual - expected) <= tolerance


def main():
    levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    editor = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
    if not levels.load_level(MAP):
        raise RuntimeError("Could not load " + MAP)

    state = {
        "kind": "wp23_5_second_turn_echo_slice",
        "engine": unreal.SystemLibrary.get_engine_version(),
        "map": MAP,
        "status": "RUNNING",
        "acceptance": {},
        "not_claimed": [
            "Gloaming travel or save recovery", "regional completion",
            "interactive keyboard or gamepad walkthrough", "new packaged build",
        ],
    }
    REPORT.parent.mkdir(parents=True, exist_ok=True)
    REPORT.write_text(json.dumps(state, indent=2) + "\n", encoding="utf-8")
    stage = "wait_pie"
    elapsed = 0.0
    pre_repeat_stable = True
    post_repeat_health = None
    measurements = {}
    refs = {}

    def finish():
        nonlocal stage
        state["measurements"] = measurements
        state["status"] = "PASS" if all(state["acceptance"].values()) else "FAIL"
        REPORT.write_text(json.dumps(state, indent=2) + "\n", encoding="utf-8")
        stage = "ending"
        levels.editor_request_end_play()

    def tick(delta):
        nonlocal stage, elapsed, pre_repeat_stable, post_repeat_health
        if stage == "ending":
            if not levels.is_in_play_in_editor():
                unreal.unregister_slate_post_tick_callback(handle)
                unreal.SystemLibrary.quit_editor()
            return

        world = editor.get_game_world()
        if not world or not levels.is_in_play_in_editor():
            return

        if stage == "wait_repeat":
            elapsed += delta
            target_attrs = refs["target_attrs"]
            player = refs["player"]
            player_attrs = refs["player_attrs"]
            if elapsed < 0.5:
                pre_repeat_stable = pre_repeat_stable and close_enough(target_attrs.get_current_health(), 362.5)
                return
            if elapsed < 0.75:
                return
            post_repeat_health = target_attrs.get_current_health()
            measurements["observed_repeat_elapsed_seconds"] = elapsed
            measurements["target_health_after_repeat"] = post_repeat_health
            measurements["source_health_after_repeat"] = player_attrs.get_current_health()
            state["acceptance"]["delayed_half_base_repeat"] = bool(
                pre_repeat_stable and close_enough(post_repeat_health, 343.75) and
                not player.has_pending_second_turn_repeat() and
                close_enough(player.get_pending_second_turn_raw_damage(), 0.0))
            state["acceptance"]["repeat_cannot_recurse_or_trigger_other_echoes"] = bool(
                player.is_sanguine_strike_primed() and
                close_enough(player_attrs.get_current_health(), 50.0) and
                not player.is_second_turn_primed() and
                not player.has_matching_gameplay_tag("State.Combat.SecondTurnPending"))
            stage = "confirm_single_repeat"
            elapsed = 0.0
            return

        if stage == "confirm_single_repeat":
            elapsed += delta
            if elapsed < 0.75:
                return
            target_health = refs["target_attrs"].get_current_health()
            measurements["target_health_after_recursion_guard"] = target_health
            state["acceptance"]["exactly_one_repeat"] = close_enough(target_health, post_repeat_health)
            finish()
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
        twin = twins[0]
        region = unreal.WyrmGloamingSubsystem.get_gloaming_subsystem(world)
        if not region:
            raise RuntimeError("Gloaming fact owner unavailable in PIE")

        region.reset_gloaming_state()
        malvaine.reset_encounter()
        twin.reset_encounter_pair()
        prerequisites = region.record_arrival() and region.resolve_ashgrave_extraction_seal()
        malvaine_route = (
            malvaine.begin_encounter(player) and malvaine.resolve_by_parley(player) and
            malvaine.manifest_sanguine_strike(player))
        twin_route = twin.begin_encounter(player) and twin.resolve_by_release(player)
        manifested = twin.manifest_second_turn(player)
        duplicate_rejected = not twin.manifest_second_turn(player)
        unlock_recorded = (
            region.has_fact(unreal.Name("echo.second_turn")) and
            region.has_receipt(unreal.Name("gloaming.hollow_twins.second_turn_manifested")) and
            player.is_echo_unlocked("SecondTurn"))

        # Resolution made the pair invulnerable; reset only their actor fixture
        # so one Twin can serve as a deterministic GAS target without changing facts.
        twin.reset_encounter_pair()
        target = twin
        player_attrs = player.get_attributes()
        target_attrs = target.get_attributes()
        player_attrs.set_current_max_health(100.0)
        player_attrs.set_current_health(50.0)
        player_attrs.set_current_focus(100.0)
        target_attrs.set_current_max_health(400.0)
        target_attrs.set_current_health(400.0)
        target_attrs.set_current_armor(0.0)

        equipped = player.equip_echo("SecondTurn")
        activated = player.activate_equipped_echo()
        focus_after_activation = player_attrs.get_current_focus()
        cooldown_at_activation = player.get_second_turn_remaining_cooldown()
        primed_at_activation = player.is_second_turn_primed()

        ordinary_damage = unreal.WyrmMeleeAttackAbility.apply_damage_effect(
            player.get_ability_system(), target.get_ability_system(), 10.0)
        ordinary_health = target_attrs.get_current_health()
        ordinary_did_not_consume = player.is_second_turn_primed()

        target_attrs.set_current_health(400.0)
        target_attrs.set_current_armor(100.0)
        eligible_damage = unreal.WyrmMeleeAttackAbility.apply_eligible_weapon_damage_effect(
            player.get_ability_system(), target.get_ability_system(), 100.0)
        immediate_health = target_attrs.get_current_health()
        queued = player.has_pending_second_turn_repeat()
        queued_delay = player.get_second_turn_remaining_delay()
        queued_raw_damage = player.get_pending_second_turn_raw_damage()

        # Equip and prime Sanguine while the repeat is pending. The repeat must
        # use generic GAS damage and leave this separate Echo untouched.
        equipped_sanguine = player.equip_echo("SanguineStrike")
        sanguine_activated = player.activate_equipped_echo()
        second_turn_cooldown_after_unequip = player.get_second_turn_remaining_cooldown()

        excluded_absent = not region.has_fact(unreal.Name("gloaming.region_complete"))
        state["acceptance"] = {
            "resolved_twins_route_manifests_once": bool(
                prerequisites and malvaine_route and twin_route and manifested and
                duplicate_rejected and unlock_recorded),
            "gas_activation_costs_25_focus": bool(
                equipped and activated and close_enough(focus_after_activation, 75.0)),
            "fourteen_second_cooldown_survives_unequip": bool(
                13.8 <= cooldown_at_activation <= 14.0 and
                second_turn_cooldown_after_unequip > 13.5),
            "generic_secondary_or_reflected_path_does_not_consume": bool(
                ordinary_damage and close_enough(ordinary_health, 390.0) and ordinary_did_not_consume),
            "eligible_basic_hit_snapshots_half_base": bool(
                primed_at_activation and eligible_damage and close_enough(immediate_health, 362.5) and
                queued and 0.5 <= queued_delay <= 0.61 and close_enough(queued_raw_damage, 50.0)),
            "delayed_half_base_repeat": False,
            "repeat_cannot_recurse_or_trigger_other_echoes": False,
            "exactly_one_repeat": False,
            "regional_completion_absent": excluded_absent,
        }
        measurements.update({
            "focus_before": 100.0,
            "focus_after_second_turn_activation": focus_after_activation,
            "cooldown_seconds": cooldown_at_activation,
            "eligible_base_raw_damage": 100.0,
            "target_health_before_eligible_hit": 400.0,
            "target_armor_for_eligible_hit": 100.0,
            "target_health_immediately_after_eligible_hit": immediate_health,
            "snapshotted_repeat_raw_damage": queued_raw_damage,
            "repeat_delay_seconds": queued_delay,
            "sanguine_armed_during_delay": bool(equipped_sanguine and sanguine_activated),
        })
        refs.update({
            "player": player, "player_attrs": player_attrs, "target_attrs": target_attrs,
        })
        stage = "wait_repeat"
        elapsed = 0.0

    handle = unreal.register_slate_post_tick_callback(tick)
    levels.editor_request_begin_play()
    print("WP-23.5 Second Turn PIE proof started")


main()
