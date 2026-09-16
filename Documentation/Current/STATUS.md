# Current implementation status

**September 16, 2026 · starter v0.2 · C: native build/tests and focused PIE exercised**

This status supersedes only the old documentation pack's statement that no scaffold exists. It does not change approved design or grant broader Git/art permissions. The uploaded pack's 36 files remain byte-for-byte preserved under Documentation/DesignPack.

| Area | Actual status | Evidence / next proof |
|---|---|---|
| Source project | AUTHORED | WYRMFALL.uproject, targets, runtime module, Config |
| Portable tooling | Test results recorded in root VALIDATION.md | Standard-library tests; no engine emulation |
| Engine on this Windows host | UE 5.8.2 VERIFIED on C: | `C:\Program Files\UE_5.8`, CL 56702186; old D: failure retained as history |
| Windows UE installation | Five required descriptors INSPECTED | Mutable 1.8.0, EnhancedInput, GAS, Python and EditorScriptingUtilities; doctor is metadata evidence |
| UHT / UBT compile | PASS | Actual headers; latest build with per-command `-NoUBA -NoPCH` succeeded in 14.52 seconds |
| Native automation | 22/22 Success | Latest `Saved/Automation/Scaffold/index.json`, complete source-declared suite including CharacterMutableBinding, AdapterCapability, AdapterYield, SharedControlFoundation, CombatCanonicalDamage, CombatBoundsAndDrain, CombatCostAndCooldown, CombatEnemyRolesAndStatus, ItemGenerationAndRolls, InventoryCapacityAndTransfer, EquipmentStatApplication, SaveSubsystemRoundtrip, ProgressionSaveRoundtrip, ProgressionXpAndLevelUp, RangedProjectileDamage, WeaponFamilyGatingAndKitSwitch |
| Diagnostic map | SAVED and unchanged on bootstrap rerun | Real 17,817-byte `.umap` and validated receipt |
| PIE, input, cameras, HUD | VERIFIED (AWyrmPlayerController & AWyrmCharacter); WP-03 PASS | Seamless third-person & top-down camera switching, direct movement cancelling click-move (WRLD-06), movement lock and pause gating (UI-02, UI-07), control state persistence (SAVE-05); 8/8 PIE checks pass; [evidence](WP03_CONTROL_PROOF.md) |
| Mutable & Character | VERIFIED (CO_Knight & AWyrmCharacter); WP-02 PASS | Authoritative recipe authored/compiled; runtime binding, parameters (Helmet, ArmorTint), socket attachment (SM_Sword on Hand_Right), and persistence verified; [evidence](WP02_MUTABLE_RECIPE_PROOF.md) |
| Terrain | GeoForge VERIFIED (AWyrmGeoForgeAdapter); WP-01 PASS | Authoritative adapter authored; 8/8 native tests pass; 7/7 PIE checks (WRLD-01..05, 08, SAVE-01..04) pass; [evidence](WP01_TERRAIN_PROVIDER_PROOF.md) |
| GeoForge prerequisite | ProceduralMeshComponent bundled and native load PASS | Explicitly enabled in the project and loaded successfully alongside GeoForge; [dependency evidence](PROCEDURAL_MESH_DEPENDENCY.md) |
| Real sample assets | Dirt/stone/knight/44-part Green Dragon plus wolf/sword imported and statically previewed | [Measured diagnostic scales and supplied palettes](WP00_REAL_ASSET_REVIEW.md); animation, collision, Mutable and production suitability unverified |
| GAS & Combat | VERIFIED (AWyrmCharacter & AWyrmEnemyCharacter); WP-04 PASS | Authoritative GAS combat loop, canonical damage mitigation formulas (COM-01), shield absorption, invulnerability immunity, dead clamp (COM-02), two-camera combat (COM-03), 20 Focus cost & 5s cooldown gating (COM-04), slow combining & boss CC resistance (COM-05); 18/18 native tests pass; 5/5 PIE cases pass; [evidence](WP04_COMBAT_PROOF.md) |
| Inventory, equipment & save | VERIFIED (UWyrmInventoryComponent & UWyrmSaveSubsystem); WP-05 PASS | Single inventory authority on character, capacity & stacking, overflow rejection (COM-07), rolled item affix generation & idempotent equipment stats with zero leaks (COM-06), single save coordinator unifying character attributes, camera, appearance, inventory, and GeoForge terrain delta (SAVE-01); 18/18 native tests pass; 3/3 PIE checks pass; [evidence](WP05_INVENTORY_PROOF.md) |
| Second Build & Progression | VERIFIED (Ranged Skirmisher, AWyrmProjectile, Level/XP); WP-06 PASS | Real supplied ranger bow/arrow assets, physical projectile combat, weapon family gating & dynamic kit switching, GAS-authoritative Level/XP progression fixture, progression save roundtrip (COM-08); 22/22 native tests pass; 4/4 PIE checks pass; [evidence](WP06_PROGRESSION_PROOF.md) |
| Dragon, Heartfold, Echoes | Requirements and integration locations preserved; NOT IMPLEMENTED | Original WP-09 onward |
| Vehicles/colony | Future required deliverables; NOT IMPLEMENTED | Original WP-18/19 |
| Git/remote/branch | Git initialized; origin configured; main | [DocDamage/voxeldragongame](https://github.com/DocDamage/voxeldragongame); existing remote history retained; generated data and supplied/imported vendor assets excluded |

## v0.2 changes

Added bounded onboarding/recovery reports and source/evidence freshness checks;
made the native runner require the source-declared suite; added process-tree
cleanup and unique hashed attempt logs; preserved asset roots during engine-path
updates; strengthened metadata/map receipts. The existing C++ shell now guards
input rebinding/paused movement and clamps GAS base health as well as current
health. These native changes and the typed map checks have now been exercised
on the C: UE 5.8.2 installation, as scoped in the recovery report.

The first-run wrapper is offline by default; `--native` explicitly adds local
engine steps. No new gameplay owner, terrain provider or speculative content
system was introduced. See [changes](../../CHANGELOG.md).

## What Codex should do next

Use the [current handoff](HANDOFF.md) for paths, evidence and the bounded next task.

Read [the current C: recovery report](BOOT-01_C_RECOVERY.md). Finish
[WP-00](tasks/WP-00.md) at the inventory-source/remaining acceptance boundary;
then one eligible [WP-01](tasks/WP-01.md) proof. Do not jump to the entire first
region. GeoForge's stale D: registration and failing D: cache were recovered with
user approval. Fab now uses `C:\EpicVaultCache\VaultCache`; GeoForge is installed
in the C: engine and native loading passed. Finish the remaining WP-00 inspections.

## Unverified boundaries to keep visible

Input mappings are transient developer defaults rather than final remappable input assets. The top-down fixture has no roof/occlusion, menus, target selection or edit-aware nav invalidation. Camera preference is not saved. The humanoid is intentionally unmeshed. GAS state is owned by the humanoid while it exists; destruction/region transfer reconstruction is not implemented. The terrain seam intentionally omits speculative vendor completion/save APIs. The map builder never creates production materials, rigs, animations or UI.

Later sessions must replace these specific statuses with observed results, not delete the limitations wholesale. Use [the report template](SESSION_REPORT_TEMPLATE.md).

## Current Windows evidence and remaining boundary

[Current C: recovery, commands and PIE evidence](BOOT-01_C_RECOVERY.md) ·
[WP-00 scoped readiness](WP-00_SCOPED_READINESS.md) ·
[Historical D: failure](BOOT-01_WINDOWS_SESSION.md).

Use installed `py -3.12`; default `python` is 3.10. The final C: editor build,
real bootstrap map, six native tests and focused keyboard/mouse PIE checks passed.
The portable suite ran 124 tests with two platform/privilege skips. The old
onboarding receipt correctly reports STALE_EVIDENCE after the engine/source
changes; current individual execution evidence is listed in the recovery report.

WP-00 native sample intake found real dirt/stone meshes and materials, a
15-bone knight through the current FBX importer, and 44 Green Dragon skeletal
parts sharing a 195-bone imported hierarchy plus 20 clips. Legacy knight import
failed with multiple roots; the source FBX was preserved. Later native wolf/sword
imports, supplied-palette previews and static dragon assembly are recorded in
[the real-asset review](WP00_REAL_ASSET_REVIEW.md). Diagnostic scales are measured,
not production approvals. Animation, collision, Mutable, full ownership/provenance
acceptance and inventory source inspection remain incomplete.

The C: engine now has GeoForgeRuntime 5.0.0 with native runtime/editor/dependency
loading verified. Authoritative adapter AWyrmGeoForgeAdapter is implemented and compiled
into the WYRMFALL runtime module. 8/8 native automation tests pass under Unreal Editor.
The headless PIE verification suite (`verify_wp01_terrain_proof.py`) exercised synchronous
digging/refilling with zero pending queues (`WRLD-01`, `WRLD-02`), finite yields and duplicate
prevention (`WRLD-03`), new surface nav projection (`WRLD-04`), stale sub-surface nav
cancellation (`WRLD-05`), active pawn envelope occupied-fill protection (`WRLD-08`), and
binary save/load payload round-trip restoration (`SAVE-01..04`). See [WP-01 report](WP01_TERRAIN_PROVIDER_PROOF.md).
WP-01 is **PASS / VERIFIED**.

[Native owner inspection](WP00_OWNER_IMPLEMENTATION_INSPECTION.md) now covers eight
EBS, five Waterline, and ten AGIS Blueprints with unchanged source hashes. EBS has its own
resource balances and save-slot/actor-reconstruction flow; Waterline has physical
side effects as well as visuals; AGIS (Advanced Grid Inventory System by Kaya Products,
`G:\VaultCache\Advancedc03c38f197d4V1`) has full spatial grid math, container UIDs,
author-confirmed controller decoupling (`PlayerController_AGIS` is empty), `Inventory_Player`
component attachment, `_BP_ItemBase` physical pickup spawning, and `SG_AGIS_World` savegame
coordination. None were integrated into WYRMFALL runtime descriptor. Architectural
reconciliation across capacity, overflow, terrain rewards, EBS spends, and save
coordination is documented in the owner inspection report.

[Real-asset review](WP00_REAL_ASSET_REVIEW.md) now adds native wolf/sword imports,
explicit diagnostic scales, supplied palette materials and two inspected renders
of the 44-part assembled Green Dragon, knight, wolf, sword, dirt and stone.
Static visual inspection does not clear animation, Mutable, collision or full
RDY acceptance. The Armory ZIP is PNG icons; the real sword came from Knights.

**Next bounded task:** WP-01 (GeoForge terrain provider proof), WP-02 (Playable Mutable character recipe & runtime proof),
WP-03 (Shared humanoid control, camera switching & input gating proof), WP-04 (First real combat loop proof),
WP-05 (Loot, equipment, inventory, and coherent save snapshot), and WP-06 (Second build and progression fixture)
are all complete and verified with native test automation and headless PIE evidence suites. The next bounded milestone is
**WP-07: Vertical Slice Integration Map & Encounter Fixture** (combining editable terrain, combat encounters, loot drop triggers,
progression reward loop, and coherent save state in a playable developer integration arena).
