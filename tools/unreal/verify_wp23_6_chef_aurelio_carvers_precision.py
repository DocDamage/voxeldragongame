"""Exercise the optional Chef Aurelio investigation and Carver's Precision in live PIE."""

import json
from pathlib import Path

import unreal


ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
MAP = "/Game/WYRMFALL/World/Regions/L_CogspireHarbor"
REPORT = ROOT / "Saved/Diagnostics/WP23_6_chef_aurelio_carvers_precision_proof.json"


def close_enough(actual, expected, tolerance=0.15):
    return abs(actual - expected) <= tolerance


def main():
    levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    editor = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
    editor_actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    if not levels.load_level(MAP):
        raise RuntimeError("Could not load " + MAP)

    target = editor_actors.spawn_actor_from_class(
        unreal.WyrmEnemyCharacter, unreal.Vector(700, -700, 320), unreal.Rotator())
    target.set_actor_label("DIAG_WP23_6_CarversPrecisionTarget")
    state = {
        "kind": "wp23_6_chef_aurelio_carvers_precision_optional_slice",
        "engine": unreal.SystemLibrary.get_engine_version(),
        "map": MAP,
        "status": "RUNNING",
        "acceptance": {},
        "not_claimed": [
            "interactive keyboard or gamepad walkthrough", "new packaged build",
        ],
    }
    REPORT.parent.mkdir(parents=True, exist_ok=True)
    REPORT.write_text(json.dumps(state, indent=2) + "\n", encoding="utf-8")
    stage = "wait_pie"
    settled_ticks = 0
    wound_elapsed = 0.0
    context = {}

    def tick(delta):
        nonlocal stage, settled_ticks, wound_elapsed
        if stage == "ending":
            if not levels.is_in_play_in_editor():
                unreal.unregister_slate_post_tick_callback(handle)
                editor_actors.destroy_actor(target)
                unreal.SystemLibrary.quit_editor()
            return

        world = editor.get_game_world()
        if not world or not levels.is_in_play_in_editor():
            return
        if stage == "wait_pie":
            settled_ticks += 1
            if settled_ticks < 60:
                return

            players = unreal.GameplayStatics.get_all_actors_of_class(world, unreal.WyrmCharacter)
            chefs = unreal.GameplayStatics.get_all_actors_of_class(world, unreal.WyrmChefAurelioCharacter)
            targets = [actor for actor in unreal.GameplayStatics.get_all_actors_of_class(
                world, unreal.WyrmEnemyCharacter)
                if actor.get_actor_label() == "DIAG_WP23_6_CarversPrecisionTarget"]
            adapters = unreal.GameplayStatics.get_all_actors_of_class(world, unreal.WyrmGeoForgeAdapter)
            if len(players) != 1 or len(chefs) != 1 or len(targets) != 1 or len(adapters) != 1:
                return

            player, chef, marked_target, adapter = players[0], chefs[0], targets[0], adapters[0]
            player.set_actor_location(unreal.Vector(600, -700, 320), False, False)
            marked_target.set_actor_location(unreal.Vector(760, -700, 320), False, False)
            region = unreal.WyrmCogspireSubsystem.get_cogspire_subsystem(world)
            if not region:
                raise RuntimeError("Cogspire owner unavailable in PIE")

            region.reset_cogspire_state()
            investigation = (
                region.record_arrival() and
                chef.record_patron_testimony(player) and
                chef.examine_kitchen_evidence(player) and
                chef.trace_ingredient_source(player) and
                chef.begin_confrontation(player))
            gas_defeat = unreal.WyrmMeleeAttackAbility.apply_damage_effect(
                player.get_ability_system(), chef.get_ability_system(), 2000.0)
            resolved = (
                chef.is_defeated() and
                region.has_fact("cogspire.chef_aurelio.defeated") and
                region.has_receipt("cogspire.chef_aurelio.defeat_committed"))
            manifested = chef.manifest_carvers_precision(player)
            duplicate_rejected = not chef.manifest_carvers_precision(player)

            equipped = player.equip_echo("CarversPrecision")
            player_attrs = player.get_attributes()
            target_attrs = marked_target.get_attributes()
            player_attrs.set_current_focus(100.0)
            player_attrs.set_current_power(20.0)
            target_attrs.set_current_max_health(400.0)
            target_attrs.set_current_health(400.0)
            target_attrs.set_current_armor(40.0)
            activated = player.activate_carvers_precision()
            focus_after = player_attrs.get_current_focus()
            window = player.get_carvers_precision_remaining_window()
            cooldown = player.get_carvers_precision_remaining_cooldown()

            ranged_hit = unreal.WyrmMeleeAttackAbility.apply_eligible_weapon_damage_effect(
                player.get_ability_system(), marked_target.get_ability_system(), 10.0, False)
            ranged_health = target_attrs.get_current_health()
            ranged_preserves_prime = player.is_carvers_precision_primed()

            target_attrs.set_current_health(400.0)
            melee_hit = unreal.WyrmMeleeAttackAbility.apply_eligible_weapon_damage_effect(
                player.get_ability_system(), marked_target.get_ability_system(), 100.0, True)
            direct_health = target_attrs.get_current_health()
            consumed = not player.is_carvers_precision_primed()
            cooldown_rejects = not player.activate_carvers_precision()

            context.update({
                "player": player, "chef": chef, "target": marked_target, "adapter": adapter,
                "region": region, "player_attrs": player_attrs, "target_attrs": target_attrs,
                "investigation": investigation, "gas_defeat": gas_defeat, "resolved": resolved,
                "manifested": manifested, "duplicate_rejected": duplicate_rejected,
                "equipped": equipped, "activated": activated, "focus_after": focus_after,
                "window": window, "cooldown": cooldown, "ranged_hit": ranged_hit,
                "ranged_health": ranged_health, "ranged_preserves_prime": ranged_preserves_prime,
                "melee_hit": melee_hit, "direct_health": direct_health, "consumed": consumed,
                "cooldown_rejects": cooldown_rejects,
            })
            stage = "wait_wound"
            return

        if stage == "wait_wound":
            wound_elapsed += delta
            if wound_elapsed < 3.25:
                return

            player = context["player"]
            adapter = context["adapter"]
            region = context["region"]
            target_attrs = context["target_attrs"]
            wound_health = target_attrs.get_current_health()
            snapshot = unreal.WyrmSaveSubsystem.create_snapshot_object(
                "WP23_6_ChefAurelioMemory", player, adapter, world)
            saved_facts = [str(item) for item in snapshot.cogspire_record.known_facts]
            saved_cooldown = snapshot.character_record.carvers_precision_remaining_cooldown
            region.reset_cogspire_state()
            player.restore_echo_state([], unreal.Name("None"), False, 0.0, 0.0)
            player.restore_carvers_precision_state(0.0)
            cleared = not player.is_echo_unlocked("CarversPrecision") and not region.has_fact("echo.carvers_precision")
            restored = unreal.WyrmSaveSubsystem.apply_snapshot_object(snapshot, player, adapter, world)
            recovered = (
                restored and player.is_echo_unlocked("CarversPrecision") and
                region.has_fact("echo.carvers_precision") and
                region.has_receipt("cogspire.chef_aurelio.carvers_precision_manifested") and
                close_enough(player.get_carvers_precision_remaining_cooldown(), saved_cooldown, 0.2))

            state["acceptance"] = {
                "supplied_chef_presentation_loaded": bool(
                    context["chef"].get_presentation_mesh().skeletal_mesh is not None),
                "ordered_optional_investigation_commits_once": bool(
                    context["investigation"] and context["gas_defeat"] and context["resolved"] and
                    context["manifested"] and context["duplicate_rejected"]),
                "gas_activation_costs_25_focus": bool(
                    context["equipped"] and context["activated"] and close_enough(context["focus_after"], 75.0)),
                "prime_window_is_four_seconds": bool(
                    3.8 <= context["window"] <= 4.0 and
                    player.has_matching_gameplay_tag("Cooldown.Echo.CarversPrecision")),
                "non_melee_hit_does_not_consume_prime": bool(
                    context["ranged_hit"] and close_enough(context["ranged_health"], 394.0) and
                    context["ranged_preserves_prime"]),
                "melee_hit_ignores_thirty_percent_armor_and_consumes": bool(
                    context["melee_hit"] and close_enough(context["direct_health"], 331.8182) and context["consumed"]),
                "wound_deals_point_six_power_over_three_seconds": bool(
                    close_enough(wound_health, 324.6182, 0.25)),
                "twelve_second_cooldown_rejects_reactivation": bool(
                    11.8 <= context["cooldown"] <= 12.0 and context["cooldown_rejects"]),
                "schema8_restores_unlock_facts_and_cooldown": bool(
                    snapshot and snapshot.schema_version == 8 and "echo.carvers_precision" in saved_facts and
                    cleared and recovered),
                "optional_route_does_not_complete_cogspire": bool(
                    not region.has_fact("cogspire.region_complete")),
            }
            state["measurements"] = {
                "focus_after_activation": context["focus_after"],
                "prime_window_seconds": context["window"],
                "cooldown_seconds": context["cooldown"],
                "source_power": context["player_attrs"].get_current_power(),
                "target_armor": 40.0,
                "target_health_after_non_melee_hit": context["ranged_health"],
                "target_health_after_direct_melee_hit": context["direct_health"],
                "target_health_after_wound": wound_health,
                "saved_cooldown_seconds": saved_cooldown,
                "schema": snapshot.schema_version,
            }
            state["status"] = "PASS" if all(state["acceptance"].values()) else "FAIL"
            REPORT.write_text(json.dumps(state, indent=2) + "\n", encoding="utf-8")
            stage = "ending"
            levels.editor_request_end_play()

    handle = unreal.register_slate_post_tick_callback(tick)
    levels.editor_request_begin_play()
    print("WP-23.6 Chef Aurelio / Carver's Precision PIE proof started")


main()
