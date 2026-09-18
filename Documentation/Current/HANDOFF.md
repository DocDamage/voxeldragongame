# WYRMFALL handoff — September 18, 2026

## Resume here

Continue the v0.2 scaffold in `G:/assets/voxel project`; do not re-plan the game.
Read `AGENTS.md`, `CODEX_START_HERE.md`, `Documentation/Current/STATUS.md`, this
handoff, and only the packet for the next bounded task.

Repository: https://github.com/DocDamage/voxeldragongame

Branch: `main` (resume at pushed checkpoint `bb09254`)

Use `git -c safe.directory="G:/assets/voxel project" ...` for every Git command.
Do not change global Git settings. Keep editor-generated AndroidFileServer
settings out of commits.

## Host

- UE 5.8.2 CL 56702186: `C:/Program Files/UE_5.8`
- GeoForgeRuntime 5.0.0:
  `C:/Program Files/UE_5.8/Engine/Plugins/Marketplace/GeoForge33217d52984fV2`
- Python: `py -3.12`
- Do not use the failed D: engine installation.

## Fresh verified results

- Editor target compiled cleanly.
- Selected scaffold native automation completed 46/46 tests: 42 Success, 4 SuccessWithWarnings, and 0 failures. The focused Region 01 native test separately passed 1/1.
- Portable verification passed; 124 tooling tests passed with two expected skips.
- WP-01 real PIE passed 7/7 terrain groups.
- WP-06 real PIE passed 6/6 ranged/progression groups.
- WP-07 scoped real PIE passed 8/8 activities groups.
- WP-08 real PIE passed 7/7 supported camp & storage groups (`ACT-06..10`, `WRLD-08..09`).
- WP-09 real PIE passed 5/5 green dragon locomotion, combat & direct control groups (`DRG-01..04`, `SAVE-08`).
- WP-10 real PIE passed 5/5 green dragon riding, flight locomotion, obstacle collision & mounted persistence groups (`DRG-05..07`, `DRG-14`, `SAVE-09`):
  authentic humanoid attachment to mount socket (0, 0, 160) without duplicate actors, compact mount rejection, 3D flight locomotion (`MOVE_Flying`, max fly speed 1600), overhead clearance box sweep & obstacle collision, in-flight dismount rejection, dual flight camera views (third-person 1100cm / top-down 1800cm), safe ground landing with slope limits, mounted defeat emergency ground recovery to anchor beneath, hub companion recovery (420 Max HP), and airborne mounted save roundtrip and recovery.
- WP-11 real PIE passed 7/7 Green Dragon/Verdance Heartfold groups (`DRG-08..13`, `SAVE-08`): compact capsule doorway fit, compact combat/direct control, one-second timed form changes with four-second recovery cooldown, state conservation, blocked growth, damage interruption, town combat suppression, and compact save roundtrip. `DRG-15` passed as a native/source policy check: unvalidated rigs cannot inherit Verdance mesh, collision, mount, flight, or Heartfold values.
- WP-12 is **PASS in real PIE (REG-01..05, REG-09..11)**:
  `Content/WYRMFALL/World/Regions/L_Region01.umap` is composed with GeoForge planar finite terrain, bound `AWyrmGeoForgeAdapter`, NavMeshBoundsVolume with dynamic navmesh generation, 13 authored landmarks (`AWyrmRegion01Landmark`), 6 normalized 26-bone NPCs (`AWyrmRegion01Npc`) with authentic point-sampled palette materials, 0.5 scale factor, and idle/walk animation sequences, 5 interactables (`AWyrmRegion01Interactable`), and real vendor geometry (`Content/BanditCamp` and `Content/DarkHalls`).
  Live PIE acceptance suite `py -3.12 tools/run_wp12_production_pie_proof.py` passed all 8 production test cases under `UEDPIE_0_L_Region01`:
  - `REG-01.ArrivalExcavation`: real terrain removal edit cleared exit; player traversed through to Tamsin, triggering visit and `heart.exit_reached`.
  - `REG-11.NoActivityFactGate`: zero activity facts required; town visit recorded; quarry route accessible.
  - `REG-03.SellaFirst`: direct underworks traversal; Sella secured first granting account; auxiliary restraint disabled; notice read later; duplicate rescue rejected.
  - `REG-04.RedundantEvidence`: machine evidence discovered without written records; conflict evidence evaluated true.
  - `REG-05.PostBondRuskCustody`: flank bypass to arena; Verdance defeated alive; claim broken; dragon bonded; Rusk surrendered into custody, stopping extraction.
  - `REG-09.LateRescueAndImmediateDragon`: bond receipt issued; direct control transfer/return between player and dragon verified; partial debrief available before worker rescue; homecoming held for worker rescue.
  - `REG-02.IndependentWorkerReceipts`: Pell and Iven secured independently; relief resolved; full homecoming completed at Tidecross; Silent Landing opened.
  - `REG-10.UnifiedFactSaveRestore`: wage record recovered once with duplicate rejected; camp foundation piece placed; unified snapshot saved via `UWyrmSaveSubsystem`; memory reset; restored snapshot recovers homecoming, wage, dragon bond, camp piece, and Rusk custody outcome identically.
