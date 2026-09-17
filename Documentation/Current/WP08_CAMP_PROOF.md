# WP-08: Supported Camp and Storage Verification Proof

**Date**: 2026-09-17  
**Engine**: Unreal Engine 5.8.2 (`C:\Program Files\UE_5.8`)  
**Status**: **PASS in Real PIE & Native Automation**  
**Evidence Receipts**:
- `Saved/Diagnostics/WP08_camp_proof.json` (7/7 PIE scenarios PASS)
- `Saved/Automation/Scaffold/index.json` (32/32 native automation tests PASS)

---

## 1. Scope & Authority Boundaries

In strict compliance with [AGENTS.md](../../AGENTS.md) and single-owner system boundaries:
1. **Building Authority (`UWyrmBuildingSubsystem`)**: Authoritative `UGameInstanceSubsystem` managing camp piece definitions, placement validation, material deductions, piece spawning, demolition refunds, companion growth clearances, and camp persistence serialization.
2. **Inventory/Storage Authority (`UWyrmInventoryComponent`)**: Remains the sole inventory and storage authority. `AWyrmStorageActor` wraps a `UWyrmInventoryComponent` instance (`StorageInventory`). Moving items between player bag and storage containers preflights capacity and preserves item GUIDs, rolls, and instance identity.
3. **Terrain Authority (`AWyrmGeoForgeAdapter`)**: Project terrain adapter enforces ground support footprint locking for placed foundation pieces (`WRLD-09`). Excavations directly undermining foundation support bounds are rejected with `RejectionReason_CampSupport` until the piece is demolished.
4. **Save Authority (`UWyrmSaveSubsystem`)**: Coordinates camp state persistence via `FWyrmCampSaveRecord` embedded in `UWyrmSaveGame` (Schema Version 4), ensuring pieces, doors, and storage container inventories restore deterministically across sessions.

---

## 2. Implemented Components

### 2.1 Genuine Camp Art Assets (`Content/WYRMFALL/Environment/`)
Imported and verified authentic meshes:
- `/Game/WYRMFALL/Environment/Camp/SM_StorageChest`
- `/Game/WYRMFALL/Environment/Building/Wood/SM_Stylized_Wood_Foundation`
- `/Game/WYRMFALL/Environment/Building/Wood/SM_Stylized_Wood_Wall`
- `/Game/WYRMFALL/Environment/Building/Wood/SM_Stylized_Wood_Doorframe`
- `/Game/WYRMFALL/Environment/Building/Wood/SM_Stylized_Wood_Door`
- `/Game/WYRMFALL/Environment/Building/Wood/SM_Stylized_Wood_Roof`
- `/Game/WYRMFALL/Environment/Building/Wood/SM_Stylized_Wood_Ceiling`

### 2.2 Core C++ Classes (`Source/WYRMFALL/`)
- `AWyrmBuildingPiece`: Base actor for all camp pieces with bounds calculation, door toggle state, material refund evaluation, and ground support column bounds calculation.
- `AWyrmStorageActor`: Extends `AWyrmBuildingPiece` with an authoritative `UWyrmInventoryComponent` for player stash/chest storage.
- `AWyrmRecoveryBundleActor`: Authoritative overflow container actor spawned on demolition when player bag has insufficient capacity, holding refunded materials and stored items without data loss or duplication.
- `UWyrmBuildingSubsystem`: Central building authority handling:
  - `ValidatePlacement`: Dry-run validation testing ground support, structural support, pawn occupancy, and overlap with zero side effects.
  - `ExecutePlacement`: Atomically deducts inventory costs, spawns piece, and registers support footprint with GeoForge adapters.
  - `DemolishPiece`: Refunds materials, empties storage into player bag, spawns `AWyrmRecoveryBundleActor` for overflow, unregisters support from GeoForge adapters, and destroys piece.
  - `CheckCompanionGrowthClearance`: Verifies vertical clearance for dragons/companions. Physical roofs and ceilings overhead block companion growth.
  - `BuildSaveRecord` & `RestoreFromSaveRecord`: Serializes and deserializes all placed pieces, door states, and container contents.

---

## 3. Verification Scenarios & Results

All 7 scenarios executed in real PIE via `py -3.12 tools/run_wp08_pie_proof.py` (and mirrored in native automation test `WyrmScaffoldTests.cpp`):

| Test Identifier | Acceptance Requirement | Result | Verified Details |
|---|---|---|---|
| **ASSET.CampAssets** | Real genuine assets loaded | **PASS** | 7 genuine static meshes verified in package registry |
| **ACT-06.UsefulCampPlacement** | Place useful camp: foundation, walls, doorframe, door, roof, storage, bench | **PASS** | 7 pieces placed atomically, door toggled open, storage chest active |
| **ACT-07_WRLD-08.PlacementRejection** | Rejection validation with zero side effects | **PASS** | Floating piece rejected (`Unsupported`), overlapping rejected (`Overlapping`), piece on pawn rejected (`Occupied`), no materials rejected (`InsufficientMaterials`). Active piece count unchanged (zero dangling actors) |
| **ACT-08.StorageContainerIdentity** | Bag $\leftrightarrow$ Chest item transfers with single owner | **PASS** | Stack transferred from bag to chest and back. Item GUID `191EA048...` and properties preserved across transfer |
| **ACT-09.DemolitionOverflow** | Demolition with full bag creates recovery bundle | **PASS** | Demolished chest with bag 100% full; `AWyrmRecoveryBundleActor` spawned holding stored potato and refunded wood; zero item loss |
| **ACT-10.CampPersistenceAndClearance** | Camp save/restore & companion growth clearance | **PASS** | 7 pieces saved to snapshot, world cleared, restored from snapshot. Restored door remained open, restored chest held item. Roof overhead blocked companion growth; open air permitted growth |
| **WRLD-09.CampSupportTerrainInteraction** | Ground support locking beneath foundation | **PASS** | Excavation directly beneath foundation rejected with `RejectionReason_CampSupport`. Foundation demolished, excavation re-attempted and permitted |

---

## 4. Automation & Reproduction Commands

```powershell
# Compile editor target
& "C:/Program Files/UE_5.8/Engine/Build/BatchFiles/Build.bat" WYRMFALLEditor Win64 Development "-Project=G:/assets/voxel project/WYRMFALL.uproject" -WaitMutex -NoHotReloadFromIDE -NoEngineChanges

# Run full native automation test suite (32/32 tests)
py -3.12 tools/wyrm.py ue-test

# Run full WP-08 Play-In-Editor proof
py -3.12 tools/run_wp08_pie_proof.py
```
