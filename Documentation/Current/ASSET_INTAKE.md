# Approved-content intake and missing-input record

The only uploaded bytes used in this task were the documentation ZIP. Its references report assets, but this scaffold does not pretend those paid/model/plugin archives were imported. Preserve any already-known asset locations; do not ask the user to repeat them or scan unrelated personal folders.

Use `tools/wyrm.py inventory --root "actual approved folder or ZIP"` for a bounded read-only list. It lists archive metadata without extracting, installing, importing, moving assets or certifying license entitlement. Update Config/IntegrationReadiness.json with actual evidence, not inferred availability from a product title.

| Gate | Locate first | Required evidence |
|---|---|---|
| BOOT-01 | Actual UE installation, C++ compiler/SDK, required built-in plugins | Build.version, descriptors, UBT/editor logs |
| G0/G1 | One available terrain candidate, real materials/resource visual | Version/API/owner/license record; dig/add/collision/nav/save proof |
| G2 | Actual modular voxel humanoid parts, compatible animations | Mutable recipe generation, animation, revision-safe changes, cooked test |
| G3 | Weapon, two real animated enemy roles, existing UI | Actual combat presentation and grant ownership |
| G4/G4-H | Green Dragon/Verdance assets, actual rig and rider animation | Living defeat/bond, AI/direct/riding/flight, compact combat and clearance |
| E1 | Counselor model, weapon/animation/audio/VFX coverage | Real encounter and permanent usable Relentless Advance |
| V1/Z1 | Hovercar/seat/collision data and city/colony assets | Occupancy, flight/landing, safe transfers and coherent save |

Terrain shortlist remains the supplied one: Voxel Plugin candidate, GeoForgeRuntime, draquel/VoxelWorlds, UnrealSandboxTerrain or a narrowly useful referenced alternative. This is not permission to fetch/integrate all of them. Stop comparing after one candidate passes the bounded requirement set. No engine change, purchase or downgrade by inference.

Import through Unreal's supported content workflow. Leave original vendor mount roots intact and author project-facing variants under Content/WYRMFALL. Keep proprietary archives out of public Git, and decide LFS/licensing policy before committing large binaries. This archive configures binary diff handling but does not install Git LFS or publish assets.

Missing roles require precise requests: exact product/role, required mesh/rig/clip/material or runtime API, inspected paths and the blocker. “Need more assets” is not a useful report. An unlocated flying car does not block validating the humanoid host.
