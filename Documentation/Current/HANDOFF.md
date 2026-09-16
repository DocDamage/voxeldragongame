# WYRMFALL handoff - September 15, 2026

## Resume here

Continue v0.2 in `G:/assets/voxel project`; do not re-plan the game. Read root
`AGENTS.md`, `CODEX_START_HERE.md`, `Documentation/Current/STATUS.md`, then this
handoff, the relevant reports below and `Documentation/Current/tasks/WP-00.md`.
Preserved DesignPack requirements still apply; do not ingest the entire backlog.

Repository: https://github.com/DocDamage/voxeldragongame
Branch: `main`. The commit containing this file is the handoff snapshot; the user
response supplies its pushed hash. Earlier checkpoints: `cc8ce1a` (GeoForge bridge),
`185fa0c` (EBS/Waterline inspection). Inspect working state before editing.

Use this ownership override for **every Git command**:

```powershell
git -c safe.directory="G:/assets/voxel project" status --short
```

Do not change global Git settings. A previous stat-only DefaultEngine.ini modified
marker had an empty actual diff. Native editors can append AndroidFileServer
machine settings; do not commit those. Preserve unrelated changes.

## Verified host

- UE: `C:/Program Files/UE_5.8`, version 5.8.2, CL 56702186, compatible BuildId 55116800.
- Use `py -3.12`; default Python is 3.10. Builds passed with `-NoUBA -NoPCH`.
- The old D: engine suffered device/read/write failures; do not build against it.
- GeoForgeRuntime 5.0.0 is installed/readable at
  `C:/Program Files/UE_5.8/Engine/Plugins/Marketplace/GeoForge33217d52984fV2`.
- GeoForge was command-line enabled for probes only; it is **not integrated or
  selected** in WYRMFALL. ProceduralMeshComponent is explicitly enabled.
- Fab cache is `C:/EpicVaultCache/VaultCache`, changed with prior approval.
  Installation is fixed. Do not repeat troubleshooting without a new failure.
  See `GEOFORGE_INSTALL_RECOVERY.md`.

## Actual native evidence

WP-01: One Real Terrain Provider Proof (GeoForge) has been implemented and verified:
- **Authoritative Adapter Authored:** `AWyrmGeoForgeAdapter` implementing `IWyrmTerrainProvider`
  bridges WYRMFALL directly to `GeoForgeRuntime 5.0.0` (`AGeoForgeActor`).
- **C++ Native Compilation:** UE 5.8.2 Development Editor build passed cleanly in 11.72s (`-NoUBA -NoPCH`).
- **Native Automation Test Suite:** 8/8 SUCCEEDED (`Saved/Automation/Scaffold/index.json`), including
  two new test cases: `Scaffold.Wyrm.AdapterCapability` and `Scaffold.Wyrm.AdapterYield`.
- **Headless PIE Proof Suite:** `Saved/Diagnostics/WP01_terrain_provider_proof.json` verifies 7/7 cases:
  - `WRLD-01`: Synchronous dig collision dropped 400.0 cm (900.0 -> 500.0 cm) with zero pending background jobs.
  - `WRLD-02`: Synchronous refill collision restored floor back to 1000.0 cm.
  - `WRLD-03`: Finite yield extraction (`Resource.Dirt`, count 131, volume 65,449,848 cm3) and duplicate action rejection with `duplicate_prevented = true` and `extracted_count = 0`.
  - `WRLD-04`: New surface Recast navigation projected at lower excavated elevation [1550.0, 850.0, 560.0] cm.
  - `WRLD-05`: Stale sub-surface navigation point at Z=500.0 cm returned None after refill.
  - `WRLD-08`: Occupied-fill protection detected pawn envelope (`CheckVolumeOccupied`), rejected edit, and preserved crater.
  - `SAVE-01..04`: Terrain delta persistence serialized 57,286-byte binary payload, reset geometry, and successfully restored floor elevation upon reload.
- **Evidence & Report:** [Documentation/Current/WP01_TERRAIN_PROVIDER_PROOF.md](WP01_TERRAIN_PROVIDER_PROOF.md).

WP-02: Playable Mutable Character Recipe & Runtime Proof has been implemented and verified:
- **Authoritative Recipe Authored & Compiled:** `CO_Knight` (`/Game/WYRMFALL/Characters/Player/CO_Knight.uasset`)
  authored using `FWyrmMutableRecipeBuilder` and compiled cleanly via `CustomizableObject` plugin compiler in UE 5.8.2.
  Recipe includes Base Mesh (`SK_Knight`), Mesh Switch (`Helmet` with `SK_KnightHelmDown` / None), and Vector Parameter (`ArmorTint`).
- **C++ Runtime Component Integration:** `AWyrmCharacter` binds `UCustomizableSkeletalComponent` directly to `GetMesh()`.
  Implemented dynamic parameter manipulation (`SetColorParameter`, `SetIntParameter`), appearance snapshot persistence
  (`CaptureAppearanceDescriptor`, `RestoreAppearanceDescriptor` via Base64 serialization), and socket-based equipment attachment
  (`AttachEquipmentMesh` with automatic `Movable` mobility enforcement).
