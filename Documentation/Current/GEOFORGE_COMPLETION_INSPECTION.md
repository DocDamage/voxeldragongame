# WP-00 — GeoForge completion and save inspection

September 15, 2026. Workspace `G:\assets\voxel project`, clean initial `main`
at `32407491f6cd9bb34848ffb6fb59a2d82be14904`. UE 5.8.2, CL 56702186,
`C:\Program Files\UE_5.8`; GeoForgeRuntime 5.0.0, compatible BuildId 55116800.

**Result: a completion-barrier assumption was disproved in native Unreal.**
`DigSphere` followed by `PrimeTerrainSupportAtWorldLocation` can return with
persisted edits ahead of the mesh, including with asynchronous generation disabled
when an edit crosses chunks. This is a blocker for using that sequence as the
WYRMFALL completion contract, not a finding that GeoForge cannot eventually
complete edits. GeoForge remains the first candidate, not a selected authority.
WP-00 remains PARTIAL; no eligible complete WP-01 integration was established.

**Subsequent continuation:** [the synchronous bridge probe](GEOFORGE_SYNC_BRIDGE_PROBE.md)
records a compiled development helper and actual PIE dig/fill collision plus
navigation-projection evidence using explicit visual/nav refresh. It preserves
this report's finding about priming alone. Validation/next-task statements below
describe the initial inspection; the linked report records the later checks.

## Scope and evidence

Read the requested current reports, WP-00/WP-01 packets, preserved asset/plugin
records, G0/G1 assignment, editable-world rules, save/control contract and applicable
RDY/WRLD/SAVE cases. Inspected the installed implementation and engine collision
implementation without modifying or copying vendor code into the repository.

Added `tools/unreal/probe_geoforge_completion.py`, a disposable native editor
diagnostic. It creates an unsaved blank map and uses the already imported real
dirt material at `/Game/WYRMFALL/Development/Intake/WP00/Dirt/dirt-1/Materials/palette`.
There is no new player, inventory, save coordinator or terrain adapter. The plugin
is enabled only on the probe command line. The project descriptor is unchanged.

The probe inspects procedural mesh vertices/triangles, provider queue counts and
the real provider save snapshot. It does not render a material preview, trace
collision, run a game world, traverse a cave or save a gameplay slot. Python
reflection reads actual native objects; it does not emulate Unreal.

Installed source/binary/script paths, sizes and hashes are recorded in
`Saved/Diagnostics/GeoForge_semantics_manifest.json`. This identifies the inspected
files; it does not attest that the vendor DLL was compiled from those source bytes.

## Exact implementation findings

Paths below are relative to the installed plugin's `Source/GeoForgeRuntime/`.
Line numbers identify this recorded version, not a future vendor update.

| Concern | Inspected implementation | Consequence for WYRMFALL |
|---|---|---|
| Edit acceptance | `Private/GeoForgeInfiniteTerrainActor.cpp:12714`, `:21592`: standalone dig calls `ApplySphereEdit`; occupancy and persisted cell state change before mesh work is queued; only one queued rebuild is processed immediately (`:21858`). | A returned call or changed cell count does not mean geometry is ready. Sphere operations are cell-occupancy edits rendered through Marching Cubes, not a removed-volume measurement. |
| Client notification | `Private/GeoForgeTerrainEditProxyActor.cpp`, `ServerRequestSphereEdit_Implementation`: sends the result directly after applying cells. Actor `:11327` forwards the reply through `OnClientTerrainEditRequestCompleted`. | Client request acknowledgement, not collision/nav completion; standalone direct dig does not use this client-request path. Do not use it to pay rewards. |
| Synchronous support | Actor `:22667`: an existing synchronous full mesh is reused if generation is not in flight. Queued/in-flight **mesh rebuilds** are not checked there. `EnsureChunkExists` at `:23062` can disable async cooking, but returns early for an already built mesh. | Support priming is not an edit flush. Native reproduction below covers async and synchronous cross-chunk cases. |
| Mesh jobs | Actor `:15971`, `:16151`, `:16245`: revisioned queues, worker/apply stages and async generation setting. `RefreshLoadedChunkVisualState` at `:19304` queues loaded meshes and processes a budget. | There is real internal job/revision machinery. Public runtime render statistics are aggregate diagnostics, not an operation-specific collision receipt. A bounded bridge remains to be established. |
| Collision | Chunk creation at actor `:23707`/`:23733` uses ProceduralMeshComponent async cooking. Installed engine `ProceduralMeshComponent.cpp:1034` only cooks asynchronously in a game world; `:1081` handles completion/failure and replaces physics state. Its cook callback/queue are private in the header. | Editor-world mesh results cannot certify runtime async physics completion. A visible mesh, zero mesh-worker count or non-null body setup alone is insufficient. |
| Navigation | Actor `:23802` sets `CanEverAffectNavigation`; `:22534` manages navigation invokers. Neither reports per-edit nav readiness. Installed `NavigationSystem.h:444` exposes the engine generation-finished delegate; `:530` exposes a build/lock query. | A project bridge must correlate terrain revision, completed collision, affected nav rebuild and path invalidation. Existing controller path queries do not implement this. No nav pass. |
| Save capture | Actor `:14039`: version 10 payload with seed, dimensions, cell size, world shape, compatibility identity, fingerprint, edited cells, authored cells, fluid cells, planet overrides and removed decorations. No wait for outstanding mesh/collision jobs. | Native capture already includes 72 edits while geometry is pending. Coordinator must capture only a coherent transaction boundary. |
| Persistence switch | Actor `:11274`: `bPersistEditedCellsAcrossStreaming` or network persistence controls the planar edit map. `BuildTerrainSaveData` includes planar edits only through this condition. | Single-player proof must explicitly retain persistence; the method's presence alone does not guarantee saved edits. |
| Save apply | Actor `:14133`: checks supported version (4–10), seed/dimensions/cell size, versioned identity/fingerprint, world shape and bounded entries; restores state, schedules streaming/rebuilds, primes a preferred source and returns true (`:14423`). | Success is payload acceptance, not whole restored-region collision/nav readiness. Gameplay input must remain gated until actual arrival readiness. |
| Compatibility | Actor `:10455`: CRC of normalized runtime settings and selected asset **paths**, with legacy/default-field exceptions. | Not an asset-content or complete vendor-binary integrity hash. Required content/version checks still belong to the one save coordinator. |
| Slot helper | `Private/GeoForgeTerrainBlueprintLibrary.cpp:1147`: creates `UGeoForgeTerrainSaveGame`, captures terrain and calls `SaveGameToSlot`; load forwards to apply. | Reuse the payload inside the single project save owner. Do not introduce an independent terrain slot beside inventory/depletion saves. No full-game atomicity or restart proof established. |

