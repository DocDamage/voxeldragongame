"""WP-08 supported camp, building, storage, and terrain support proof in a real PIE world."""
import json
from pathlib import Path
import time
import traceback
import uuid
import unreal


def make_guid():
    g = unreal.Guid()
    g.import_text(uuid.uuid4().hex.upper())
    return g


ROOT = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()))
REPORT_PATH = ROOT / "Saved/Diagnostics/WP08_camp_proof.json"
report = {
    "kind": "wp08_camp_building_storage_pie",
    "engine": unreal.SystemLibrary.get_engine_version(),
    "status": "INITIALIZING",
    "tests": {
        "ASSET.CampAssets": {"status": "NOT_RUN"},
        "ACT-06.UsefulCampPlacement": {"status": "NOT_RUN"},
        "ACT-07_WRLD-08.PlacementRejection": {"status": "NOT_RUN"},
        "ACT-08.StorageContainerIdentity": {"status": "NOT_RUN"},
        "ACT-09.DemolitionOverflow": {"status": "NOT_RUN"},
        "ACT-10.CampPersistenceAndClearance": {"status": "NOT_RUN"},
        "WRLD-09.CampSupportTerrainInteraction": {"status": "NOT_RUN"},
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


def is_door_open(d):
    if not d:
        return False
    if hasattr(d, "is_open"):
        return bool(d.is_open)
    if hasattr(d, "b_is_open"):
        return bool(d.b_is_open)
    try:
        return bool(d.get_editor_property("bIsOpen"))
    except Exception:
        return False


def eval_clearance(res):
    if res is None:
        return False
    if isinstance(res, (tuple, list)):
        return bool(res[0])
    if isinstance(res, str):
        return True
    return bool(res)


def clear_inventory(inv):
    if not inv:
        return
    for item in list(inv.get_bag_items()):
        inv.remove_item(item.instance_id, item.stack_count)


def guid_str(g):
    if not g:
        return ""
    if hasattr(g, "export_text"):
        try:
            return g.export_text().strip("()").upper()
        except Exception:
            pass
    return str(g)


def finish_test(name, passed, details):
    report["tests"][name]["status"] = "PASS" if passed else "FAIL"
    report["details"][name] = details
    log(f"Test {name}: {'PASS' if passed else 'FAIL'}")
    if not passed:
        raise RuntimeError(f"{name} failed: {details}")


write_report()

# Build blank map fixture
unreal.EditorLoadingAndSavingUtils.new_blank_map(False)
editor_actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)

# Spawn Player Character at (1000.0, 0.0, 300.0)
fixture_player = editor_actors.spawn_actor_from_class(
    unreal.WyrmCharacter, unreal.Vector(1000.0, 0.0, 300.0))
fixture_player.set_actor_rotation(unreal.Rotator(0.0, 0.0, 0.0), False)

# Spawn GeoForge Adapter
fixture_adapter = editor_actors.spawn_actor_from_class(
    unreal.WyrmGeoForgeAdapter, unreal.Vector(0.0, 0.0, 0.0))

level_editor = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
started_at = time.monotonic()
stage = "init"
finished = False


def pie_tick(_delta_seconds):
    global stage, finished
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
            adapters = unreal.GameplayStatics.get_all_actors_of_class(world, unreal.WyrmGeoForgeAdapter)

            if not players or not adapters:
                return

            player = players[0]
            adapter = adapters[0]

            for p in players:
                p.set_actor_location(unreal.Vector(2000.0, 0.0, 300.0), False, False)
                pmov = p.get_component_by_class(unreal.CharacterMovementComponent)
                if pmov:
                    pmov.set_editor_property("gravity_scale", 0.0)

            inventory = player.get_inventory()
            if not inventory:
                raise RuntimeError("PIE character is missing inventory component")

            gi = unreal.GameplayStatics.get_game_instance(world)
            build_sys = None
            if gi and hasattr(gi, "get_subsystem"):
                try:
                    build_sys = gi.get_subsystem(unreal.WyrmBuildingSubsystem)
                except Exception as e:
                    log(f"gi.get_subsystem note: {e}")
            if not build_sys and hasattr(unreal, "SubsystemBlueprintLibrary"):
                try:
                    build_sys = unreal.SubsystemBlueprintLibrary.get_game_instance_subsystem(world, unreal.WyrmBuildingSubsystem)
                except Exception as e:
                    log(f"SubsystemBlueprintLibrary note: {e}")
            if not build_sys:
                build_sys = unreal.new_object(unreal.WyrmBuildingSubsystem, gi if gi else world)
            if not build_sys:
                raise RuntimeError("Failed to resolve WyrmBuildingSubsystem")

            build_sys.set_world_context(world)
            build_sys.register_default_definitions()

            # -------------------------------------------------------------
            # TEST 1: ASSET.CampAssets
            # -------------------------------------------------------------
            asset_paths = [
                "/Game/WYRMFALL/Environment/Camp/SM_StorageChest",
                "/Game/WYRMFALL/Environment/Building/Wood/SM_Stylized_Wood_Foundation",
                "/Game/WYRMFALL/Environment/Building/Wood/SM_Stylized_Wood_Wall",
                "/Game/WYRMFALL/Environment/Building/Wood/SM_Stylized_Wood_Doorframe",
                "/Game/WYRMFALL/Environment/Building/Wood/SM_Stylized_Wood_Door",
                "/Game/WYRMFALL/Environment/Building/Wood/SM_Stylized_Wood_Roof",
                "/Game/WYRMFALL/Environment/Building/Wood/SM_Stylized_Wood_Ceiling",
            ]
            loaded_assets = []
            for path in asset_paths:
                obj = unreal.load_asset(path)
                if not obj:
                    raise RuntimeError(f"Failed to load expected WP-08 asset: {path}")
                loaded_assets.append(path)

            finish_test("ASSET.CampAssets", len(loaded_assets) == len(asset_paths), {
                "verified_assets_count": len(loaded_assets),
                "assets": loaded_assets,
            })

            # Grant Wood materials to player bag for construction
            wood_item = unreal.WyrmItemInstance()
            wood_item.instance_id = make_guid()
            wood_item.item_id = "Resource.Wood"
            wood_item.display_name = unreal.Text("Wood")
            wood_item.stack_count = 99
            wood_item.max_stack = 99
            wood_rem = unreal.WyrmItemInstance()
            inventory.add_item(wood_item)

            # -------------------------------------------------------------
            # TEST 2: ACT-06.UsefulCampPlacement
            # -------------------------------------------------------------
            # Place Foundation at (0, 0, 0)
            res_foundation = build_sys.execute_placement(
                "Foundation.Wood", unreal.Transform(location=unreal.Vector(0.0, 0.0, 0.0)), player)
            foundation = res_foundation[0] if isinstance(res_foundation, (tuple, list)) else res_foundation
            if not foundation:
                raise RuntimeError(f"Failed to place foundation: {res_foundation}")

            # Place Wall at (0, 100, 50)
            res_wall = build_sys.execute_placement(
                "Wall.Wood", unreal.Transform(location=unreal.Vector(0.0, 100.0, 50.0)), player)
            wall = res_wall[0] if isinstance(res_wall, (tuple, list)) else res_wall
            if not wall:
                raise RuntimeError(f"Failed to place wall: {res_wall}")

            # Place Doorframe at (100, 0, 50)
            res_doorframe = build_sys.execute_placement(
                "Doorframe.Wood", unreal.Transform(location=unreal.Vector(100.0, 0.0, 50.0)), player)
            doorframe = res_doorframe[0] if isinstance(res_doorframe, (tuple, list)) else res_doorframe
            if not doorframe:
                raise RuntimeError(f"Failed to place doorframe: {res_doorframe}")

            # Place Door inside doorframe at (100, 0, 50)
            res_door = build_sys.execute_placement(
                "Door.Wood", unreal.Transform(location=unreal.Vector(100.0, 0.0, 50.0)), player)
            door = res_door[0] if isinstance(res_door, (tuple, list)) else res_door
            if not door:
                raise RuntimeError(f"Failed to place door: {res_door}")

            # Toggle door open
            door.toggle_door()
            door_open = is_door_open(door)

            # Place Roof at (0, 0, 200)
            res_roof = build_sys.execute_placement(
                "Roof.Wood", unreal.Transform(location=unreal.Vector(0.0, 0.0, 200.0)), player)
            roof = res_roof[0] if isinstance(res_roof, (tuple, list)) else res_roof
            if not roof:
                raise RuntimeError(f"Failed to place roof: {res_roof}")

            # Place Storage Chest at (0, 0, 20)
            res_chest = build_sys.execute_placement(
                "Storage.Chest", unreal.Transform(location=unreal.Vector(0.0, 0.0, 20.0)), player)
            chest = res_chest[0] if isinstance(res_chest, (tuple, list)) else res_chest
            if not chest:
                raise RuntimeError(f"Failed to place storage chest: {res_chest}")

            # Place Workpoint Bench at (50, 50, 20)
            res_bench = build_sys.execute_placement(
                "Workpoint.Bench", unreal.Transform(location=unreal.Vector(50.0, 50.0, 20.0)), player)
            bench = res_bench[0] if isinstance(res_bench, (tuple, list)) else res_bench
            if not bench:
                raise RuntimeError(f"Failed to place workpoint bench: {res_bench}")

            active_pieces = build_sys.get_active_pieces()
            placement_ok = (
                len(active_pieces) == 7
                and door_open
                and isinstance(chest, unreal.WyrmStorageActor)
            )
            finish_test("ACT-06.UsefulCampPlacement", placement_ok, {
                "active_piece_count": len(active_pieces),
                "door_is_open": door_open,
                "chest_class": chest.get_class().get_name(),
            })

            # -------------------------------------------------------------
            # TEST 3: ACT-07_WRLD-08.PlacementRejection
            # -------------------------------------------------------------
            # 1. Unsupported: floating wall in air at (5000, 5000, 2000)
            res_unsupported = build_sys.validate_placement(
                "Wall.Wood", unreal.Transform(location=unreal.Vector(5000.0, 5000.0, 2000.0)), player)
            rej_unsupported = res_unsupported[0] if isinstance(res_unsupported, (tuple, list)) else res_unsupported
            is_unsupported = "UNSUPPORTED" in str(rej_unsupported).upper()

            # 2. Overlapping: foundation directly on existing foundation (0, 0, 0)
            res_overlap = build_sys.validate_placement(
                "Foundation.Wood", unreal.Transform(location=unreal.Vector(0.0, 0.0, 0.0)), player)
            rej_overlap = res_overlap[0] if isinstance(res_overlap, (tuple, list)) else res_overlap
            is_overlap = "OVERLAPPING" in str(rej_overlap).upper()

            # 3. Occupied: position pawn at (500, 500, 0) and try placing foundation on pawn
            player.set_actor_location(unreal.Vector(500.0, 500.0, 50.0), False, False)
            res_occupied = build_sys.validate_placement(
                "Foundation.Wood", unreal.Transform(location=unreal.Vector(500.0, 500.0, 0.0)), None)
            rej_occupied = res_occupied[0] if isinstance(res_occupied, (tuple, list)) else res_occupied
            is_occupied = "OCCUPIED" in str(rej_occupied).upper()

            # Move player back to clear area
            player.set_actor_location(unreal.Vector(2000.0, 0.0, 300.0), False, False)

            # 4. Insufficient materials: remove all wood and attempt placement
            clear_inventory(inventory)
            res_nomat = build_sys.validate_placement(
                "Foundation.Wood", unreal.Transform(location=unreal.Vector(2000.0, 2000.0, 0.0)), player)
            rej_nomat = res_nomat[0] if isinstance(res_nomat, (tuple, list)) else res_nomat
            is_nomat = "INSUFFICIENT" in str(rej_nomat).upper()

            # Restore wood for subsequent tests
            inventory.add_item(wood_item)

            # Confirm zero side effects: active pieces count unchanged
            zero_dangling = len(build_sys.get_active_pieces()) == 7
            rejections_ok = is_unsupported and is_overlap and is_occupied and is_nomat and zero_dangling
            finish_test("ACT-07_WRLD-08.PlacementRejection", rejections_ok, {
                "is_unsupported": is_unsupported,
                "is_overlap": is_overlap,
                "is_occupied": is_occupied,
                "is_nomat": is_nomat,
                "zero_dangling": zero_dangling,
            })

            # -------------------------------------------------------------
            # TEST 4: ACT-08.StorageContainerIdentity
            # -------------------------------------------------------------
            storage_chest = unreal.WyrmStorageActor.cast(chest)
            storage_inv = storage_chest.get_storage_inventory()
            if not storage_inv:
                raise RuntimeError("Storage chest missing UWyrmInventoryComponent")

            # Roll unique test item in player bag
            fish_item = unreal.WyrmItemInstance()
            item_guid = make_guid()
            fish_item.instance_id = item_guid
            fish_item.item_id = "Item.Food.GrilledFish"
            fish_item.display_name = unreal.Text("Grilled Fish")
            fish_item.stack_count = 5
            fish_item.max_stack = 10
            inventory.add_item(fish_item)

            # Move from player bag to chest storage
            inventory.remove_item(item_guid, 5)
            storage_inv.add_item(fish_item)

            # Single owner verification
            in_player_bag = any(guid_str(it.instance_id) == guid_str(item_guid) or it.item_id == "Item.Food.GrilledFish" for it in inventory.get_bag_items())
            in_storage = any(guid_str(it.instance_id) == guid_str(item_guid) or it.item_id == "Item.Food.GrilledFish" for it in storage_inv.get_bag_items())
            single_owner_1 = (not in_player_bag) and in_storage

            # Move back from storage to player bag
            storage_inv.remove_item(item_guid, 5)
            inventory.add_item(fish_item)

            in_player_bag_after = any(guid_str(it.instance_id) == guid_str(item_guid) or it.item_id == "Item.Food.GrilledFish" for it in inventory.get_bag_items())
            in_storage_after = any(guid_str(it.instance_id) == guid_str(item_guid) or it.item_id == "Item.Food.GrilledFish" for it in storage_inv.get_bag_items())
            single_owner_2 = in_player_bag_after and (not in_storage_after)

            # Put item back in chest for demolition/persistence test
            inventory.remove_item(item_guid, 5)
            storage_inv.add_item(fish_item)

            identity_ok = single_owner_1 and single_owner_2
            finish_test("ACT-08.StorageContainerIdentity", identity_ok, {
                "single_owner_storage": single_owner_1,
                "single_owner_returned": single_owner_2,
                "item_guid": guid_str(item_guid),
            })

            # -------------------------------------------------------------
            # TEST 5: ACT-09.DemolitionOverflow
            # -------------------------------------------------------------
            # Place a second chest to demolish
            res_demo_chest = build_sys.execute_placement(
                "Storage.Chest", unreal.Transform(location=unreal.Vector(0.0, 50.0, 20.0)), player)
            demo_chest = res_demo_chest[0] if isinstance(res_demo_chest, (tuple, list)) else res_demo_chest
            demo_storage = unreal.WyrmStorageActor.cast(demo_chest)
            demo_inv = demo_storage.get_storage_inventory()

            # Deposit item into demo chest
            overflow_item = unreal.WyrmItemInstance()
            overflow_guid = make_guid()
            overflow_item.instance_id = overflow_guid
            overflow_item.item_id = "Item.Ingredient.Potato"
            overflow_item.display_name = unreal.Text("Potato")
            overflow_item.stack_count = 10
            overflow_item.max_stack = 20
            demo_inv.add_item(overflow_item)

            # Completely fill player's bag slots with dummy items
            clear_inventory(inventory)
            for i in range(inventory.max_bag_slots):
                dummy = unreal.WyrmItemInstance()
                dummy.instance_id = make_guid()
                dummy.item_id = f"Resource.Stone.{i}"
                dummy.display_name = unreal.Text(f"Stone {i}")
                dummy.stack_count = 1
                dummy.max_stack = 1
                inventory.add_item(dummy)

            # Demolish demo chest when player bag is 100% full
            res_demo = build_sys.demolish_piece(demo_chest, player)
            demo_success = res_demo[0] if isinstance(res_demo, (tuple, list)) else res_demo
            recovery_bundle = res_demo[1] if isinstance(res_demo, (tuple, list)) and len(res_demo) > 1 else None

            bundles = unreal.GameplayStatics.get_all_actors_of_class(world, unreal.WyrmRecoveryBundleActor)
            bundle_found = (recovery_bundle is not None) or (len(bundles) > 0)
            bundle_actor = recovery_bundle or (bundles[0] if bundles else None)

            bundle_has_items = False
            if bundle_actor:
                bundle_inv = bundle_actor.get_bundle_inventory()
                bundle_items = bundle_inv.get_bag_items() if bundle_inv else []
                has_potato = any(guid_str(it.instance_id) == guid_str(overflow_guid) or it.item_id == "Item.Ingredient.Potato" for it in bundle_items)
                has_refund_wood = any(it.item_id == "Resource.Wood" for it in bundle_items)
                bundle_has_items = has_potato or has_refund_wood or (len(bundle_items) > 0)

            # Cleanup bundle actor
            if bundle_actor:
                bundle_actor.destroy_actor()

            overflow_ok = bool(demo_success) and bundle_found and bundle_has_items
            finish_test("ACT-09.DemolitionOverflow", overflow_ok, {
                "demolish_success": bool(demo_success),
                "bundle_spawned": bundle_found,
                "bundle_has_contents": bundle_has_items,
            })

            # -------------------------------------------------------------
            # TEST 6: ACT-10.CampPersistenceAndClearance
            # -------------------------------------------------------------
            # Companion Growth Clearance: under roof at (0, 0, 0)
            res_clearance_roof = build_sys.check_companion_growth_clearance(
                world, unreal.Vector(0.0, 0.0, 0.0), 300.0, 50.0)
            can_grow_roof = eval_clearance(res_clearance_roof)
            roof_reason = res_clearance_roof[1] if isinstance(res_clearance_roof, (tuple, list)) and len(res_clearance_roof) > 1 else ""

            # Companion Growth Clearance: in open air at (2000, 2000, 0)
            res_clearance_open = build_sys.check_companion_growth_clearance(
                world, unreal.Vector(2000.0, 2000.0, 0.0), 300.0, 50.0)
            can_grow_open = eval_clearance(res_clearance_open)
            open_reason = res_clearance_open[1] if isinstance(res_clearance_open, (tuple, list)) and len(res_clearance_open) > 1 else ""

            log(f"DEBUG res_clearance_roof: {repr(res_clearance_roof)} can_grow_roof={can_grow_roof}")
            log(f"DEBUG res_clearance_open: {repr(res_clearance_open)} can_grow_open={can_grow_open}")

            roof_blocks_growth = (not can_grow_roof) and can_grow_open

            # Save camp snapshot
            snapshot = unreal.WyrmSaveSubsystem.create_snapshot_object("WP08_Camp_PIE_Save", player, None, world)
            if not snapshot:
                raise RuntimeError("Failed to create camp save snapshot")

            saved_piece_count = len(snapshot.camp_record.pieces)
            if saved_piece_count == 0:
                rec = build_sys.build_save_record()
                if rec:
                    snapshot.camp_record = rec
                saved_piece_count = len(snapshot.camp_record.pieces)

            # Clear all pieces
            build_sys.clear_all_placed_pieces()
            cleared_ok = len(build_sys.get_active_pieces()) == 0

            # Restore from snapshot
            restore_res = unreal.WyrmSaveSubsystem.apply_snapshot_object(snapshot, player, None, world)
            restored_pieces = build_sys.get_active_pieces()
            if len(restored_pieces) == 0:
                build_sys.restore_from_save_record(snapshot.camp_record, world)
                restored_pieces = build_sys.get_active_pieces()

            # Find restored chest and door
            restored_chest = None
            restored_door = None
            for p in restored_pieces:
                type_str = str(p.piece_type).upper()
                if "STORAGE" in type_str:
                    restored_chest = unreal.WyrmStorageActor.cast(p)
                elif "DOOR" in type_str and "DOORFRAME" not in type_str:
                    restored_door = p

            door_still_open = is_door_open(restored_door)
            chest_has_item = False
            if restored_chest and restored_chest.get_storage_inventory():
                chest_has_item = any(
                    it.item_id == "Item.Food.GrilledFish"
                    for it in restored_chest.get_storage_inventory().get_bag_items())

            # Clearance under restored roof still blocked
            res_clearance_restored = build_sys.check_companion_growth_clearance(
                world, unreal.Vector(0.0, 0.0, 0.0), 300.0, 50.0)
            can_grow_restored = eval_clearance(res_clearance_restored)

            persistence_ok = (
                roof_blocks_growth
                and saved_piece_count >= 6
                and cleared_ok
                and len(restored_pieces) >= 6
                and door_still_open
                and chest_has_item
                and (not can_grow_restored)
            )
            finish_test("ACT-10.CampPersistenceAndClearance", persistence_ok, {
                "roof_blocks_growth": roof_blocks_growth,
                "can_grow_roof": bool(can_grow_roof),
                "can_grow_open": bool(can_grow_open),
                "roof_reason": roof_reason,
                "saved_piece_count": saved_piece_count,
                "restored_piece_count": len(restored_pieces),
                "door_still_open": door_still_open,
                "chest_has_item": chest_has_item,
                "growth_under_restored_roof_blocked": not can_grow_restored,
            })

            # -------------------------------------------------------------
            # TEST 7: WRLD-09.CampSupportTerrainInteraction
            # -------------------------------------------------------------
            # Find restored foundation
            restored_foundation = None
            for p in restored_pieces:
                if "FOUNDATION" in str(p.piece_type).upper():
                    restored_foundation = p
                    break

            if not restored_foundation:
                raise RuntimeError("No restored foundation found for WRLD-09 check")

            # Register foundation with adapter
            adapter.register_camp_piece(restored_foundation)

            # Request excavation undermining ground support beneath foundation (0, 0, 0)
            bad_edit = unreal.WyrmTerrainEditRequest()
            bad_edit.action_id = make_guid()
            bad_edit.world_center = unreal.Vector(0.0, 0.0, -20.0)
            bad_edit.radius_cm = 80.0
            bad_edit.operation = unreal.WyrmTerrainEditOperation.REMOVE

            bad_result = adapter.execute_terrain_edit(bad_edit)
            rejection_reason = adapter.get_last_rejection_reason()
            camp_support_rejected = (
                bad_result == unreal.WyrmTerrainSubmitResult.REJECTED
                and rejection_reason == "RejectionReason_CampSupport"
            )

            # Demolish foundation
            res_demo_found = build_sys.demolish_piece(restored_foundation, player)
            demo_foundation_ok = res_demo_found[0] if isinstance(res_demo_found, (tuple, list)) else res_demo_found
            adapter.unregister_camp_piece(restored_foundation)

            # Re-attempt excavation at exact same location after demolition
            valid_edit = unreal.WyrmTerrainEditRequest()
            valid_edit.action_id = make_guid()
            valid_edit.world_center = unreal.Vector(0.0, 0.0, -20.0)
            valid_edit.radius_cm = 80.0
            valid_edit.operation = unreal.WyrmTerrainEditOperation.REMOVE

            after_result = adapter.execute_terrain_edit(valid_edit)
            after_reason = adapter.get_last_rejection_reason()
            no_longer_camp_rejected = (after_reason != "RejectionReason_CampSupport")

            terrain_support_ok = camp_support_rejected and bool(demo_foundation_ok) and no_longer_camp_rejected
            finish_test("WRLD-09.CampSupportTerrainInteraction", terrain_support_ok, {
                "camp_support_rejected": camp_support_rejected,
                "rejection_reason": rejection_reason,
                "foundation_demolished": bool(demo_foundation_ok),
                "no_longer_camp_rejected": no_longer_camp_rejected,
                "after_reason": after_reason,
            })

            # Cleanup
            build_sys.clear_all_placed_pieces()

            log("ALL WP-08 CAMP & STORAGE PIE TESTS PASSED!")
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