- WP-13 is **PASS in real PIE (REG-06, DRG-01, REG-07, REG-08, SAVE-10)**:
  `Content/WYRMFALL/World/Regions/L_Region01.umap` Verdance authored boss arena, central claim console, voluntary bond consent sequence, Crown relief squad combat, and living-defeat persistence boundaries verified in live PIE under `UEDPIE_0_L_Region01` via `py -3.12 tools/run_wp13_boss_and_bond_proof.py`:
  - `REG-06.AuxiliaryImpact`: matched trials with/without auxiliary restraint shutdown; verified arena electrical pulse interference (15 damage / 4s) active when unsuppressed, cleanly suppressed when `quarry.aux_disabled` is true; both routes winnable against the identical boss dragon.
  - `DRG-01.LivingOutcomeRoleConversion`: Verdance takes large hit (1800 -> 100 HP) and lingering damage to 0 HP; enters living terminal state (`DefeatedAlive`); no corpse, ragdoll, or de-spawn; actor remains alive and valid; once-only ally role conversion upon bond.
  - `REG-07.ConsentSequence`: living defeat alone strictly blocks ownership/control (`can_offer_voluntary_bond = False`); central claim console must be shattered (`BreakVerdanceClaim`) before voluntary bond is permitted; voluntary bond converts Verdance to `AlliedCompanion` (210/420 HP, 50% recovery); duplicate bond attempts rejected.
  - `REG-08.ReliefUse`: post-bond Crown relief encounter in gameplay with Verdance companion combat participation (claw attack + area sweep); relief resolved (`relief.resolved`); single dragon authority maintained (no second dragon unlock).
  - `SAVE-10.LivingDefeatBoundaries`: persistence verified across 3 distinct boundaries via `UWyrmSaveSubsystem` (Schema 2): Boundary A (defeated alive, claim unbroken), Boundary B (claim broken, bond pending), Boundary C (companion bonded, relief resolved).