## Ownership conflicts inspected

GeoForge `ApplySphereEdit` calls `HandleDestroyedCell` before geometry completion.
That implementation (`:26075`) supports configured dig-time actor spawns and
disappearing, physics/fading, follow-player or custom actor drops. Those are not
WYRMFALL's finite deposit ledger, stable item instances or once-only reward receipts.
Any future resource proof must explicitly reconcile/disable those side effects
through the selected owner; do not pay again from input or the client callback.

GeoForge also owns optional water overlays, fluid state, localized water volumes
and `IsWorldLocationInsideWater` (`:14454`). This is an actual potential conflict
with the preferred Waterline Pro 6 candidate, not merely a material feature.
No second water owner was activated. Its water semantics were inspected only at
this terrain boundary; swim/fishing/bounded-water behavior remains NOT_RUN.

Easy Building System's actual Blueprint graph behavior, resource transfers and
save implementation have **not** been inspected in this session. Its located
components and save packages remain the previously documented candidate inputs.
Inventory/equipment/interaction ownership, full Waterline inspection, enemy/weapon
opening, knight scale/material completion and assembled Green Dragon visuals remain
open. RDY-02/03/04 stay NOT_RUN. This report does not turn those prior filename or
sample findings into completed readiness.

## Native reproduction

All probes use seed 1337, finite planar bounds, 16×16×16-cell chunks, 100 cm cells,
ground level 8, Marching Cubes, collision and persistence enabled. The diagnostic
disables automatic editor rebuild/preset application and generated caves/trees.
This tiny extent isolates an API boundary; it is not representative-region proof.

| Probe | Before dig | After dig and immediate support prime | Result |
|---|---|---|---|
| One chunk, async generation | 0 pending jobs, 0 saved cells, 2,928 vertices | 72 saved cells, 1 mesh job in flight; identical mesh hash and vertex count | Priming does not flush the edit. |
| One chunk, synchronous control | 0 pending jobs, 0 saved cells, 2,928 vertices | 72 saved cells, 0 pending jobs; changed mesh, 3,774 vertices | Narrow mesh-update control succeeds; no physics/nav/gameplay pass. |
| Three chunks along X, synchronous generation, boundary edit | Initial neighbor seams drained; 0 pending jobs, 0 saved cells, 7,770 vertices | 72 saved cells, 2 queued rebuilds; identical mesh hash and vertex count | Disabling async generation alone does not make priming an edit barrier. |

The boundary center is `(1550, 850, 750)` cm with radius 250 cm; support priming
covers horizontal radius 1. All three procedural components are synchronous.
Initial seam work is explicitly drained with `RefreshLoadedChunkVisuals` and a
64-rebuild budget before the probe requires zero baseline work. Dig still processes
only one queued rebuild immediately; the following prime leaves two pending.

