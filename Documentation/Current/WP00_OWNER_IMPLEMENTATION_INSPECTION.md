# WP-00 owner implementation inspection — September 15, 2026

**PARTIAL.** Native read-only Blueprint exports now establish actual EBS resource,
interaction, building and save behavior. This does not select a WYRMFALL owner or
pass gameplay/save acceptance. The preceding diagnostic bridge was committed and
pushed as `cc8ce1a8617cac2824add214bf20241873ec917f` before this continuation.

## Method and local evidence

The C: UE 5.8.2 editor loaded the supplied EBS project and used its native
`ObjectExporterT3D` to export eight selected Blueprints, including graph nodes,
execution/data pin links, defaults and generated graphs. Inspection followed the
authored graph connections, not just filenames or function-name searches.
`UEdGraphNode::ExportCustomProperties` in the installed engine exports the pins.
No vendor graph text is tracked; exports remain under excluded Saved diagnostics.

- Project: `assets and old docs/EasyBuildingSystemV10/EasyBuildingSystemv10.uproject`.
- Export report and source/export hashes: `Saved/Diagnostics/EBSOwnerInspection/report.json`.
- All eight loads/exports succeeded and the inspected package hashes stayed unchanged.
- Final process: `Saved/ScaffoldLogs/20260915T181704Z_8249955857f5_ebs-owner-final.log` and `.json`; exit 0, 20.453 seconds.
- Source project warning: obsolete iOS config value `IOS_9` fails config import;
  also a connectivity-check timeout. These do not invalidate the individual
  Windows graph exports, but this is not a clean project build or compatibility pass.
- An earlier command returned exit 0 despite failing to find the script because
  backslashes were interpreted in the Python filename. That attempt is **FAIL**:
  `20260915T181318Z_8b043a3f3528_ebs-owner-graphs.log`. Forward-slash paths fixed it.
  The export report and actual files, not process exit alone, establish success.

Reproduce from PowerShell (use forward slashes in the script argument):

```powershell
& 'C:/Program Files/UE_5.8/Engine/Binaries/Win64/UnrealEditor-Cmd.exe' `
  'G:/assets/voxel project/assets and old docs/EasyBuildingSystemV10/EasyBuildingSystemv10.uproject' `
  /Engine/Maps/Entry -EnablePlugins=PythonScriptPlugin -unattended -nop4 -nosplash -nosound -nullrhi `
  '-ExecutePythonScript=G:/assets/voxel project/tools/unreal/inspect_wp00_owner_graphs.py' `
  -stdout -FullStdOutLogOutput
```

## EBS ownership and concrete constraints

All assets below are rooted at `/Game/EasyBuildingSystem/Blueprints/` in the
supplied EBS project. Graph names refer to the corresponding `.t3d` exports.

