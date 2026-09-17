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
- Native automation passed 37/37 source-declared tests (including 5 dragon locomotion, combat, direct control, and save tests).
- Portable verification passed; 124 tooling tests passed with two expected skips.
- WP-01 real PIE passed 7/7 terrain groups.
- WP-06 real PIE passed 6/6 ranged/progression groups.
- WP-07 scoped real PIE passed 8/8 activities groups.
- WP-08 real PIE passed 7/7 supported camp & storage groups (`ACT-06..10`, `WRLD-08..09`).
- WP-09 real PIE passed 5/5 green dragon locomotion, combat & direct control groups (`DRG-01..04`, `SAVE-08`):
  authentic modular skeletal assets verified, living defeat (1800 HP -> 0 HP DefeatedAlive) & one-way bond (420 Max HP, 210 initial HP), autonomous companion orders & GAS combat (24 raw primary, 18 area with 6s cooldown), direct control possession with humanoid body anchoring, 150m tether limit & waiting humanoid damage interrupts, and unified save schema roundtrip.

Evidence:

- `Saved/Automation/Scaffold/index.json`
- `Saved/Diagnostics/WP01_terrain_provider_proof.json`
- `Saved/Diagnostics/WP06_progression_proof.json`
- `Saved/Diagnostics/WP07_activities_proof.json`
- `Saved/Diagnostics/WP08_camp_proof.json`
- `Saved/Diagnostics/WP09_dragon_proof.json`
- [WP-01 report](WP01_TERRAIN_PROVIDER_PROOF.md)
- [WP-06 report](WP06_PROGRESSION_PROOF.md)
- [WP-07 scoped report](WP07_ACTIVITIES_PROOF.md)
- [WP-08 report](WP08_CAMP_PROOF.md)
- [WP-09 report](WP09_DRAGON_PROOF.md)

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

## Boundaries

- WP-00 remains PARTIAL; RDY-02/03/04 are open.
- WP-02 through WP-05 full proof scripts remain editor-world evidence.
- Physical controller, cook and packaged-game validation remain NOT_RUN.

## Next bounded task

Reconcile WP-00 readiness or begin next scheduled feature workpackage (e.g. WP-10 Dragon flight / riding / Heartfold or WP-18 Hovercar / vehicle locomotion).
Mutable remains creator, GAS remains combat authority, and each subsystem keeps one owner.