- WP-14 is **PASS in real PIE (REG-09, DRG.TerraceFlightRoute, DRG.TownEntryShrink, REG-12, REG-10, REG-11)**:
  `Content/WYRMFALL/World/Regions/L_Region01.umap` Ally Terrace flight route, authentic 3D flight traversal across the canyon, town entry Heartfold compact shrink into Tidecross, compact cave crawlway mission at `LM-COMPACTCAVE`, unified save/load roundtrips, and optional activity skip preparation verified in live PIE under `UEDPIE_0_L_Region01` via `py -3.12 tools/run_wp14_terrace_cave_proof.py`:
  - `REG-09.LateRescueAndImmediateDragon`: dragon bonded before all worker rescues finished; direct control and riding available immediately on Ally Terrace (`LM-TERRACE`); partial debrief available at Tidecross; full homecoming blocked until Pell, Iven, Sella secured; late rescues complete full homecoming.
  - `DRG.TerraceFlightRoute`: humanoid mounted Verdance in TrueForm on Ally Terrace (`3500, 1000, 850`), takeoff into authentic 3D flight (`MOVE_Flying`, max speed 1600) across canyon to town entry (`LM-TOWNENTRY` at `800, 500, 950`), safe ground landing with slope checks, and dismount. No grounded substitute accepted.
  - `DRG.TownEntryShrink`: town entry buffer enforces Heartfold compact shrink (`CompanionForm`), allowing compact Verdance (capsule diameter 60cm, height 70cm) to accompany player into Tidecross safely without doorframe collision (`60cm < 100cm`, `70cm < 210cm`) or trampling citizens.
  - `REG-12.CompactCave` ("A Smaller Kind of Strength" at `LM-COMPACTCAVE` `1300, -1500, 888`): humanoid staged safely outside cave entrance; player transfers direct control to compact Verdance; compact dragon navigates low crawlway tunnel beneath low ceiling obstacle (`StaticMeshActor` at `1500, -1500, 950`, scaled `2.0, 2.0, 0.5`); defeats cave crawler enemy with compact attacks (9 primary, 6 area sweep); growth check blocked under low crawlway ceiling (`can_change_form(TrueForm)` rejected); wide inner chamber growth check succeeds (`can_change_form(TrueForm)` succeeds); service cache interactable interacted with to commit `cache.recovered` and `cave.service_unlocked` in `UWyrmRegion01Subsystem`; control returned cleanly to staged humanoid waiting outside cave without teleporting through pet hole.
  - `REG-10.PersistentLocalRecovery`: unified save/load roundtrip via `UWyrmSaveSubsystem` (Schema 2) verifying persistence of homecoming, cache recovery, camp piece, wage recovery, and dragon companion state without duplicate rewards or world regeneration.
  - `REG-11.SkipPreparation`: verifies that main quest progression requires zero optional activity/cave facts.

Evidence:

- `Saved/Automation/Scaffold/index.json`
- `Saved/Diagnostics/WP01_terrain_provider_proof.json`
- `Saved/Diagnostics/WP06_progression_proof.json`
- `Saved/Diagnostics/WP07_activities_proof.json`
- `Saved/Diagnostics/WP08_camp_proof.json`
- `Saved/Diagnostics/WP09_dragon_proof.json`
- `Saved/Diagnostics/WP10_flight_proof.json`
- `Saved/Diagnostics/WP11_heartfold_proof.json`
- `Saved/Diagnostics/WP12_region01_proof.json`
- `Saved/Diagnostics/WP12_region01_asset_intake.json`
- `Saved/Diagnostics/WP12_region01_npc_intake.json`
- `Saved/Diagnostics/WP12_region01_source_material_probe.json`
- `Saved/Diagnostics/WP12_region01_fbx_palette_layout.json`
- `Saved/Diagnostics/WP12_region01_fbx_animation_compat.json`
- `Saved/Diagnostics/WP12_region01_normalized_npc_sources.json`
- `Saved/Diagnostics/WP12_region01_normalized_npc_intake.json`
- `Saved/Diagnostics/WP12Region01NpcPreview/report.json`
- `Saved/Diagnostics/WP12Region01NormalizedNpcPreview/report.json`
- `Saved/Diagnostics/WP12_region01_map_composition.json`
- `Saved/Diagnostics/WP12_production_pie_proof.json`
- `Saved/Diagnostics/WP13_boss_and_bond_proof.json`
- `Saved/Diagnostics/WP14_terrace_cave_proof.json`
- `Saved/Automation/Region01/index.json`
- [WP-01 report](WP01_TERRAIN_PROVIDER_PROOF.md)
- [WP-06 report](WP06_PROGRESSION_PROOF.md)
- [WP-07 scoped report](WP07_ACTIVITIES_PROOF.md)
- [WP-08 report](WP08_CAMP_PROOF.md)
- [WP-09 report](WP09_DRAGON_PROOF.md)
- [WP-10 report](WP10_FLIGHT_PROOF.md)
- [WP-11 report](WP11_HEARTFOLD_PROOF.md)
- [WP-12 report](WP12_REGION01_PROOF.md)
- [WP-13 report](WP13_BOSS_AND_BOND_PROOF.md)
- [WP-14 report](WP14_TERRACE_AND_CAVE_PROOF.md)

## Verification corrections

