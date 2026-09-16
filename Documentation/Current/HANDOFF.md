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

WP-03: Shared Humanoid Control, Camera Switching & Input Gating Proof has been implemented and verified:
- **Shared Controller & Character Architecture:** `AWyrmPlayerController` and `AWyrmCharacter`
  support both third-person follow camera (ArmLength=450, FOV=80, PawnRotation=true, Cursor=hidden)
  and top-down isometric camera (ArmLength=1100, Pitch=-55, FOV=55, PawnRotation=false, Cursor=visible).
- **Direct Movement Cancels Path Navigation (`WRLD-06`):** Calling `Move()` on controller immediately
  triggers `StopMovement()` before applying pawn input, interrupting active click-to-move pathfinding.
- **Strict Input Gating (`UI-02`, `UI-07`):** Movement, look, and jump inputs are strictly suppressed when
  `bMovementLocked` is set on either controller or character, when input is ignored, or when game is paused.
- **Control State Persistence (`SAVE-05`):** `FWyrmControlState` serializes camera mode, click-to-move toggle,
  and movement lock state to/from JSON with exact roundtrip fidelity.
- **Native Automation Test Suite:** 10/10 SUCCEEDED (`Saved/Automation/Scaffold/index.json`), including new test
  `Scaffold.Wyrm.SharedControlFoundation` verifying camera mode toggling, boom adjustments, and state serialization.
- **Headless PIE Proof Suite:** `Saved/Diagnostics/WP03_control_proof.json` verifies 8/8 cases:
  - `CTRL-00`: Character and controller spawn, possession, and binding verified.
  - `CTRL-01`: Third-person camera defaults (ArmLength=450, FOV=80, PawnRotation=true, Cursor=hidden) verified.
  - `CTRL-02`: Top-down mode switch (ArmLength=1100, FOV=55, PawnRotation=false, Cursor=visible) verified.
  - `CTRL-03`: Click-to-move pathfinding with reachability check and unreachable point rejection verified.
  - `CTRL-04`: Direct movement interruption and path cancellation via `StopMovement()` verified.
  - `CTRL-05`: Input lock gating verified on both controller and character.
  - `CTRL-06`: Control state capture and restore roundtrip verified with JSON fidelity.
  - `CTRL-07`: Return to third-person camera verified.
- **Evidence & Report:** [Documentation/Current/WP03_CONTROL_PROOF.md](WP03_CONTROL_PROOF.md).

WP-04: First Real Combat Loop Proof has been implemented and verified:
- **Authoritative GAS Combat Architecture:** `AWyrmCharacter` and `AWyrmEnemyCharacter` implement
  `IAbilitySystemInterface` with `UAbilitySystemComponent` and `UWyrmAttributeSet`. GAS is the sole authority
  for health, shield, focus, power, armor, damage calculations, ability costs, and cooldowns.
- **Canonical Combat Formulas (`COM-01`):** Raw damage calculated via `WeaponBase + Power * PowerCoefficient`,
  mitigated via physical formula `MitigationFraction = Armor / (Armor + 50 + 10 * Level)`. Mitigated damage
  is 17.143 at Level 1 with 20 Raw vs 10 Armor (14.3% mitigation observed).
- **Bounds & Drain Enforcement (`COM-02`):** 10.0 Shield absorbed completely prior to health deduction;
  invulnerable target (`State.Combat.Invulnerable`) ignores damage; defeated targets clamp HP at 0.0 with
  `State.Dead` loose tag, movement disabled, and collision cleared (overkill prevention).
- **Two-Camera Combat Parity (`COM-03`):** Primary attacks deal identical 17.14 damage in Third-Person
  and Top-Down camera modes without state, attack, or ability reset.
- **Focus Cost & Cooldown Commit (`COM-04`):** Secondary melee attack atomically deducts 20 Focus (100 -> 80),
  applies 5.0s cooldown tag (`Cooldown.Melee.Secondary`), rejects immediate re-activation, and rejects activation
  when Focus < 20.
- **Status Interactions & Contrasting Enemy Roles (`COM-05`):** Melee Chaser (Wolf profile: 60 HP / 10 Armor / 550 Spd)
  and Ranged Skirmisher (50 HP / 5 Armor / 400 Spd) verified distinct. Slow combines by highest magnitude
  (0.3 -> 385 Spd, 0.5 -> 275 Spd). Boss enemies (`bIsBoss = true`) resist hard crowd control (stun/root).
- **Native Automation Test Suite:** 14/14 SUCCEEDED (`Saved/Automation/Scaffold/index.json`), including 4 new combat tests:
  `CombatCanonicalDamage`, `CombatBoundsAndDrain`, `CombatCostAndCooldown`, and `CombatEnemyRolesAndStatus`.
- **Headless PIE Proof Suite:** `Saved/Diagnostics/WP04_combat_proof.json` verifies 5/5 cases (COM-01 through COM-05) with exit code 0.
- **Evidence & Report:** [Documentation/Current/WP04_COMBAT_PROOF.md](WP04_COMBAT_PROOF.md).

WP-05: Loot, Equipment, Inventory, and Coherent Save Snapshot has been implemented and verified:
- **Single Save Coordinator:** `UWyrmSaveSubsystem` (GameInstanceSubsystem) and `UWyrmSaveGame` (SaveGame)
  unifying character attributes, camera mode, appearance descriptor, inventory/equipment, and GeoForge terrain delta.
  Respects the single save owner constraint without secondary managers (`SAVE-01`).
