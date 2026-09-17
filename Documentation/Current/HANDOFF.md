# WYRMFALL handoff — September 17, 2026

## Resume here

Continue the v0.2 scaffold in `G:/assets/voxel project`; do not re-plan the game.
Read `AGENTS.md`, `CODEX_START_HERE.md`, `Documentation/Current/STATUS.md`, this
handoff, and only the packet for the next bounded task.

Repository: https://github.com/DocDamage/voxeldragongame

Branch: `main`

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
- WP-12 is **PARTIAL**: the Region 01 fact ledger passed its 13-landmark logical graph, native 1/1, and diagnostic PIE 8/8 proof (including a real Green Dragon bond/direct-control handoff and unified save restore). This is not production map placement; the PIE fixture explicitly used a blank diagnostic map and modeled a future relief fact only to validate the completion predicate.

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
- `Saved/Automation/Region01/index.json`
- [WP-01 report](WP01_TERRAIN_PROVIDER_PROOF.md)
- [WP-06 report](WP06_PROGRESSION_PROOF.md)
- [WP-07 scoped report](WP07_ACTIVITIES_PROOF.md)
- [WP-08 report](WP08_CAMP_PROOF.md)
- [WP-09 report](WP09_DRAGON_PROOF.md)
- [WP-10 report](WP10_FLIGHT_PROOF.md)
- [WP-11 report](WP11_HEARTFOLD_PROOF.md)
- [WP-12 report](WP12_REGION01_PROOF.md)

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

## Boundaries

- WP-00 remains PARTIAL; RDY-02/03/04 are open.
- WP-02 through WP-05 full proof scripts remain editor-world evidence.
- Physical controller, cook and packaged-game validation remain NOT_RUN.
- Jadefang has source metadata only; it is not an imported, playable, or Heartfold-proven rig.
- WP-12 production placement is blocked: the audited import set lacks real quarry, Tidecross/settlement, underworks/cave, broken cart/claim apparatus, and distinct Tamsin, Mara, Pell, Iven, Sella, and Rusk content. Preserve `REG-01`–`05` and `REG-09`–`11` as NOT_RUN until those real interactions exist.
- `py -3.12 tools/wyrm.py report` is stale because its old onboarding receipt predates the current source and `Saved/Diagnostics/doctor.json` is absent. Do not use it as current WP-12 proof.

## Next bounded task

Provide or import approved existing real Region 01 quarry, settlement, cave,
cart/claim, and named-NPC content; then place and validate the actual landmarks
and production `REG` cases. Do not expand into WP-13 before that blocker is resolved.
Mutable remains creator, GAS remains combat authority, and each subsystem keeps one owner.
