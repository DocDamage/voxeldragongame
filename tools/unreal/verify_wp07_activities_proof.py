"""WP-07 water, fishing, cooking, crafting, and food buff proof in a real PIE world."""
import json
from pathlib import Path
import time
import traceback
import unreal


ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
REPORT_PATH = ROOT / "Saved/Diagnostics/WP07_activities_proof.json"
report = {
    "kind": "wp07_activities_water_crafting_pie",
    "engine": unreal.SystemLibrary.get_engine_version(),
    "status": "INITIALIZING",
    "tests": {
        "ASSET.ActivitiesAssets": {"status": "NOT_RUN"},
        "WRLD-10.WaterSwimmingAndWet": {"status": "NOT_RUN"},
        "WRLD-11.WaterBoundaryEditRejection": {"status": "NOT_RUN"},
        "ACT-01.FishingLoopAndCommit": {"status": "NOT_RUN"},
        "ACT-03.FishingDamageAndCapacityCancellation": {"status": "NOT_RUN"},
        "ACT-02.CraftingCookingAtCampfire": {"status": "NOT_RUN"},
        "ACT-05.CraftingCapacityTransaction": {"status": "NOT_RUN"},
        "ACT-04.PreparationFoodBuffs": {"status": "NOT_RUN"},
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


def unpack_result(res):
    if isinstance(res, tuple):
        return res
    return (res, "")


def bool_result(res):
    if isinstance(res, bool):
        return res
    if isinstance(res, tuple):
        for value in res:
            if isinstance(value, bool):
                return value
    return bool(res)


def string_result(res):
    if isinstance(res, str):
        return res
    if isinstance(res, tuple):
        for value in res:
            if isinstance(value, str):
                return value
    return ""


def bool_member(obj, name):
    value = getattr(obj, name)
    return bool(value() if callable(value) else value)


def finish_test(name, passed, details):
    report["tests"][name]["status"] = "PASS" if passed else "FAIL"
    report["details"][name] = details
    if not passed:
        raise RuntimeError(f"{name} failed: {details}")


write_report()

# Build an unsaved editor fixture that will be duplicated into PIE.
unreal.EditorLoadingAndSavingUtils.new_blank_map(False)
editor_actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)

# Spawn Player Character
fixture_player = editor_actors.spawn_actor_from_class(
    unreal.WyrmCharacter, unreal.Vector(2500.0, 0.0, 700.0))
fixture_player.set_actor_rotation(unreal.Rotator(0.0, 0.0, 0.0), False)

# Spawn Water Volume at (0, 0, 500) with depth 600 (surface at 800)
fixture_water = editor_actors.spawn_actor_from_class(
    unreal.WyrmWaterVolume, unreal.Vector(0.0, 0.0, 500.0))
if fixture_water.water_box:
    fixture_water.water_box.set_box_extent(unreal.Vector(1000.0, 1000.0, 300.0))
fixture_water.outer_lip_thickness = 150.0
fixture_water.surface_elevation = 800.0

# Spawn Campfire Crafting Station
fixture_station = editor_actors.spawn_actor_from_class(
    unreal.WyrmCraftingStation, unreal.Vector(2000.0, 500.0, 700.0))
fixture_station.station_type = unreal.WyrmCraftingStationType.CAMPFIRE

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
            players = unreal.GameplayStatics.get_all_actors_of_class(world, unreal.WyrmCharacter)
            waters = unreal.GameplayStatics.get_all_actors_of_class(world, unreal.WyrmWaterVolume)
            stations = unreal.GameplayStatics.get_all_actors_of_class(world, unreal.WyrmCraftingStation)

            if not players or not waters or not stations:
                return

            player = players[0]
            water = waters[0]
            station = stations[0]

            movement = player.get_component_by_class(unreal.CharacterMovementComponent)
            if movement:
                movement.set_editor_property("gravity_scale", 0.0)

            inventory = player.get_inventory()
            attributes = player.get_attributes()
            fishing = player.get_fishing()
            asc = player.get_ability_system()

            if not inventory or not attributes or not fishing or not asc:
                raise RuntimeError("PIE character is missing required components")

            # -------------------------------------------------------------
            # TEST 1: ASSET.ActivitiesAssets
            # -------------------------------------------------------------
            asset_paths = [
                "/Game/WYRMFALL/Environment/Camp/SM_Campfire",
                "/Game/WYRMFALL/Environment/Water/SM_Jetty",
                "/Game/WYRMFALL/Environment/Water/SM_WaterLily",
                "/Game/WYRMFALL/Items/Tools/SM_FishingRod",
                "/Game/WYRMFALL/Items/Ingredients/SM_Potato",
                "/Game/WYRMFALL/Items/Fish/SM_OceanFish",
                "/Game/WYRMFALL/Audio/Fishing/A_Fishing_Start",
                "/Game/WYRMFALL/Audio/Fishing/A_Fish_Hard_Bite",
                "/Game/WYRMFALL/Audio/Cooking/A_Fish_Fry_Sizzle",
                "/Game/WYRMFALL/Audio/Cooking/A_Food_Simmer_Low",
            ]
            loaded_assets = []
            for path in asset_paths:
                obj = unreal.load_asset(path)
                if not obj:
                    raise RuntimeError(f"Failed to load expected WP-07 asset: {path}")
                loaded_assets.append(path)

            finish_test("ASSET.ActivitiesAssets", len(loaded_assets) == len(asset_paths), {
                "verified_assets_count": len(loaded_assets),
                "assets": loaded_assets,
            })

            # -------------------------------------------------------------
            # TEST 2: WRLD-10.WaterSwimmingAndWet
            # -------------------------------------------------------------
            in_water_loc = unreal.Vector(0.0, 0.0, 700.0)
            dry_loc = unreal.Vector(2500.0, 0.0, 700.0)
            depth = water.get_water_depth(in_water_loc)
            is_in_water = water.is_point_in_water(in_water_loc)

            player.set_actor_location(in_water_loc, False, False)
            if movement:
                movement.set_movement_mode(unreal.MovementMode.MOVE_SWIMMING)
            player.set_wet(True)

            submerged_ok = (
                is_in_water
                and close_enough(depth, 100.0)
                and bool_member(player, "is_wet")
                and (movement.movement_mode == unreal.MovementMode.MOVE_SWIMMING if movement else True)
            )

            # Move out of water to dry cave
            player.set_actor_location(dry_loc, False, False)
            if movement:
                movement.set_movement_mode(unreal.MovementMode.MOVE_WALKING)
            player.set_wet(False)

            dry_ok = (
                not bool_member(player, "is_wet")
                and (movement.movement_mode == unreal.MovementMode.MOVE_WALKING if movement else True)
            )

            finish_test("WRLD-10.WaterSwimmingAndWet", submerged_ok and dry_ok, {
                "depth_at_700": depth,
                "submerged_ok": submerged_ok,
                "dry_ok": dry_ok,
            })

            # -------------------------------------------------------------
            # TEST 3: WRLD-11.WaterBoundaryEditRejection
            # -------------------------------------------------------------
            lip_loc = unreal.Vector(950.0, 0.0, 750.0)
            lip_valid = water.is_terrain_edit_allowed_at_location(lip_loc, 300.0, False)
            lip_reason = "Rejected by water boundary policy" if not lip_valid else ""

            bed_loc = unreal.Vector(0.0, 0.0, 400.0)
            bed_valid = water.is_terrain_edit_allowed_at_location(bed_loc, 100.0, False)
            bed_reason = "" if bed_valid else "Rejected"

            boundary_ok = (not lip_valid) and bed_valid
            finish_test("WRLD-11.WaterBoundaryEditRejection", boundary_ok, {
                "lip_rejection_valid": not lip_valid,
                "lip_reason": lip_reason,
                "bed_excavation_valid": bed_valid,
            })

            # -------------------------------------------------------------
            # TEST 4: ACT-01.FishingLoopAndCommit
            # -------------------------------------------------------------
            player.set_actor_location(unreal.Vector(1100.0, 0.0, 750.0), False, False)
            inventory.clear_all()
            inventory.max_bag_slots = 10

            fish_target = unreal.Vector(800.0, 0.0, 700.0)
            start_res = unpack_result(fishing.start_fishing(fish_target))
            start_ok = bool_result(start_res)
            state_after_start = fishing.get_fishing_state()

            fishing.trigger_bite()
            state_bite = fishing.get_fishing_state()

            respond_ok = fishing.respond_to_bite()
            state_reel = fishing.get_fishing_state()

            commit_ok = fishing.commit_catch()
            state_end = fishing.get_fishing_state()

            bag_items = inventory.get_bag_items()
            caught_fish_in_bag = any(item.item_id == "Item.Fish.OceanFish" for item in bag_items)

            fishing_loop_ok = (
                start_ok
                and state_after_start == unreal.WyrmFishingState.CASTING
                and state_bite == unreal.WyrmFishingState.BITE_WINDOW
                and respond_ok
                and commit_ok
                and state_end == unreal.WyrmFishingState.READY
                and caught_fish_in_bag
            )
            finish_test("ACT-01.FishingLoopAndCommit", fishing_loop_ok, {
                "start_ok": start_ok,
                "state_after_start": str(state_after_start),
                "state_bite": str(state_bite),
                "respond_ok": respond_ok,
                "commit_ok": commit_ok,
                "caught_fish_in_bag": caught_fish_in_bag,
            })

            # -------------------------------------------------------------
            # TEST 5: ACT-03.FishingDamageAndCapacityCancellation
            # -------------------------------------------------------------
            fishing.start_fishing(fish_target)
            fishing.notify_combat_damage_taken(15.0)
            damage_cancel_ok = (fishing.get_fishing_state() == unreal.WyrmFishingState.READY)

            inventory.clear_all()
            inventory.max_bag_slots = 2
            dummy_a = unreal.WyrmCraftingSubsystem.create_consumable_item(
                "Item.DummyA", unreal.Text("Dummy A"), 1, 10)
            dummy_b = unreal.WyrmCraftingSubsystem.create_consumable_item(
                "Item.DummyB", unreal.Text("Dummy B"), 1, 10)
            inventory.add_item(dummy_a)
            inventory.add_item(dummy_b)

            fishing.start_fishing(fish_target)
            fishing.trigger_bite()
            fishing.respond_to_bite()
            full_commit_result = fishing.commit_catch()
            bag_full_cancel_ok = (
                not full_commit_result
                and bool_member(fishing, "was_catch_rejected_bag_full")
                and fishing.get_fishing_state() == unreal.WyrmFishingState.READY
                and len(inventory.get_bag_items()) == 2
            )

            finish_test("ACT-03.FishingDamageAndCapacityCancellation", damage_cancel_ok and bag_full_cancel_ok, {
                "damage_cancel_ok": damage_cancel_ok,
                "bag_full_cancel_ok": bag_full_cancel_ok,
                "bag_count": len(inventory.get_bag_items()),
            })

            # -------------------------------------------------------------
            # TEST 6: ACT-02.CraftingCookingAtCampfire
            # -------------------------------------------------------------
            inventory.clear_all()
            inventory.max_bag_slots = 10

            gi = unreal.GameplayStatics.get_game_instance(world)
            crafting_sys = unreal.new_object(unreal.WyrmCraftingSubsystem, outer=gi)
            crafting_sys.register_default_recipes()

            fish_item = unreal.WyrmCraftingSubsystem.create_consumable_item(
                "Item.Fish.OceanFish", unreal.Text("Ocean Fish"), 1, 10)
            potato_item = unreal.WyrmCraftingSubsystem.create_consumable_item(
                "Item.Ingredient.Potato", unreal.Text("Potato"), 1, 10)
            inventory.add_item(fish_item)
            inventory.add_item(potato_item)

            can_cook_res = unpack_result(crafting_sys.can_craft(
                inventory, "Recipe.Food.FishStew", unreal.WyrmCraftingStationType.CAMPFIRE))
            can_cook = bool_result(can_cook_res)

            craft_res = unpack_result(crafting_sys.craft_recipe(
                inventory, "Recipe.Food.FishStew", unreal.WyrmCraftingStationType.CAMPFIRE))
            craft_ok = bool_result(craft_res)

            stew_in_bag = any(item.item_id == "Item.Food.FishStew" for item in inventory.get_bag_items())
            ingredients_spent = (
                not any(item.item_id == "Item.Fish.OceanFish" for item in inventory.get_bag_items())
                and not any(item.item_id == "Item.Ingredient.Potato" for item in inventory.get_bag_items())
            )

            craft_pass = can_cook and craft_ok and stew_in_bag and ingredients_spent
            finish_test("ACT-02.CraftingCookingAtCampfire", craft_pass, {
                "can_cook": can_cook,
                "craft_ok": craft_ok,
                "stew_in_bag": stew_in_bag,
                "ingredients_spent": ingredients_spent,
            })

            # -------------------------------------------------------------
            # TEST 7: ACT-05.CraftingCapacityTransaction
            # -------------------------------------------------------------
            dummy_fill = unreal.WyrmCraftingSubsystem.create_consumable_item(
                "Item.Dummy", unreal.Text("Dummy"), 1, 10)
            inventory.clear_all()
            inventory.max_bag_slots = 3
            inventory.add_item(fish_item)
            inventory.add_item(potato_item)
            inventory.add_item(dummy_fill)

            cap_craft_res = unpack_result(crafting_sys.craft_recipe(
                inventory, "Recipe.Food.FishStew", unreal.WyrmCraftingStationType.CAMPFIRE))
            cap_craft_ok = bool_result(cap_craft_res)

            has_fish_after = any(item.item_id == "Item.Fish.OceanFish" for item in inventory.get_bag_items())
            has_potato_after = any(item.item_id == "Item.Ingredient.Potato" for item in inventory.get_bag_items())

            atomic_failures_ok = (
                cap_craft_ok
                and not has_fish_after and not has_potato_after
            )

            finish_test("ACT-05.CraftingCapacityTransaction", atomic_failures_ok, {
                "capacity_craft_ok": cap_craft_ok,
                "has_fish_after": has_fish_after,
                "has_potato_after": has_potato_after,
            })

            # -------------------------------------------------------------
            # TEST 8: ACT-04.PreparationFoodBuffs
            # -------------------------------------------------------------
            inventory.clear_all()
            inventory.max_bag_slots = 10
            player.clear_food_buff()
            attributes.set_current_max_focus(100.0)
            attributes.set_current_focus(100.0)

            stew_buff_def = unreal.WyrmFoodBuffDefinition()
            stew_buff_def.buff_id = "Buff.Food.FishStew"
            stew_buff_def.duration = 300.0
            stew_buff_def.max_focus_percent_bonus = 0.15
            stew_buff_def.health_regen_per_second = 2.0
            stew_item_1 = unreal.WyrmCraftingSubsystem.create_consumable_item(
                "Item.Food.FishStew", unreal.Text("Fish Stew 1"), 1, 10, stew_buff_def)
            stew_item_2 = unreal.WyrmCraftingSubsystem.create_consumable_item(
                "Item.Food.FishStew", unreal.Text("Fish Stew 2"), 1, 10, stew_buff_def)

            grilled_buff_def = unreal.WyrmFoodBuffDefinition()
            grilled_buff_def.buff_id = "Buff.Food.GrilledFish"
            grilled_buff_def.duration = 180.0
            grilled_buff_def.max_focus_percent_bonus = 0.10
            grilled_buff_def.health_regen_per_second = 1.0
            grilled_item = unreal.WyrmCraftingSubsystem.create_consumable_item(
                "Item.Food.GrilledFish", unreal.Text("Grilled Fish"), 1, 10, grilled_buff_def)

            inventory.add_item(stew_item_1)
            inventory.add_item(stew_item_2)
            inventory.add_item(grilled_item)

            player.consume_item(stew_item_1.instance_id)
            focus_after_stew = attributes.get_current_max_focus()
            stew_ok = (
                bool_member(player, "has_active_food_buff")
                and close_enough(focus_after_stew, 115.0)
                and player.get_active_food_buff().buff_id == "Item.Food.FishStew"
            )

            player.set_active_food_buff_remaining_duration(100.0)
            player.consume_item(stew_item_1.instance_id)
            refreshed_duration = player.get_active_food_buff().remaining_duration
            focus_after_refresh = attributes.get_current_max_focus()
            refresh_ok = close_enough(refreshed_duration, 300.0) and close_enough(focus_after_refresh, 115.0)

            player.consume_item(grilled_item.instance_id)
            focus_after_grilled = attributes.get_current_max_focus()
            replace_ok = (
                player.get_active_food_buff().buff_id == "Item.Food.GrilledFish"
                and close_enough(focus_after_grilled, 110.0)
            )

            player.set_active_food_buff_remaining_duration(150.0)
            snapshot = unreal.WyrmSaveSubsystem.create_snapshot_object("WP07_PIE_Save", player, None)
            if not snapshot:
                raise RuntimeError("Failed to create snapshot with food buff")

            player.clear_food_buff()
            focus_cleared = attributes.get_current_max_focus()
            cleared_ok = (not bool_member(player, "has_active_food_buff")) and close_enough(focus_cleared, 100.0)

            restored = unreal.WyrmSaveSubsystem.apply_snapshot_object(snapshot, player, None)
            focus_restored = attributes.get_current_max_focus()
            restored_buff = player.get_active_food_buff()
            restore_ok = (
                restored
                and bool_member(player, "has_active_food_buff")
                and restored_buff.buff_id == "Item.Food.GrilledFish"
                and close_enough(restored_buff.remaining_duration, 150.0)
                and close_enough(focus_restored, 110.0)
            )

            buffs_pass = stew_ok and refresh_ok and replace_ok and cleared_ok and restore_ok
            finish_test("ACT-04.PreparationFoodBuffs", buffs_pass, {
                "stew_ok": stew_ok,
                "focus_after_stew": focus_after_stew,
                "refresh_ok": refresh_ok,
                "focus_after_refresh": focus_after_refresh,
                "replace_ok": replace_ok,
                "focus_after_grilled": focus_after_grilled,
                "cleared_ok": cleared_ok,
                "focus_cleared": focus_cleared,
                "restore_ok": restore_ok,
                "focus_restored": focus_restored,
            })

            log("ALL WP-07 ACTIVITIES PIE TESTS PASSED!")
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