- `AWyrmGeoForgeAdapter` resets `LastRejectionReason` at the start of each submit request.
- `AWyrmBuildingPiece::GetSupportBounds` only provides ground support bounds for `Foundation` pieces.
- `AWyrmGeoForgeAdapter` prunes stale/destroyed weak pointers and validates actors with `IsValid()`.
- `UWyrmBuildingSubsystem::ClearAllPlacedPieces` unregisters pieces from GeoForge adapters before destruction.
- `UWyrmSaveSubsystem` resolves `UWyrmBuildingSubsystem` matching world context consistently across both `CreateSnapshotObject` and `ApplySnapshotObject`.
- Companion growth clearance sweeps above ground datum to prevent false-positive collisions with floor actors.
- `AWyrmDragonCharacter` initializes AbilityActorInfo and AttributeSet in `PostInitializeComponents` and `EnsureAbilitySystemInitialized` so tests and spawned actors have valid attributes prior to Tick.
- All combat damage routes strictly through `UWyrmMeleeAttackAbility::ApplyDamageEffect` rather than standard engine `TakeDamage` to ensure GAS attribute authority.
- `CheckTetherStatus()` is exposed as a `UFUNCTION(BlueprintCallable)` allowing reliable script inspection without calling unexposed `AActor::Tick`.
- `AWyrmDragonCharacter::EndDirectControl` evaluates humanoid distance rather than unconditionally resetting tether status, correctly preserving `LimitReached` upon max tether boundary return.
- `CanLand`, `CanDismount`, and `GetSafeGroundAnchor` trace across both `ECC_WorldStatic` and `ECC_Visibility` channels with fallback ground checks for minimal test environments.
- `HandleMountedDefeat` immediately teleports the rider to the nearest valid ground anchor beneath, resets dragon flight state, and transfers controller possession back to the humanoid, preventing fallen/dropped riders in midair.
- In Unreal Engine Python test probes, C++ methods with out-parameters evaluate cleanly via return value inspection.
- Normal Heartfold requests cannot bypass cooldowns. They stop movement, suppress attacks and controller actions while transitioning, and revalidate clearance immediately before commit.
- `Verdance` is the only enabled dragon rig profile. A different `DragonId` is fail-closed until its own profile and proof exist, rather than silently reusing Green Dragon values.
- The generated Android File Server settings were removed from tracked configuration and archived locally at `Saved/ConfigArchive/AndroidFileServerSettings-2026-09-17.ini`. The archive is ignored and deliberately omits the prior credential; generate a new token if this feature is restored.
- `UWyrmRegion01Subsystem` is a fact/receipt and logical-landmark owner only. It has no persistence I/O; `UWyrmSaveSubsystem` writes/restores its record in current save schema 2 and accepts schema 1 as an empty Region 01 state.
- A primary claim break independently stops Crowncut extraction, and a real control shutdown may also do so without making Rusk an artificial route gate. Full homecoming requires all workers, extraction stopped, accepted bond, a Rusk outcome, relief resolution, and a Tidecross return event.
- In Blender 4.5+, `action_slot = action.slots[0]` must be explicitly assigned when applying imported actions to armatures so animated keyframe tracks evaluate and bake into exported FBXs rather than freezing in rest pose.
- `verify_wp12_normalized_npc_animation_intake.py` explicitly saves generated `USkeleton` objects and dirty packages to disk so skeletal meshes and animation sequences load with valid skeleton references in subsequent sessions.
- `AWyrmRegion01Npc` validates `bRescued` and rejects duplicate rescue interactions for Sella, Pell, and Iven, returning `false` and maintaining single-receipt authority.
- `verify_wp12_production_pie_proof.py` coordinates asynchronous terrain updates across engine ticks via a multi-tick Slate post-tick state machine (`init` -> `wait_dig` -> `tests`), yielding frames while `adapter.has_pending_terrain_edits()` is true so GeoForge mesh/collision and navmesh dirty generation complete before test assertions and save snapshot generation.
- In Unreal Engine Python, `GameplayStatics.get_game_instance(world)` resolves `UGameInstance`, from which `UWyrmBuildingSubsystem` is fetched via `gi.get_subsystem(unreal.WyrmBuildingSubsystem)`, and `execute_placement` out-parameters evaluate cleanly as a tuple `(AWyrmBuildingPiece, ...)` via return value inspection.
- `AWyrmVerdanceBossArena` coordinates boss encounter state transitions (`BossCombat`, `LivingDefeat`, `ClaimBroken`, `CompanionBonded`, `ReliefCombat`, `ReliefResolved`), periodic electrical interference pulses, and relief squad combat without mutating dragon actor role outside authoritative consent methods.
- The canonical fact for relief resolution in `UWyrmRegion01Subsystem` is `relief.resolved` (receipt `region01.relief.resolved`).
- `AWyrmDragonCharacter` maintains dragon actor state, attributes, and event delegates only; fact ledger commits (`verdance.defeated_alive`, `verdance.bond_accepted`) remain owned by `UWyrmRegion01Subsystem` and coordinator actors, preventing duplicate fact commit failures.
- `bHasBondReceipt` in `AWyrmDragonCharacter` is cleared when setting role to `HostileBoss` and exposed through `ResetBondReceipt()` UFUNCTION for reproducible automation without relying on editor property reflection on read-only properties.
- During PIE interaction tests, player positioning must be brought within interaction radii (`InteractionRadius` 250 cm for interactables, 500 cm for voluntary bond) to account for multi-tick gravity and terrain mesh settling.
- In `AWyrmDragonCharacter::CanChangeForm`, non-blocking components on the `ECC_Pawn` collision channel are filtered out to prevent sensor/trigger volumes (such as `AWyrmVerdanceBossArena`'s 1500 cm trigger sphere) from falsely blocking dragon growth.
- In `AWyrmDragonCharacter::CanChangeForm`, `FloorClearanceTolerance` was increased from 2.0 cm to 20.0 cm to prevent subtle ground mesh variation and procedural terrain slopes within the 120 cm True Form radius from registering as overhead collision obstacles.
- In Unreal Engine Python reflection for `can_change_form`, a successful form change returns the empty string `""` while a blocked change returns `None`; probes check `res is not None`.
- In `compose_wp12_region01_map.py`, all existing level actors are cleared prior to spawning to guarantee clean idempotent composition and prevent duplicate actors.
- In `UWyrmSaveSubsystem`, `SaveSnapshotToSlot` and `LoadSnapshotFromSlot` are static methods on the class invoked as `unreal.WyrmSaveSubsystem.save_snapshot_to_slot(...)`.

## Boundaries

- WP-00 remains PARTIAL; RDY-02/03/04 are open.
- WP-02 through WP-05 full proof scripts remain editor-world evidence.
- Physical controller, cook and packaged-game validation remain NOT_RUN.
- Jadefang has source metadata only; it is not an imported, playable, or Heartfold-proven rig.
- WP-12 production Region 01 map placement, composition, and all 8 REG acceptance cases (`REG-01`–`05`, `REG-09`–`11`) are RESOLVED in live PIE (`UEDPIE_0_L_Region01`).
- WP-13 Verdance authored boss, claim, voluntary bond consent sequence, Crown relief combat, and living-defeat persistence boundaries (`REG-06`, `DRG-01`, `REG-07`, `REG-08`, `SAVE-10`) are RESOLVED in live PIE (`UEDPIE_0_L_Region01`).
- WP-14 Ally Terrace flight route, town entry Heartfold shrink, compact cave ("A Smaller Kind of Strength"), late worker rescues with homecoming gating, persistent local recovery, and skip preparation (`REG-09`, `DRG.TerraceFlightRoute`, `DRG.TownEntryShrink`, `REG-12`, `REG-10`, `REG-11`) are RESOLVED in live PIE (`UEDPIE_0_L_Region01`).
- `py -3.12 tools/wyrm.py report` is stale because its old onboarding receipt predates the current source and `Saved/Diagnostics/doctor.json` is absent. Do not use it as current proof.

## Next bounded task

With WP-12, WP-13, and WP-14 complete in live PIE in production map `L_Region01`:
Proceed to the next backlog dependency:
WP-15 (Echo Power Manifestation / Relentless Advance and Combat Evolution),
respecting the project backlog sequence.
Mutable remains creator, GAS remains combat authority, and each subsystem keeps one owner.



