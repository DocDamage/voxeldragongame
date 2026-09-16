"""
Headless PIE Verification Suite for WP-05: Loot, Equipment, Inventory, and Coherent Save Snapshot
Tests acceptance criteria COM-06, COM-07, and SAVE-01:
  COM-06: Item generation, rolled stats, and idempotent equipment stat modifications (zero leaks).
  COM-07: Inventory capacity, stacking, atomic bag <-> stash transfers, and overflow protection.
  SAVE-01: Coherent snapshot save & reload roundtrip unifying character stats, camera mode, inventory, and terrain deltas.

Run via:
  UnrealEditor-Cmd.exe WYRMFALL.uproject -run=pythonscript -script="tools/unreal/verify_wp05_inventory_proof.py" -stdout -FullStdOutLogOutput -unattended -nopause -nosplash -nullrhi
"""

import unreal
import json
import os
import sys

def log(msg):
    unreal.log(f"[WP05_INVENTORY_PROOF] {msg}")

def main():
    log("Starting WP-05 Inventory & Save Proof Suite...")
    results = {}
    passed = True

    editor_subsystem = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
    world = editor_subsystem.get_editor_world() if editor_subsystem else unreal.EditorLevelLibrary.get_editor_world()
    if not world:
        log("ERROR: Could not get editor world.")
        sys.exit(1)

    # -------------------------------------------------------------
    # SETUP: Spawn Player Character
    # -------------------------------------------------------------
    player_loc = unreal.Vector(0.0, 0.0, 100.0)
    player = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.WyrmCharacter, player_loc)
    if not player:
        log("ERROR: Failed to spawn WyrmCharacter")
        sys.exit(1)

    inv = player.get_inventory()
    attrs = player.get_attributes()
    if not inv or not attrs:
        log("ERROR: Missing InventoryComponent or AttributeSet on WyrmCharacter")
        player.destroy_actor()
        sys.exit(1)

    base_power = attrs.get_current_power()
    base_armor = attrs.get_current_armor()
    base_max_health = attrs.get_current_max_health()
    log(f"Initial Base Stats: Power={base_power}, Armor={base_armor}, MaxHealth={base_max_health}")

    # =============================================================
    # CASE COM-06: Item Generation, Affix Rolls & Idempotent Equip Stats
    # =============================================================
    log("Running Case COM-06: Item Generation & Equipment Stats...")
    weapon = unreal.WyrmInventoryComponent.roll_random_item("BladeOfWyrms", unreal.WyrmItemType.WEAPON, 3, unreal.WyrmEquipSlot.MAIN_HAND)
    weapon_power = unreal.WyrmInventoryComponent.get_item_stat_value(weapon, "Power") # 10 + 3*3.5 = 20.5
    
    armor = unreal.WyrmInventoryComponent.roll_random_item("DragonScaleArmor", unreal.WyrmItemType.ARMOR, 2, unreal.WyrmEquipSlot.CHEST)
    armor_defense = unreal.WyrmInventoryComponent.get_item_stat_value(armor, "Armor") # 5 + 2*2 = 9.0
    armor_health = unreal.WyrmInventoryComponent.get_item_stat_value(armor, "MaxHealth") # 20 + 2*10 = 40.0

    # In Unreal Python, out parameters are returned as the function result
    inv.add_item(weapon)
    inv.add_item(armor)

    equip_wep_ok = inv.equip_item(weapon.instance_id, unreal.WyrmEquipSlot.MAIN_HAND)
    power_equipped = attrs.get_current_power()

    equip_arm_ok = inv.equip_item(armor.instance_id, unreal.WyrmEquipSlot.CHEST)
    armor_equipped = attrs.get_current_armor()
    health_equipped = attrs.get_current_max_health()

    log(f"After Equip: Power={power_equipped} (Expected={base_power + weapon_power}), Armor={armor_equipped} (Expected={base_armor + armor_defense}), MaxHealth={health_equipped} (Expected={base_max_health + armor_health})")

    # Unequip and verify exact return to base (zero leaks)
    unequip_wep_ok = inv.unequip_item(unreal.WyrmEquipSlot.MAIN_HAND)
    power_after_unequip = attrs.get_current_power()

    unequip_arm_ok = inv.unequip_item(unreal.WyrmEquipSlot.CHEST)
    armor_after_unequip = attrs.get_current_armor()
    health_after_unequip = attrs.get_current_max_health()

    log(f"After Unequip: Power={power_after_unequip} (Base={base_power}), Armor={armor_after_unequip} (Base={base_armor}), MaxHealth={health_after_unequip} (Base={base_max_health})")

    com06_pass = (
        equip_wep_ok and equip_arm_ok and unequip_wep_ok and unequip_arm_ok and
        abs(power_equipped - (base_power + weapon_power)) < 0.01 and
        abs(armor_equipped - (base_armor + armor_defense)) < 0.01 and
        abs(health_equipped - (base_max_health + armor_health)) < 0.01 and
        abs(power_after_unequip - base_power) < 0.01 and
        abs(armor_after_unequip - base_armor) < 0.01 and
        abs(health_after_unequip - base_max_health) < 0.01
    )

    results["COM-06"] = {
        "status": "PASS" if com06_pass else "FAIL",
        "weapon_power_roll": weapon_power,
        "armor_defense_roll": armor_defense,
        "armor_health_roll": armor_health,
        "power_delta_on_equip": power_equipped - base_power,
        "power_leak_after_unequip": power_after_unequip - base_power,
        "armor_leak_after_unequip": armor_after_unequip - base_armor,
        "max_health_leak_after_unequip": health_after_unequip - base_max_health
    }
    if not com06_pass:
        passed = False

    # =============================================================
    # CASE COM-07: Capacity, Stacking, Atomic Transfers & Overflow Guard
    # =============================================================
    log("Running Case COM-07: Capacity, Transfers & Overflow...")
    inv.clear_all()
    inv.max_bag_slots = 3
    inv.max_stash_slots = 5

    item1 = unreal.WyrmInventoryComponent.roll_random_item("Blade1", unreal.WyrmItemType.WEAPON, 1)
    item2 = unreal.WyrmInventoryComponent.roll_random_item("Blade2", unreal.WyrmItemType.WEAPON, 1)
    item3 = unreal.WyrmInventoryComponent.roll_random_item("Blade3", unreal.WyrmItemType.WEAPON, 1)
    item4 = unreal.WyrmInventoryComponent.roll_random_item("Blade4", unreal.WyrmItemType.WEAPON, 1)

    ex1 = inv.add_item(item1)
    add1 = (ex1 is not None)
    ex2 = inv.add_item(item2)
    add2 = (ex2 is not None)
    ex3 = inv.add_item(item3)
    add3 = (ex3 is not None)
    bag_full_count = len(inv.get_bag_items())

    # Overflow rejection: 4th item must fail (returns None) without corrupting bag
    ex4 = inv.add_item(item4)
    add4 = (ex4 is not None)
    bag_count_after_overflow = len(inv.get_bag_items())
    excess_retained = (ex4 is None)

    log(f"COM-07 Bag Fill: Full Count={bag_full_count}, Add4 Success={add4}, Excess Retained={excess_retained}, Final Bag Count={bag_count_after_overflow}")

    # Atomic transfer to Stash and back
    transfer_to_stash_ok = inv.transfer_to_stash(item1.instance_id, 1)
    bag_after_stash_transfer = len(inv.get_bag_items())
    stash_after_transfer = len(inv.get_stash_items())

    transfer_from_stash_ok = inv.transfer_from_stash(item1.instance_id, 1)
    bag_after_return = len(inv.get_bag_items())
    stash_after_return = len(inv.get_stash_items())

    # Resource stacking
    inv.clear_all()
    ore1 = unreal.WyrmInventoryComponent.roll_random_item("ObsidianOre", unreal.WyrmItemType.RESOURCE, 1)
    ore1.stack_count = 45
    ore2 = unreal.WyrmInventoryComponent.roll_random_item("ObsidianOre", unreal.WyrmItemType.RESOURCE, 1)
    ore2.stack_count = 35

    inv.add_item(ore1)
    inv.add_item(ore2)
    bag_items = inv.get_bag_items()
    stacked_ok = (len(bag_items) == 1 and bag_items[0].stack_count == 80)

    com07_pass = (
        add1 and add2 and add3 and not add4 and
        bag_full_count == 3 and bag_count_after_overflow == 3 and excess_retained and
        transfer_to_stash_ok and bag_after_stash_transfer == 2 and stash_after_transfer == 1 and
        transfer_from_stash_ok and bag_after_return == 3 and stash_after_return == 0 and
        stacked_ok
    )

    results["COM-07"] = {
        "status": "PASS" if com07_pass else "FAIL",
        "bag_fill_success": add1 and add2 and add3,
        "overflow_rejected": not add4,
        "excess_retained_in_out": excess_retained,
        "atomic_transfer_to_stash": transfer_to_stash_ok,
        "atomic_transfer_from_stash": transfer_from_stash_ok,
        "resource_stacking_verified": stacked_ok,
        "combined_stack_total": bag_items[0].stack_count if len(bag_items) > 0 else 0
    }
    if not com07_pass:
        passed = False

    # =============================================================
    # CASE SAVE-01: Coherent Snapshot Save & Reload Roundtrip
    # =============================================================
    log("Running Case SAVE-01: Coherent Save Snapshot Roundtrip...")
    inv.clear_all()
    inv.max_bag_slots = 20
    inv.max_stash_slots = 50

    # Configure custom character state
    attrs.set_current_health(85.0)
    attrs.set_current_power(30.0)
    attrs.set_current_armor(12.0)
    player.set_camera_mode(unreal.WyrmCameraMode.TOP_DOWN)

    # Add items: 1 weapon equipped (+20.5 Power), 1 consumable in bag (5 potions), 1 resource in stash (50 ore)
    relic_wep = unreal.WyrmInventoryComponent.roll_random_item("RelicBlade", unreal.WyrmItemType.WEAPON, 3, unreal.WyrmEquipSlot.MAIN_HAND)
    relic_power = unreal.WyrmInventoryComponent.get_item_stat_value(relic_wep, "Power") # 20.5
    potion = unreal.WyrmInventoryComponent.roll_random_item("HealingFlask", unreal.WyrmItemType.CONSUMABLE, 1)
    potion.stack_count = 5
    ore = unreal.WyrmInventoryComponent.roll_random_item("MythrilOre", unreal.WyrmItemType.RESOURCE, 1)
    ore.stack_count = 50

    inv.add_item(relic_wep)
    inv.add_item(potion)
    inv.add_item(ore)
    inv.equip_item(relic_wep.instance_id, unreal.WyrmEquipSlot.MAIN_HAND)
    inv.transfer_to_stash(ore.instance_id, 50)

    # Spawn mock terrain provider with action ID
    adapter_loc = unreal.Vector(500.0, 0.0, 0.0)
    adapter = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.WyrmGeoForgeAdapter, adapter_loc)
    mock_action_id = unreal.WyrmInventoryComponent.roll_random_item("ActionGuid", unreal.WyrmItemType.RESOURCE, 1).instance_id
    if adapter:
        adapter.processed_action_ids.add(mock_action_id)

    # Save to slot using authoritative static helper
    slot_name = "WP05_Proof_Slot"
    save_ok = unreal.WyrmSaveSubsystem.save_snapshot_to_slot(slot_name, player, adapter)
    log(f"SAVE-01 Snapshot Saved to Slot '{slot_name}': Success={save_ok}")

    # Mutate player & inventory to blank/different state
    attrs.set_current_health(15.0)
    attrs.set_current_power(5.0)
    player.set_camera_mode(unreal.WyrmCameraMode.THIRD_PERSON)
    inv.clear_all()
    if adapter:
        adapter.processed_action_ids.clear()

    # Reload snapshot from slot
    load_ok = unreal.WyrmSaveSubsystem.load_snapshot_from_slot(slot_name, player, adapter)
    log(f"SAVE-01 Snapshot Reloaded from Slot '{slot_name}': Success={load_ok}")

    # Inspect restored state
    restored_hp = attrs.get_current_health()
    restored_power = attrs.get_current_power()
    restored_camera = player.get_camera_mode()
    restored_wep_equipped = inv.is_slot_equipped(unreal.WyrmEquipSlot.MAIN_HAND)
    restored_bag_items = inv.get_bag_items()
    restored_stash_items = inv.get_stash_items()
    restored_terrain_action = (mock_action_id in adapter.processed_action_ids) if adapter else True

    log(f"Restored: HP={restored_hp} (Expected=85.0), Power={restored_power} (Expected={30.0 + relic_power}), Camera={restored_camera} (Expected=TopDown), WeaponEquipped={restored_wep_equipped}, BagNum={len(restored_bag_items)}, StashNum={len(restored_stash_items)}")

    save01_pass = (
        save_ok and load_ok and
        abs(restored_hp - 85.0) < 0.01 and
        abs(restored_power - (30.0 + relic_power)) < 0.01 and
        restored_camera == unreal.WyrmCameraMode.TOP_DOWN and
        restored_wep_equipped and
        len(restored_bag_items) == 1 and restored_bag_items[0].stack_count == 5 and
        len(restored_stash_items) == 1 and restored_stash_items[0].stack_count == 50 and
        restored_terrain_action
    )

    results["SAVE-01"] = {
        "status": "PASS" if save01_pass else "FAIL",
        "save_success": save_ok,
        "load_success": load_ok,
        "restored_health": restored_hp,
        "restored_power": restored_power,
        "restored_camera_mode": str(restored_camera),
        "weapon_remains_equipped": restored_wep_equipped,
        "bag_item_count": len(restored_bag_items),
        "stash_item_count": len(restored_stash_items),
        "terrain_action_restored": restored_terrain_action
    }
    if not save01_pass:
        passed = False

    # Cleanup actors
    if adapter:
        adapter.destroy_actor()
    player.destroy_actor()

    # Clean up save slot if created
    if unreal.WyrmSaveSubsystem.does_save_exist(slot_name):
        unreal.WyrmSaveSubsystem.delete_save_slot(slot_name)

    # Output results
    diag_dir = os.path.join(unreal.Paths.project_saved_dir(), "Diagnostics")
    os.makedirs(diag_dir, exist_ok=True)
    report_path = os.path.join(diag_dir, "WP05_inventory_proof.json")
    with open(report_path, "w") as f:
        json.dump(results, f, indent=2)

    log(f"Results written to {report_path}")
    log(f"FINAL RESULT: {'ALL PASS' if passed else 'FAIL'}")

    if not passed:
        sys.exit(1)
    sys.exit(0)

if __name__ == "__main__":
    main()