Reports under `Saved/Diagnostics`: `GeoForge_completion_probe.json`,
`GeoForge_completion_sync_control.json`, `GeoForge_completion_boundary_probe.json`.
Their hashes/counts concern mesh data, not rendered screenshots or collision traces.
No latency, VRAM, save payload disk size, traversal or performance claim is made.

Exact invocation from the project root (through `wyrm_process.execute_logged`,
300-second timeout, logs and command receipts under `Saved/ScaffoldLogs`):

```powershell
& 'C:\Program Files\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' 'G:\assets\voxel project\WYRMFALL.uproject' -EnablePlugins=GeoForgeRuntime -unattended -nop4 -nosplash -nosound -nullrhi '-ExecutePythonScript=G:/assets/voxel project/tools/unreal/probe_geoforge_completion.py' -stdout -FullStdOutLogOutput
```

Add `-WyrmGeoForgeSyncProbe` for the synchronous control. Add both
`-WyrmGeoForgeSyncProbe -WyrmGeoForgeBoundaryProbe` for the cross-chunk reproduction.
Always run in a fresh commandlet process, not an editor containing unsaved work.
Read the JSON status; a process exit of 0 alone is not a diagnostic success.

Successful settled-boundary receipt:
`20260915T170347Z_9fad4affdf60_geoforge-boundary-settled.log/.json` (exit 0).
Earlier single-chunk confirmations:
`20260915T165940Z_0815228d1f2a_geoforge-completion-probe.log/.json` and
`20260915T170140Z_2d171b78451f_geoforge-sync-control.log/.json`.
Final-script reruns both exited 0 with the expected JSON results:
`20260915T170516Z_5c2f880a44bf_geoforge-async-final.log/.json` and
`20260915T170528Z_57c62340605e_geoforge-sync-final.log/.json`.

Two initial instrumentation attempts used unavailable Python names: the mesh
library is exported as `ProceduralMeshLibrary`, and save capture is exposed via
`GeoForgeTerrainBlueprintLibrary`, not as a Python actor method. Their ERROR
reports are preserved as `GeoForge_completion_probe_attempt1/2.json`; neither
was counted as a pass despite editor exit 0. The first boundary attempt had
pre-existing seam work; `GeoForge_completion_boundary_unsettled.json` retains it
and is superseded by the zero-baseline reproduction above.

## Changed files and portable checks

- `tools/unreal/probe_geoforge_completion.py`: native diagnostic/reproduction.
- `Documentation/Current/GEOFORGE_COMPLETION_INSPECTION.md`: this report.
- `Documentation/Current/STATUS.md` and `WP-00_SCOPED_READINESS.md`: current
  completion gap, preserved limitations and corrected stale repository statement.
- `Config/IntegrationReadiness.json`: candidate probe evidence; authority remains
  null, WP-00 partial, WP-01 and unexecuted acceptance cases NOT_RUN.
- `VALIDATION.md`: separates latest probes from historical BOOT-01 evidence.

`py -3.12 tools/wyrm.py verify` passed; output is
`Saved/Diagnostics/WP00_completion_verify.txt`. `py -3.12 tools/wyrm.py test`
ran 124 tests in 8.996 seconds, OK with two platform/privilege skips; output is
`Saved/Diagnostics/WP00_completion_tests.txt`. These are portable checks, not
Unreal compilation. `git -c safe.directory="G:/assets/voxel project" diff --check`
passed. No commit, push or global Git setting change was made.

Unreal appended AndroidFileServer settings during the commandlet runs. The file
was backed up as `Saved/Diagnostics/DefaultEngine_after_completion_probes.ini`;
after verifying that the remainder exactly matched HEAD, only that appended
section was removed and the original tracked bytes restored. No vendor asset,
engine setting, plugin installation or production map was edited.

## Acceptance boundary and next task

WRLD-01/02/04/05/12 and SAVE-01/02/03/04/06/07 remain **NOT_RUN** as complete
acceptance cases. No C++ was modified; editor compilation, scaffold automation,
bootstrap rerun, PIE, game save/restart and cook were not run in this session.
Historical BOOT-01 evidence remains separate. Installation recovery is not repeated.

**Next bounded task — WP-00 completion bridge:** establish a supported way to
wait for every affected chunk's geometry and actual game-world collision, then
correlate runtime navigation readiness. Start from this cross-chunk reproduction;
consider a bounded synchronous bridge or a revision-aware vendor hook, without
assuming either is already valid or switching providers. Scope project changes to
terrain diagnostics/the existing seam; inspect the remaining owner implementations
before resource/save integration. Compile any changed C++ against installed headers,
run focused native tests and PIE. Stop on a verified bridge or a precise remaining
API blocker; only then select GeoForge and proceed to the full WP-01 proof.
