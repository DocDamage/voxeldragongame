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
- Native automation passed 32/32 source-declared tests (including 5 camp/storage foundation tests).
- Portable verification passed; 124 tooling tests passed with two expected skips.
- WP-01 real PIE passed 7/7 terrain groups.
- WP-06 real PIE passed 6/6 ranged/progression groups.
- WP-07 scoped real PIE passed 8/8 activities groups.
- WP-08 real PIE passed 7/7 supported camp & storage groups (`ACT-06..10`, `WRLD-08..09`):
  genuine assets verified, atomic placement, zero-side-effect rejection validation,
  bag $\leftrightarrow$ chest storage transfer with identity preservation, demolition
  overflow recovery bundles with zero item loss, camp persistence & companion growth
  clearance check, and ground support excavation locking.

Evidence:

- `Saved/Automation/Scaffold/index.json`
- `Saved/Diagnostics/WP01_terrain_provider_proof.json`
- `Saved/Diagnostics/WP06_progression_proof.json`
- `Saved/Diagnostics/WP07_activities_proof.json`
- `Saved/Diagnostics/WP08_camp_proof.json`
- [WP-01 report](WP01_TERRAIN_PROVIDER_PROOF.md)
- [WP-06 report](WP06_PROGRESSION_PROOF.md)
- [WP-07 scoped report](WP07_ACTIVITIES_PROOF.md)
- [WP-08 report](WP08_CAMP_PROOF.md)

## Verification corrections

- `AWyrmGeoForgeAdapter` resets `LastRejectionReason` at the start of each submit request.
- `AWyrmBuildingPiece::GetSupportBounds` only provides ground support bounds for `Foundation` pieces.
- `AWyrmGeoForgeAdapter` prunes stale/destroyed weak pointers and validates actors with `IsValid()`.
- `UWyrmBuildingSubsystem::ClearAllPlacedPieces` unregisters pieces from GeoForge adapters before destruction.
- `UWyrmSaveSubsystem` resolves `UWyrmBuildingSubsystem` matching world context consistently across both `CreateSnapshotObject` and `ApplySnapshotObject`.
- Companion growth clearance sweeps above ground datum to prevent false-positive collisions with floor actors.

## Boundaries

- WP-00 remains PARTIAL; RDY-02/03/04 are open.
- WP-02 through WP-05 full proof scripts remain editor-world evidence.
- Physical controller, cook and packaged-game validation remain NOT_RUN.

## Next bounded task

Reconcile WP-00 readiness or begin next scheduled feature workpackage (e.g. WP-09 Dragon companion flight / riding mechanics).
Mutable remains creator, GAS remains combat authority, and each subsystem keeps one owner.
