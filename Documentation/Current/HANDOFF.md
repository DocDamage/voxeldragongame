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

Historical BOOT-01 passed editor build, bootstrap creation/unchanged rerun, six
native scaffold tests and focused keyboard/mouse PIE: movement/jump, cameras/HUD,
rebinding, pause/input-ignore guards, click rejection and relaunch. No controller
was detected; physical controller validation remains NOT_RUN.

The latest C++ bridge has a real build, six native regressions with GeoForge
loaded and bounded PIE collision/navigation-projection evidence:

- Build: `Saved/ScaffoldLogs/20260915T175726Z_dfb49f75ae60_terrain-nav-final-build.log`.
- Native tests: `Saved/Automation/GeoForgeBridgeFinal/index.json`.
- PIE: `Saved/Diagnostics/GeoForge_PIE_explicit_nav_probe.json`.
- Commands/limits: `GEOFORGE_COMPLETION_INSPECTION.md`, `GEOFORGE_SYNC_BRIDGE_PROBE.md`.

GeoForge edit return/acknowledgment and save capture precede mesh readiness.
Support priming alone is insufficient even for synchronous cross-chunk edits.
The tested explicit visual-refresh plus native nav-data submission updated
collision, exposed lower nav and removed obsolete lower nav on refill in a fixed
three-chunk synchronous fixture. This is not a general completion callback,
streaming/action-revision integration, traversable-cave proof or selected provider.

## Actual owner implementation inspection

`WP00_OWNER_IMPLEMENTATION_INSPECTION.md` records native graph exports and traced
execution/data pins for eight EBS and five Waterline Blueprints. Inspected source
hashes stayed unchanged; vendor graph text remains under Saved.

- EBS owns resource balances; its inspected batch deduction has no rollback.
  Its demo controller also contains movement, damage and interaction behavior.
- EBS owns a save slot and destructive actor-reconstruction load flow. Do not
  adopt its controller wholesale or introduce a second inventory/save owner.
- Waterline has visual and physical side effects. Its inspected height functions
  do not query edited voxel terrain. Reconcile one logical wet-state/physics
  authority with GeoForge. Standalone BP_Buoyancy contains disconnected event
  paths; node presence is not runtime proof. UWS is unselected/uninspected.

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
platform/privilege skips, 8.752 seconds. Commands:

```powershell
py -3.12 tools/wyrm.py verify
py -3.12 tools/wyrm.py test
```

Evidence: `Saved/Diagnostics/WP00_asset_review_verify.txt` and
`WP00_asset_review_tests.txt`. These are not Unreal compilation/gameplay tests.
No C++ changed after the bridge validation; subsequent work used editor Python.

## Next bounded task and precise blocker

Inspect **Advanced Inventory System AAA** implementation, then reconcile capacity,
overflow, finite terrain rewards, duplicate protection and persistence with the
existing EBS/GeoForge seams. Inventory authority remains unselected.

Recorded candidate:
`D:/Unreal/UE_5.8/Engine/Plugins/Marketplace/Untitled5ad389823e25V1/Source/AdvancedInventorySystem`.
Its Public/Private source read failed with Windows device error 433. No matching
inventory descriptor was found under C: UE Marketplace. Evidence:
`Saved/Diagnostics/WP00_inventory_source_blocker.json`.

The user was asked for another readable local copy/archive; none was supplied
before this handoff. If provided, inspect that exact implementation. Do not
repeatedly access the failing device, install a substitute, or invent inventory/
save frameworks to bypass missing source. Keep this dependent step blocked if
the input remains unavailable. Independent remaining asset animation/rig/provenance
checks may proceed within WP-00.

Stop at an accurate readiness/ownership decision. Only then implement ONE eligible
WP-01 proof following its packet and complete preserved acceptance: traversable
dig/add/refill, navigation and stale-route cancellation, finite depletion/yield,
full-bag overflow, protected/occupied-fill cases, coherent save/quit/reload,
water edges and representative edit/streaming stress. A small floor/preview
cannot pass G1.

## Constraints and unrun acceptance

Mutable remains creator; GAS remains combat authority. Keep one owner per
subsystem, supplied real assets and preserved dragon/Heartfold/horror/vehicle/
colony requirements. Do not broaden the scaffold or silently substitute providers.

WP-00 remains PARTIAL. WP-01 and full RDY-02/03/04 remain NOT_RUN. No production
terrain, game save, inventory, equipment, Mutable recipe or playable dragon exists.
Latest asset work did not run PIE, animation playback, collision, whole-game
save/reload, water-edge/stress or cook. Earlier BOOT-01 and bounded bridge passes
retain only their recorded scopes.

Archives, imported vendor packages, Saved diagnostics, renders and machine
settings are excluded from Git. A fresh GitHub checkout will not contain them.
Preview imports/materials remain local under `/Game/WYRMFALL/Development/Intake/WP00`;
no production map was saved.
