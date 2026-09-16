# WYRMFALL handoff — September 16, 2026

## Resume here

Continue the v0.2 scaffold in `G:/assets/voxel project`; do not re-plan the game.
Read `AGENTS.md`, `CODEX_START_HERE.md`, `Documentation/Current/STATUS.md`, this
handoff, and only the packet for the next bounded task.

Repository: https://github.com/DocDamage/voxeldragongame

Branch: `main`
Verified parent checkpoint: `55172d0a36f787c5d7f53deff67dec17e803b390`

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

- Editor target compiled with `-NoUBA -NoPCH`.
- Native automation passed 27/27 source-declared tests.
- Portable verification passed; 124 tooling tests passed with two expected skips.
- WP-01 real PIE passed 7/7 terrain groups.
- WP-06 real PIE passed 6/6 ranged/progression groups.
- WP-07 scoped real PIE passed 8/8 groups: ten supplied assets loaded;
  water/wet/swim and basin protection; fishing commit, damage cancellation and
  full-bag rejection; campfire crafting; ingredient-slot capacity; food-buff
  refresh, replacement and snapshot restore.

Evidence:

- `Saved/Automation/Scaffold/index.json`
- `Saved/Diagnostics/WP01_terrain_provider_proof.json`
- `Saved/Diagnostics/WP06_progression_proof.json`
- `Saved/Diagnostics/WP07_activities_proof.json`
- [WP-01 report](WP01_TERRAIN_PROVIDER_PROOF.md)
- [WP-06 report](WP06_PROGRESSION_PROOF.md)
- [WP-07 scoped report](WP07_ACTIVITIES_PROOF.md)

## Verification corrections

- Terrain completion waits for GeoForge render/navigation queues; yield reflects
  actually removed filled cells.
- Inventory transfers and crafting transactions preflight capacity and roll back
  unexpected partial mutations.
- Save/load validates terrain failure before character mutation and saves
  food-buff base stats without double applying.
- Food buffs remove MaxFocus and Power modifiers on expiry; Focus restores after
  the active maximum.
- Fishing clears its reel timer on commit and cancels only for positive damage.
- GeoForge discovers water volumes independent of BeginPlay order.
- The crafting enum uses `EWyrmCraftingStationType` to avoid a Python reflection
  collision with the station actor.

## Boundaries

- WP-00 remains PARTIAL; RDY-02/03/04 are open.
- WP-02 through WP-05 full proof scripts remain editor-world evidence.
- Combined bound-GeoForge plus character/inventory save remains NOT_RUN.
- WP-07 has no current task packet. Its scoped fixture is proven, but production
  interaction/UI, animation, audio playback, water rendering, final fishing-rod
  art, production-map integration and full acceptance remain open.
  `SM_FishingRod` is currently a supplied spear fixture proxy.
- Physical controller, cook and packaged-game validation remain NOT_RUN.

## Next bounded task

Finish WP-00 RDY-02/03/04 and reconcile remaining stale readiness text. Preserve
the verified WP-07 fixture without expanding it until a task packet defines the
full acceptance boundary. Mutable remains creator, GAS remains combat authority,
and each subsystem keeps one owner.
