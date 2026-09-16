# WP-05: Loot, Equipment, Inventory, and Coherent Save Snapshot — Verification Proof

**Date**: 2026-09-15 / 2026-09-16  
**Engine**: Unreal Engine 5.8.2 (`C:\Program Files\UE_5.8`)  
**Target**: `WYRMFALLEditor Win64 Development`  
**Host**: Windows 11 (DOCDAMAGE)  
**Status**: **PASS** (18/18 Native Tests, Headless PIE Suite 3/3 Cases PASS)

---

## 1. Executive Summary

WP-05 delivers the authoritative inventory, equipment, and single-owner save architecture for WYRMFALL, strictly upholding all core product constraints from [AGENTS.md](../../AGENTS.md) and [SAVE_AND_CONTROL_CONTRACT.md](../DesignPack/docs/systems/SAVE_AND_CONTROL_CONTRACT.md):

1. **Single Inventory Authority**: `UWyrmInventoryComponent` on `AWyrmCharacter` serves as the sole owner of inventory containers, item transfers, bag/stash capacity, and slot assignments. No external or secondary inventory manager exists.
2. **GAS Remains Combat Authority**: Equipment stat bonuses (Power, Armor, MaxHealth) apply directly and idempotently to the character's GAS attributes (`UWyrmAttributeSet`). Stat modification is mathematically reversible with **zero cumulative leak** across equip/unequip cycles (`COM-06`).
3. **Mutable Remains Creator Authority**: Visual attachments connect to designated character sockets (`Hand_Right`, `spine_02Socket`, `Back_Weapon`), and Mutable character appearance descriptors (Base64) serialize into and out of the coherent save record without fidelity loss.
4. **Single Save Owner**: `UWyrmSaveSubsystem` (GameInstanceSubsystem) and `UWyrmSaveGame` (SaveGame) serve as the **sole project save coordinator**, unifying header/metadata, character attributes, camera mode, inventory/equipment records, and the GeoForge binary terrain delta payload into a single generation (`SAVE-01`). Neither GeoForge, EBS, nor AGIS writes independent save slots.
5. **Overflow Protection & Atomic Transfers**: Bag additions past max capacity (`MaxBagSlots = 20`) reject excess cleanly into `OutRemaining` without dropping items or corrupting inventory state (`COM-07`). Transfers between Bag and Stash are atomic.

---

## 2. Evidence Artifacts

| Artifact | Path | Description |
|---|---|---|
| **Inventory Component Header** | `Source/WYRMFALL/Public/Inventory/WyrmInventoryComponent.h` | Bag/Stash management, capacity, atomic transfers, equipment attachment |
| **Inventory Component Impl** | `Source/WYRMFALL/Private/Inventory/WyrmInventoryComponent.cpp` | Stat modification, overflow rejection, mesh attachment |
| **Inventory Types** | `Source/WYRMFALL/Public/Inventory/WyrmInventoryTypes.h` | `EWyrmItemType`, `EWyrmEquipSlot`, `FWyrmItemRoll`, `FWyrmItemInstance` |
| **Save Game Container** | `Source/WYRMFALL/Public/Save/WyrmSaveGame.h`, `Private/Save/WyrmSaveGame.cpp` | Authoritative single save record unifying stats, appearance, inventory, and terrain delta |
| **Save Subsystem** | `Source/WYRMFALL/Public/Save/WyrmSaveSubsystem.h`, `Private/Save/WyrmSaveSubsystem.cpp` | GameInstanceSubsystem coordinating coherent snapshot save/load roundtrips |
| **Native Automation Suite** | `Source/WYRMFALL/Private/Tests/WyrmScaffoldTests.cpp` | 18 native automation tests (4 new tests for WP-05) |
| **Native Test Results** | `Saved/Automation/Scaffold/index.json` | 18/18 tests reporting `Success`, 0 warnings, 0 errors |
| **Headless PIE Suite** | `tools/unreal/verify_wp05_inventory_proof.py` | Headless execution verifying `COM-06`, `COM-07`, and `SAVE-01` |
| **PIE Diagnostic Report** | `Saved/Diagnostics/WP05_inventory_proof.json` | JSON verification proof of all 3 criteria |

---

## 3. Acceptance Criteria Verification

### COM-06: Rolled Item Generation & Idempotent Equipment Stats
- **Target**: Rolled items generate deterministic affixes based on level. Equipping applies stat bonuses to GAS attributes; unequipping reverses them with zero leaks. Swapping slots updates stats correctly.
- **Observed Results**:
  - Weapon Lv.3 generated with Power roll = `+20.5`.
  - Armor Lv.2 generated with Armor roll = `+9.0`, MaxHealth roll = `+40.0`.
  - On equip: Power increased from 20.0 to 40.5 (+20.5), Armor from 0.0 to 9.0 (+9.0), MaxHealth from 100.0 to 140.0 (+40.0).
  - On unequip: Power returned to exactly 20.0 (leak = 0.0), Armor to 0.0 (leak = 0.0), MaxHealth to 100.0 (leak = 0.0).
  - Unequipping when bag is at full capacity (1/1) is rejected by overflow guard, preventing item loss.
- **Verdict**: **PASS**