- **Native Automation Test Suite:** 9/9 SUCCEEDED (`Saved/Automation/Scaffold/index.json`), including new test
  `Scaffold.Wyrm.CharacterMutableBinding` verifying component binding, parameter reflection, and appearance persistence.
- **Headless PIE Proof Suite:** `Saved/Diagnostics/WP02_mutable_recipe_proof.json` verifies 7/7 cases:
  - `CHAR-01`: Authoritative Mutable recipe asset `CO_Knight` loaded and compiled.
  - `CHAR-02`: Runtime character spawned with `UCustomizableSkeletalComponent` generating dynamic skeletal instance.
  - `CHAR-03`: Mesh switch mutation (`Helmet` parameter index 0 <-> 1) successfully swapped helmet geometry.
  - `CHAR-04`: Material color parameter mutation (`ArmorTint`) modified runtime instance without invalidating skeletal hierarchy.
  - `CHAR-05`: Equipment mesh attachment (`SM_Sword`) cleanly attached to `Hand_Right` socket of dynamic Mutable mesh.
  - `CHAR-06`: Base64 appearance descriptor captured, cleared, and restored with exact parameter state fidelity.
- **Evidence & Report:** [Documentation/Current/WP02_MUTABLE_RECIPE_PROOF.md](WP02_MUTABLE_RECIPE_PROOF.md).

## Actual owner implementation inspection

`WP00_OWNER_IMPLEMENTATION_INSPECTION.md` records native graph exports and traced
execution/data pins for eight EBS, five Waterline, and ten AGIS Blueprints. Inspected source
hashes stayed unchanged; vendor graph text remains under Saved.

- EBS owns resource balances; its inspected batch deduction has no rollback.
  Its demo controller also contains movement, damage and interaction behavior.
- EBS owns a save slot and destructive actor-reconstruction load flow. Do not
  adopt its controller wholesale or introduce a second inventory/save owner.
- Waterline has visual and physical side effects. Its inspected height functions
  do not query edited voxel terrain. Reconcile one logical wet-state/physics
  authority with GeoForge. Standalone BP_Buoyancy contains disconnected event
  paths; node presence is not runtime proof. UWS is unselected/uninspected.
- AGIS (Advanced Grid Inventory System by Kaya Products, `G:\VaultCache\Advancedc03c38f197d4V1`)
  has complete spatial grid maths, rotation, container UIDs, author-confirmed decoupled
  controller (`PlayerController_AGIS` is empty), pawn component attachment (`Inventory_Player`),
  physical pickup spawning (`_BP_ItemBase` via `BP_ItemSpawner`), and `SG_AGIS_World` savegame
  coordination. Clean native load in UE 5.8.2; zero C++ compile required.

## Actual real-asset evidence

`WP00_REAL_ASSET_REVIEW.md` records native imports, measured bounds/materials and
two visually inspected static renders of the knight, assembled 44-part Green
Dragon, wolf, sword, dirt and stone. Dragon parts share a 195-bone skeleton;
knight has 15 bones; wolf has 23 bones and seven imported clips.

The sword comes from Knights. Premium Armory contains PNG icons, not a 3D weapon.

- Native report: `Saved/Diagnostics/WP00_asset_review.json`.
- Preview evidence: `Saved/Diagnostics/WP00Preview/report.json`, `manifest.json`,
  `visual_review.json`.
- Images: `Saved/Diagnostics/WP00Preview/assembled_front.png`, `assembled_rear.png`.
- Explicit diagnostic scales: knight 0.5 (about 180 cm), dragon 0.02 (about
  272 x 361 x 229 cm), wolf 0.5, sword 1/3 (about 100 cm long).
- These are static preview choices, not accepted game scale, animation, collision,
  Mutable recipes, dragon combat/riding/flight or production material approval.
- Wolf/sword FBX smoothing-group warnings remain. The knight static pose has
  separated hand/foot blocks; no speculative mesh repair was applied.

Portable verify passed. Latest full portable suite: 124 tests OK, two
platform/privilege skips, 8.414 seconds. Commands:

```powershell
py -3.12 tools/wyrm.py verify
py -3.12 tools/wyrm.py test
```

## Next bounded task: WP-03 Shared Humanoid Control & Movement

With **WP-01** (One Real Terrain Provider Proof) and **WP-02** (Playable Mutable Character Recipe & Runtime Proof)
both fully verified and passing native test automation and headless PIE proof suites:

Proceed to **WP-03**:
1. Implement shared humanoid control supporting both third-person direct WASD/gamepad movement and
   top-down click-to-move navigation on the same humanoid actor.
2. Implement seamless camera perspective switching with persistent mode preferences (`SAVE-05` / `UI-01..03`).
3. Enforce movement gating under interaction / pause / hit-stun states without duplicating pawn progression.
4. Maintain single GAS authority on humanoid.