- **Authoritative Inventory Authority:** `UWyrmInventoryComponent` on `AWyrmCharacter` with bag/stash capacity,
  atomic transfers, and overflow protection (`COM-07`).
- **Stable Rolled Item Instances:** `FWyrmItemInstance` with persistent GUID, rolls, and stack counts (`COM-06`).
- **Idempotent Equipment Stat Bonuses:** Power, Armor, MaxHealth, and socket mesh attachments (`Hand_Right`,
  `spine_02Socket`, `Back_Weapon`) with **0.0 stat leak** upon equip/unequip cycles (`COM-06`).
- **Coherent Snapshot Roundtrip Across Restart (`SAVE-01`):** Player HP (85.0), Power (50.5), Camera Mode (TopDown),
  equipped weapon, 1 bag item, 1 stash item, and GeoForge terrain action preserved across save and reload.
- **Native Automation Test Suite:** 18/18 SUCCEEDED (`Saved/Automation/Scaffold/index.json`), including 4 new tests:
  `ItemGenerationAndRolls`, `InventoryCapacityAndTransfer`, `EquipmentStatApplication`, and `SaveSubsystemRoundtrip`.
- **Headless PIE Proof Suite:** `Saved/Diagnostics/WP05_inventory_proof.json` verifies 3/3 cases (`COM-06`, `COM-07`, `SAVE-01`) with exit code 0.
- **Evidence & Report:** [Documentation/Current/WP05_INVENTORY_PROOF.md](WP05_INVENTORY_PROOF.md).

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

WP-06: Second Build and Progression Fixture has been implemented and verified:
- **Supplied Real Art Intake:** Imported real supplied static meshes and textures from `assets and old docs/voxel/characters/rangers.zip` into `/Game/WYRMFALL/Items/Weapons/` (`SM_Bow`, `SM_Arrow`, `T_Bow`, `T_Arrow`).
- **Authoritative Projectile Combat:** Authored `AWyrmProjectile` with `USphereComponent`, `UProjectileMovementComponent`, team filtering (`Combat.Team.Player`), `ApplyDamageEffect`, and self-destruction.
- **Ranged Gameplay Abilities:** `UWyrmPrimaryRangedAbility` (rapid fire, 0 cost, 0.50 Power coef) and `UWyrmSecondaryRangedAbility` (charged heavy shot, 20 Focus cost, 5s cooldown, 0.90 Power coef).
- **Tactical Mobility:** `UWyrmEvadeAbility` giving the character a directional launch impulse with a 1.25s cooldown.
- **Weapon Family Gating & Dynamic Kit Switching:** Added `EWyrmWeaponFamily` (`Unarmed`, `Melee1H`, `Melee2H`, `RangedBow`) to `FWyrmItemInstance`. `AWyrmCharacter::UpdateActiveWeaponKit` dynamically binds combat abilities according to the active main-hand weapon family, restoring the unarmed kit on unequip.
- **GAS-Authoritative Progression & Attribute Scaling:**
  - Level formula: $\text{XPNeeded}(L) = 100 + 50 \times (L - 1)$.
  - Health scaling: $\text{MaxHealth}(L) = 100 + 8 \times (L - 1)$.
  - Power scaling: $\text{Power}(L) = 20 + 3 \times (L - 1)$.
  - Overflow XP retention across multi-level jumps and additive integration with active equipment bonuses.
- **Progression Persistence:** `FWyrmCharacterSaveRecord` in `UWyrmSaveGame` preserves `CharacterLevel` and `CurrentXP` across game restarts, restored faithfully by `UWyrmSaveSubsystem`.
- **Native Automation Test Suite:** 22/22 SUCCEEDED (`Saved/Automation/Scaffold/index.json`), including 4 new tests:
  `ProgressionSaveRoundtrip`, `ProgressionXpAndLevelUp`, `RangedProjectileDamage`, and `WeaponFamilyGatingAndKitSwitch`.
- **Headless PIE Proof Suite:** `Saved/Diagnostics/WP06_progression_proof.json` verifies 4/4 cases (`COM-08_Progression`, `COM-08_KitSwitching`, `COM-08_CombatArchetypes`, `COM-08_CameraParity`) with exit code 0.
- **Evidence & Report:** [Documentation/Current/WP06_PROGRESSION_PROOF.md](WP06_PROGRESSION_PROOF.md).

## Next bounded task: WP-07 Vertical Slice Integration Map & Encounter Fixture

With **WP-01** (Terrain Provider Proof), **WP-02** (Mutable Character Recipe), **WP-03** (Shared Control),
**WP-04** (First Real Combat Loop), **WP-05** (Loot, Equipment, Inventory & Coherent Save Snapshot), and
**WP-06** (Second Build and Progression Fixture) all fully verified and passing native test automation (22/22)
and headless PIE proof suites:

Proceed to **WP-07**:
1. Author a dedicated vertical slice developer encounter arena map (`L_DEV_VerticalSlice.umap`).
2. Integrate GeoForge editable terrain fixture, procedural resource extraction, and occupied-fill protection in the arena.
3. Spawn real combat encounters featuring both Melee Chaser and Ranged Skirmisher enemy roles with GAS attribute sets.
4. Wire complete gameplay encounter loop: combat victory -> loot generation -> inventory acquisition -> XP progression & stat scaling.
5. Demonstrate single-owner save/load roundtrip preserving character progression, equipment, and terrain modifications in the integrated level.
6. Author native test automation cases and headless PIE verification suite for WP-07.