| Asset / authored graph | Inspected behavior | WYRMFALL consequence |
| --- | --- | --- |
| `Components/BP_EBS_ResourcesComponent`, `AddResource` | Searches `Resources` by resource enum; adds the requested value to a matching entry or appends the supplied struct. No capacity/overflow check in this graph. | It is a resource balance owner, not evidence of the required full-bag inventory contract. Do not run it alongside a second independent resource inventory. |
| Same, `RemoveResource` / `RemoveResources` | Matching entries are written using subtraction through a Clamp node; the subtraction feeds both Value and Max, with Min 0. Missing types return false. The batch calls RemoveResource in sequence, returns false on failure, and has no rollback of earlier writes. | Do not treat this as an atomic multi-resource spend or a terrain reward transaction. Insufficient/negative/nonfinite amounts still need runtime boundary tests. |
| Same, `InitComponent` / server events | Initializes `Resources` from `StartingResources`; server events route into the same mutators. | Preserve one source of balances; a demo initialization must not reset loaded inventory. |
| `Game/BP_EBS_PlayerController`, `CheckRequirements_BPI` and collapsed `Building events` | Requirement checks use the resource component. Completion obtains the data-table row then calls `RemoveResources`; the returned success pin is not consumed there. | A future adapter must check and commit against the selected inventory owner. Completion cannot silently imply a successful spend. |
| `Components/BP_EBS_BuildingComponent`, `TryStartBuildObject`, `CheckBuildingRequirements`, `CompleteBuildingRequirements`, `FinishBuild` | Routes requirements through player interfaces and handles build status, support, attachments and completion. | Candidate building functionality has a useful existing seam; no duplicate building framework is needed. Placement/resource transaction behavior still needs testing. |
| `Components/BP_EBS_InteractionComponent`, `UpdateInteractionActor`, `TryStartInteractonWithActor`, `CompleteInteractionNotify` | Updates a traced/range-limited target; uses character rotation/montage and sends completion through the interaction interface. | This is an interaction implementation to reconcile with the project host, not an inert helper. No second active interaction owner. |
| `Game/BP_EBS_SaveGame`, `SaveGame_BPI` / `SaveGameToSlot_BPI` | Writes itself via synchronous `GameplayStatics.SaveGameToSlot`, UserIndex 0, SlotName field; propagates the native bool. | Actual competing slot ownership exists if integrated unchanged. Route payloads under the one eventual project coordinator. |
| Same, `InitActor_BPI` / `InitActorWithSaveID_BPI` | Assigns a new ID from `SavedActors` map length, or accepts a supplied ID, calls the actor save interface and adds it to the map. | These IDs are not proven stable across independent procedural generation/streaming or deleted-map gaps. They cannot be assumed to identify finite terrain deposits. |
| Same, `SaveLevelData_BPI`, `LoadLevelData_BPI` | Stores/looks up per-level actor/building data and transfers temporary maps. | No inspected connection waits for GeoForge geometry/nav readiness or captures an inventory/terrain transaction. |
| `Game/BP_EBS_PlayerController`, `Building events`, save/load server events | Save creates a save object, enumerates save-interface actors, initializes IDs, captures actor data, stores level data and writes the slot. Load checks the slot/level, destroys current save-interface actors, then respawns saved classes and restores their IDs/data. | This is a destructive scene-reconstruction strategy, not a proven coherent whole-game restore. Do not adopt the demo controller as the WYRMFALL save coordinator. |
| `BuildingObjects/Base/BP_EBS_Building_BaseObject`, `SaveData_BPI` / `LoadData_BPI` | Captures class, transform, handle, formatted variables and building durability/floor fields; loads building state through the save interface. | Existing building payload is useful; humanoid combat remains GAS-owned. |
| `Game/BP_EBS_SaveLibrary` | Formats/parses typed values through delimited strings. | Serialization helpers are not transaction ownership, version migration, corruption recovery or terrain save acceptance. |

`Game/BP_EBS_Library` was also loaded/exported. The demo player controller includes
movement/camera and `ApplyDamage` paths: wholesale adoption would conflict with the
existing control/GAS host. No EBS assets were copied into WYRMFALL Content and no
runtime owner or dependency was added.

## Inventory candidate inspection: Advanced Grid Inventory System (AGIS) and search evidence

The previously recorded candidate was **Advanced Inventory System AAA**, module
`AdvancedInventorySystem`, located at `D:/Unreal/UE_5.8/Engine/Plugins/Marketplace/Untitled5ad389823e25V1`.
A comprehensive multi-drive search across fixed drives (C:, D:, E:, F:, G:, I:) confirmed that
physical drive D: suffered device error 433 (`[WinError 433] A device which does not exist was specified`),
and no readable file or archive exists for that package.

Following explicit user authorization to inspect and install needed assets and candidate packages,
the local candidate **Advanced Grid Inventory System (AGIS)** by Kaya Products was discovered at
`G:\VaultCache\Advancedc03c38f197d4V1`. All 663 assets were verified 100% readable with zero device errors.

### Native UE 5.8 AGIS inspection method and evidence

The C: UE 5.8.2 editor loaded `G:\VaultCache\Advancedc03c38f197d4V1\data\AGIS_7.uproject` via
`UnrealEditor-Cmd.exe` headless with `-EnablePlugins=PythonScriptPlugin`. AGIS is a pure Blueprint
system requiring zero C++ compilation; 9,463 assets were discovered cleanly during load.
Native `ObjectExporterT3D` exported 10 core Blueprints into `Saved/Diagnostics/AGISOwnerInspection/`:

1. `Inventory__Main`: 103 functions (`Add Item`, `Add Item to Container`, `Can Add Item Inside`,
   `Can Stack Item`, `Find Space In Container`, `Delete Item By Address`, `Rotate Item`).
   Full spatial grid inventory supporting 2D width/height tiles, rotations, stack maximums, and nested bags.
   `Can Add Item Inside` performs strict bounds and tile occupancy checks; failure returns false rather than silently discarding items.
2. `Inventory_Player`: 76 functions (`Client Trace`, `Can Drop Item`, `Drop Item By UID`, `Empty Hands SERVER`).
   Handles player interaction traces, HUD/widget creation and caching, and player item drops.
3. `Inventory_Storage` and `Inventory_Crafter`: Storage container and crafting table logic.
4. `FL_AGIS`: 52 functions (`Get AGIS SG Slot Name for Level`, `Extract Items From Containers`, `Find Empty Tile`).
5. `SG_AGIS_World`: SaveGame object containing `Save Player_MERGED`, `Save StorageActor_MERGED`, `Save ItemActor_MERGED`,
   and UID counter mappings.
6. `GameInstance_AGIS`: 15 functions (`Save World`, `Load World`, `Save Player Inventories`, `Load Player Inventories`,
   `Save StorageActors`, `Load StorageActors`, `Save ItemActors`, `Load ItemActors`, `Save Last UIDs`, `Load Last UIDs`).
   Orchestrates save and load across player inventories, storage actors, and world item pickups.
   On `Load World`, transient level items are cleaned up and reconstructed deterministically from saved structs.
7. `PlayerController_AGIS`: Confirmed completely decoupled from controller hierarchy.
   The author explicitly documented: `NodeComment="InventoryController is empty. You can use your own controller."`
   Interaction line-traces and inventory listening route through the pawn's component rather than locking down controller inheritance.
8. `BP_AGIS_ExampleCharacter`: Documents clear, minimal integration requirements:
   - Add `Inventory_Player` component to the pawn.
   - Implement `BPI_AGIS_Player` interface, providing only `Get Trace Points` (camera location and forward vector for interaction traces).
   - Optional `BPI_AGIS_CharacterAnimations` interface for locomotion tags.
9. `BP_ItemSpawner` and `_BP_ItemBase`: Spawns physical world item pickups (`/Game/INVENTORY/Items/BaseBlueprints/_BP_ItemBase`)
   with physics, collision, and item payload data when items are spawned into the world or dropped via `Drop Item By UID`.

- Export report: `Saved/Diagnostics/AGISOwnerInspection/report.json` and `save_load_report.json`.
- Source packages remained byte-for-byte identical; exports remain under excluded Saved diagnostics.
- Process logs: `Saved/ScaffoldLogs/20260915T204800Z_agis_owner_inspection.log` and `save_load_report.json`.
- Search evidence: `Saved/Diagnostics/WP00_inventory_source_blocker.json` and `Saved/Diagnostics/WP00_inventory_source_search.json`.

## Architectural reconciliation: Capacity, Overflow, Rewards, and Save Ownership

Based on the inspected EBS graphs and GeoForge completion probes, the architectural
boundaries and transaction contracts between subsystems are reconciled as follows:

| Concern | Subsystem seam & current inspected state | WYRMFALL reconciled authority & contract |
|---|---|---|
| **Authority** | EBS has `BP_EBS_ResourcesComponent`; GeoForge has terrain voxels; GAS has attributes; Mutable has meshes. | **Strict single-owner principle:** Mutable owns mesh customization; GAS owns combat attributes and gameplay effects; one selected inventory authority owns item instances, counts, and slots; EBS owns building placement mechanics only. No duplicate economy or resource components. |
| **Capacity & Overflow** | EBS `AddResource` appends/increments with no max capacity or overflow logic. | When items/resources are acquired, the inventory authority must enforce capacity constraints. If inventory is full, an explicit overflow policy must execute: drop as a physical world pickup actor at the player position, or fail the acquisition transaction with UI feedback. Items must never be silently destroyed or allowed to overflow without bound. |
| **Terrain Rewards** | GeoForge edits (`DigSphere`) remove voxels and calculate yields. | Voxel yield generation and inventory intake must be an atomic transaction. Depleted voxels award resources to inventory; if inventory reject/overflow occurs, physical world drops are spawned. Terrain yield must never desynchronize from voxel state. |
| **Building Spend** | EBS `BP_EBS_PlayerController` calls `RemoveResources` in a loop with no rollback on mid-batch failure. | EBS building requirement checks and deductions must route through the project's inventory authority rather than EBS's internal resource component. Spends must be atomic: all required materials are validated and committed together, or the build attempt is rejected with zero deduction. |
| **Save Ownership** | EBS `BP_EBS_SaveGame` uses its own slot and destructive actor respawn on load. GeoForge uses `BuildTerrainSaveData`/`ApplyTerrainSaveData`. | Neither EBS nor GeoForge may act as independent save coordinators. A single top-level WYRMFALL save coordinator will orchestrate saving: (1) player transform & GAS attributes, (2) player inventory state, (3) placed EBS building records, and (4) GeoForge voxel diffs into a unified save payload. Load must restore state coherently without destructive uncoordinated scene clears. |


## Waterline implementation inspection

The supplied `UE_WaterlinePRO6_DownloadPiratecom.rar` was extracted locally with
7-Zip into `Saved/AssetIntake/WP00/WaterlineOwner`. Extraction reported success:
1,007 files, 2,010,784,977 uncompressed bytes. A minimal isolated 5.8 project
`WaterlinePRO/WaterlineOwnerInspection.uproject` enabled Python for inspection.
No vendor packages were migrated into the game or modified/saved by the exporter.
The full source/extract listings are local, not license/provenance acceptance.

- Manifest: `Saved/Diagnostics/Waterline_implementation_manifest.txt`.
- Extraction: `Saved/Diagnostics/Waterline_extract.txt`.
- Native report/hashes: `Saved/Diagnostics/WaterlineOwnerInspection/report.json`.
- Five selected Blueprints loaded/exported; all inspected source hashes unchanged.
- Process: `Saved/ScaffoldLogs/20260915T181828Z_a0eac6206b63_waterline-owner-graphs.log`
  and `.json`; exit 0, 40.610 seconds. Connectivity timeout warning only in the
  inspected warning/error output. No runtime water behavior was exercised.

Exact extraction and native commands:

```powershell
& 'C:/Program Files/7-Zip/7z.exe' x `
  'assets and old docs/UE_WaterlinePRO6_DownloadPiratecom.rar' `
  '-oG:/assets/voxel project/Saved/AssetIntake/WP00/WaterlineOwner' -aos -bso1 -bsp0
& 'C:/Program Files/UE_5.8/Engine/Binaries/Win64/UnrealEditor-Cmd.exe' `
  'G:/assets/voxel project/Saved/AssetIntake/WP00/WaterlineOwner/WaterlinePRO/WaterlineOwnerInspection.uproject' `
  /Engine/Maps/Entry -WyrmWaterlineOwnerProbe -unattended -nop4 -nosplash -nosound -nullrhi `
  '-ExecutePythonScript=G:/assets/voxel project/tools/unreal/inspect_wp00_owner_graphs.py' `
  -stdout -FullStdOutLogOutput
```

All selected asset paths are explicit in the inspection script. Connected graph
inspection found:

- `1_Water_Large` and `Water_Volume_Basic` construct surface/underwater materials
  and post-process blendables. This is accessible visual implementation.
- `BP_Lake_2 / Get Water Height` returns actor Z minus `(Buoyancy Offset - 20)`;
  `Circle_Water_Volume / Get Water Height` returns `Water Plane Location.Z`.
  Neither function accepts a query position or reads GeoForge geometry. These
  functions cannot themselves prove wetness in an excavated/filled cave.