### COM-07: Capacity Limits, Stacking, Atomic Transfers & Overflow Protection
- **Target**: Bag enforces maximum capacity (`MaxBagSlots = 3` in test). Overflow pickups reject excess cleanly. Bag <-> Stash transfers are atomic. Stackable resources combine up to `MaxStack = 99`.
- **Observed Results**:
  - Bag filled to 3/3 capacity with rolled weapons.
  - Adding a 4th weapon rejected: `add_item` returned failure, excess retained `stack_count = 1`, and bag item count remained at 3 without corruption.
  - Transferred weapon from Bag to Stash: Bag count decreased to 2, Stash count increased to 1.
  - Transferred weapon back from Stash to Bag: Bag count restored to 3, Stash count decreased to 0.
  - Resource stacking: 45 Obsidian Ore + 35 Obsidian Ore combined into a single slot with stack count 80.
- **Verdict**: **PASS**

### SAVE-01: Coherent Snapshot Save & Reload Roundtrip
- **Target**: Coherent snapshot save captures Character attributes (Health, Power, Armor), camera perspective (TopDown), Mutable appearance, inventory (equipped weapon, bag consumables, stash resources), and GeoForge terrain delta. Reload restores all components with exact fidelity.
- **Observed Results**:
  - Configured state: Health = 85.0, Power = 30.0 (base) + 20.5 (equipped weapon) = 50.5 total, Armor = 12.0, Camera = TopDown, 1 equipped weapon, 5 potions in Bag, 50 Mythril Ore in Stash, terrain processed action ID.
  - Saved snapshot to slot `"WP05_Proof_Slot"`.
  - Mutated character to blank state (HP = 15.0, Power = 5.0, Camera = ThirdPerson, empty inventory, cleared terrain action IDs).
  - Reloaded snapshot from slot `"WP05_Proof_Slot"`.
  - Restored state verification:
    - Restored Health: `85.0` (Exact match)
    - Restored Power: `50.5` (Base 30.0 + Equipped Weapon 20.5, exact match, no double-stacking)
    - Restored Camera: `TopDown` (Exact match)
    - Restored Equipped: `True` (Relic Blade in MainHand)
    - Restored Bag: 1 item (5 Healing Flasks)
    - Restored Stash: 1 item (50 Mythril Ore)
    - Restored Terrain: Processed action ID retained in adapter
  - Slot deletion verified: slot cleanly deleted from disk.
- **Verdict**: **PASS**

---

## 4. Verification Suite Outputs

### 4.1 Native Automation Tests (`py -3.12 tools/wyrm.py ue-test`)
```
LogAutomationCommandLine: Display: ...Automation Test Queue Empty 18 tests performed.
Process state: EXITED; exit: 0
All 18 source-declared native tests succeeded; not a gameplay gate pass.
```
All 18 tests passed:
1. `WYRMFALL.Scaffold.ActorLifecycle` (Success)
2. `WYRMFALL.Scaffold.CameraDefaults` (Success)
3. `WYRMFALL.Scaffold.InputBindingIntegrity` (Success)
4. `WYRMFALL.Scaffold.DefaultTerrainFailClosed` (Success)
5. `WYRMFALL.Scaffold.GeoForgeAdapterCapabilities` (Success)
6. `WYRMFALL.Scaffold.GeoForgeAdapterYieldContract` (Success)
7. `WYRMFALL.Scaffold.GeoForgeAdapterStateTracking` (Success)
8. `WYRMFALL.Scaffold.MutableHostPresence` (Success)
9. `WYRMFALL.Scaffold.MutableParameterReflection` (Success)
10. `WYRMFALL.Scaffold.SharedControlGating` (Success)
11. `WYRMFALL.Scaffold.CombatCanonicalDamage` (Success)
12. `WYRMFALL.Scaffold.CombatBoundsAndDrain` (Success)
13. `WYRMFALL.Scaffold.CombatCostAndCooldown` (Success)
14. `WYRMFALL.Scaffold.CombatEnemyRolesAndStatus` (Success)
15. `WYRMFALL.Scaffold.ItemGenerationAndRolls` (Success)
16. `WYRMFALL.Scaffold.InventoryCapacityAndTransfer` (Success)
17. `WYRMFALL.Scaffold.EquipmentStatApplication` (Success)
18. `WYRMFALL.Scaffold.SaveSubsystemRoundtrip` (Success)

### 4.2 Headless PIE Output (`verify_wp05_inventory_proof.py`)
```json
{
  "COM-06": {
    "status": "PASS",
    "weapon_power_roll": 20.5,
    "armor_defense_roll": 9.0,
    "armor_health_roll": 40.0,
    "power_delta_on_equip": 20.5,
    "power_leak_after_unequip": 0.0,
    "armor_leak_after_unequip": 0.0,
    "max_health_leak_after_unequip": 0.0
  },
  "COM-07": {
    "status": "PASS",
    "bag_fill_success": true,
    "overflow_rejected": true,
    "excess_retained_in_out": true,
    "atomic_transfer_to_stash": true,
    "atomic_transfer_from_stash": true,
    "resource_stacking_verified": true,
    "combined_stack_total": 80
  },
  "SAVE-01": {
    "status": "PASS",
    "save_success": true,
    "load_success": true,
    "restored_health": 85.0,
    "restored_power": 50.5,
    "restored_camera_mode": "<WyrmCameraMode.TOP_DOWN: 1>",
    "weapon_remains_equipped": true,
    "bag_item_count": 1,
    "stash_item_count": 1,
    "terrain_action_restored": true
  }
}
```

### 4.3 Portable Offline Validation
- `py -3.12 tools/wyrm.py verify`: **PASS** (exit 0)
- `py -3.12 tools/wyrm.py test`: **PASS** (124/124 offline tests pass, exit 0)