- The lake's buoyancy-volume overlap paths add/remove entries; entry can enable
  physics. Its connected Tick path reaches force and torque application.
- The circle volume's connected overlap paths add/remove entries and set linear/
  angular damping. Connected Tick reaches force, component line traces and
  render-target update paths. These are physical side effects to reconcile with
  any chosen water/terrain authority, not automatically safe visual-only actors.
- `BP_Buoyancy` contains force/mesh/Niagara operations, but its authored Tick,
  particle-data and overlap event execution outputs are disconnected; BeginPlay
  ends at a disconnected reroute. Presence of those nodes is not proof that this
  standalone Blueprint supplies functioning runtime buoyancy.
- No project wet-state, terrain-edit water-edge or game-save integration was
  established in these five graphs. This bounded inspection does not assert that
  the entire vendor package lacks other features. UWS remains uninspected and
  unselected; no second water stack was enabled.

**Decision:** Waterline is a readable visual/physical candidate, not yet the
single logical water-state owner. GeoForge's fluid/volume/wet-query implementation
and these Waterline actors must not independently decide contradictory wetness or
apply duplicate forces. Actual water-edge/save behavior remains NOT_RUN.

## Readiness boundary and next task

WP-00 remains PARTIAL. GeoForge is still the first candidate for a future single
provider proof, with the previously recorded bounded synchronous bridge evidence;
it remains unselected in project configuration. WP-01/full RDY-02/03/04 stay
NOT_RUN. This continuation added inspection tooling and evidence only.

Open inputs/evidence are now specific:

1. **Inventory candidate inspection complete:** Advanced Grid Inventory System (AGIS)
   in `G:\VaultCache\Advancedc03c38f197d4V1` has been fully inspected across 10 core
   Blueprints in native UE 5.8.2. Author-confirmed controller decoupling, pawn component
   attachment, `_BP_ItemBase` physical pickup drops, and `SG_AGIS_World` savegame
   structures are documented. The unreadable D: `Advanced Inventory System AAA` blocker
   is resolved by the user's asset inspection/install permission and the completed AGIS inspection.
2. **EBS integration:** Must replace its demo resource/slot/control ownership through
   existing seams. Building spends must route atomically through the inventory authority
   before placement completes. No runtime transaction or coherent save/reload test has passed.
3. **Waterline/GeoForge:** Must have one logical wet-state/physics decision and actual
   water-edge tests; source graphs alone do not decide the authority.
4. **Real-asset review:** Real material completion, knight/dragon scale and assembled
   visuals, selected enemy/weapon opening and complete asset provenance acceptance remain open.

**Next bounded task:** Conclude WP-00 scoped readiness acceptance with all three candidate
subsystem owners (EBS for building, Waterline for water visuals/physics, AGIS for inventory)
inspected and structurally reconciled. Then proceed to the single eligible WP-01 terrain provider
proof (GeoForge synchronous collision/nav bridge) under preserved acceptance criteria.

No new C++/game configuration was changed, so the prior bridge build, six native
regressions and focused PIE remain the latest evidence for that code. EBS, Waterline,
and AGIS gameplay, save/quit/reload, full terrain proof and cook were NOT_RUN.

## Portable checks and changed files

- `py -3.12 tools/wyrm.py verify`: PASS (`Saved/Diagnostics/WP00_owner_verify.txt`).
- `py -3.12 tools/wyrm.py test`: 124 tests, OK, two platform/privilege skips,
  8.954 seconds (`Saved/Diagnostics/WP00_owner_tests.txt`).
- `git -c safe.directory="G:/assets/voxel project" diff --check`: PASS.
- Tracked continuation changes: this report, `STATUS.md`, `HANDOFF.md`,
  `WP-00_SCOPED_READINESS.md`, `Config/IntegrationReadiness.json`, and
  `tools/unreal/inspect_wp00_owner_graphs.py`. These remain local after the
  requested initial commit/push. Exports, extracted vendor content and raw logs
  remain excluded from Git. No C++ or runtime engine/project configuration change.
