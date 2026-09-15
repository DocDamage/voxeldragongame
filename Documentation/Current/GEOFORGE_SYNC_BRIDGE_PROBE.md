# GeoForge synchronous collision/navigation bridge probe

September 15, 2026. UE 5.8.2 CL 56702186 at `C:\Program Files\UE_5.8`;
GeoForgeRuntime 5.0.0. Workspace `G:\assets\voxel project`.

**Outcome:** a bounded synchronous sequence updated cross-chunk collision and
runtime navigation in actual PIE. The earlier finding that support priming alone
is not completion remains valid. This is a diagnostic bridge proof, not G1,
a selected terrain authority, a performance result or completed WP-00 readiness.

## Git action and scope

At the user's request, committed the previous inspection/probe/report work as
`c4b447c` (`Inspect GeoForge completion semantics and reproduce priming gap`) and
pushed `main` to `origin/main` successfully. Used per-command
`git -c safe.directory="G:/assets/voxel project"`; no global Git change.
Continuation changes described here were authored afterward and remain local.

The task continued from the documented completion barrier. It added no terrain
provider dependency, inventory, rewards, water owner or save coordinator. GeoForge
was enabled only by command line. No project descriptor or vendor code changed.

## Implemented diagnostic sequence

1. Use the existing three-chunk finite planar fixture, real imported dirt material,
   100 cm cells and 16×16×16-cell chunks. Disable async generation and automatic
   preset/editor rebuild, prime all three chunks, and finish initial seam work.
2. In the real PIE world, disable the terrain actor's tick for the measured sequence.
   This prevents streaming/concurrent terrain work from being mistaken for an
   operation-specific result. This restriction is part of the proof's scope.
3. Dig across X=1600 cm, call support prime, and inspect all queued work and three
   vertical collision rays. Then call `RefreshLoadedChunkVisuals` with a sufficient
   **fixture-specific** budget (64), and require zero outstanding mesh work.
4. After actual synchronous collision changes, call the engine's public
   `FNavigationSystem::UpdateComponentData` for registered navigation-relevant
   candidate components on the terrain actor. Await the engine build/dirty/lock
   query and confirm a navigation projection on the target surface.
5. Fill the edited volume, refresh geometry/collision/navigation, and require that
   an ordinary nearby navigation surface remains while the old lower projection
   disappears. A small filled mound does not have to offer a walkable top.

`UWyrmTerrainDiagnostics` is development instrumentation over existing engine APIs.
Its refresh return value is the number of submitted components (11 here), **not**
completion or a count of changed terrain chunks. Its navigation query wrappers
avoid Python accessing a `NavigationSystemV1` class default object, whose native
class requires a World outer. Outside editor builds the helpers fail closed.
No capability flag or reward was granted from the submission count.

## Actual native observations

Final world: `/Temp/UEDPIE_0_Untitled_1.Untitled`; one GeoForge actor. Recast runtime
generation was Dynamic. Nav bounds half-extents were `(3000,1500,1500)` cm.
Actor navigation participation was true. Requested editor window: 1280×720,
D3D12 rendering; no frame-rate, VRAM or representative-region measurement.

Three downward visibility-channel complex collision rays at Y=850 cm:

| Stage | Z at X=1550 | Z at X=1450 | Z at X=1650 | Mesh work remaining |
|---|---:|---:|---:|---|
| Before edit | 900 | 900 | 900 | None |
| Dig plus support prime | 500 | 525 | **900** | Two queued rebuilds |
| Explicit visual refresh | 500 | 525 | 525 | None |
| Fill plus visual refresh | 1000 | 975 | approximately 975 | None |

All reported hits belonged to the PIE terrain actor's native procedural mesh
components. The partial prime result demonstrates why checking one ray or one
chunk cannot establish completion for a cross-chunk edit.

Navigation projections after the explicit native update:

- Baseline target `(800,850,900)` → `(800,850,910)`.
- Newly dug target `(1550,850,500)` → `(1550,850,560)`, inside the specified
  `(40,40,60)` cm query extent. This is a new lower surface, not the old Z=910 nav.
- After refill, the baseline point still projected at Z=910 and the old lower
  query returned **no projection**.

The observer waited approximately 0.188 / 0.047 / 0.047 seconds for these three
conditions in this run. These are single-run observer intervals, not production
latency budgets or a stress/performance pass. The final log has no Python errors
or handled ensure. Routine fixture warnings remain, including no PlayerStart,
empty component bounds during initial creation and NavMesh tile-size recreation.
This test does not validate character placement or visual/material completion.

Evidence: `Saved/Diagnostics/GeoForge_PIE_explicit_nav_probe.json`, status
`PASS_SCOPED_PIE_COLLISION_AND_NEW_SURFACE_NAV_PROJECTION`;
`Saved/Diagnostics/GeoForge_completion_refresh_probe.json` records the editor
fixture. File identities are in `Saved/Diagnostics/GeoForge_bridge_manifest.json`.
Generated maps were unsaved and no paid assets were added to Git.

## Commands and validation

All native commands ran through `wyrm_process.execute_logged` with per-attempt
logs/command receipts under `Saved/ScaffoldLogs`.

```powershell
& 'C:\Program Files\UE_5.8\Engine\Build\BatchFiles\Build.bat' WYRMFALLEditor Win64 Development '-Project=G:\assets\voxel project\WYRMFALL.uproject' -WaitMutex -NoHotReloadFromIDE -NoEngineChanges -NoUBA -NoPCH
```

Final build PASS, 18.56 seconds: `20260915T175726Z_dfb49f75ae60_terrain-nav-final-build.log/.json`.
An earlier helper build also passed; it is not substituted for this final build.

```powershell
& 'C:\Program Files\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' 'G:\assets\voxel project\WYRMFALL.uproject' -EnablePlugins=GeoForgeRuntime -unattended -nop4 -nosplash -nosound -nullrhi '-ExecCmds=Automation RunTests WYRMFALL.Scaffold' '-TestExit=Automation Test Queue Empty' '-ReportExportPath=G:\assets\voxel project\Saved\Automation\GeoForgeBridgeFinal' -stdout -FullStdOutLogOutput
```

All six source-declared tests succeeded: AttributeBaseClamps, AttributeInputClamps,
NoImplicitTerrainSupport, TagRegistration, TerrainNumericBoundaries and
TerrainRequestValidation. `assess_automation` verified the complete source-declared
selection and Success states. These are scaffold regressions; the PIE probe
exercises the new helper. Report: `Saved/Automation/GeoForgeBridgeFinal/index.json`;
log: `20260915T175745Z_5981c67beae2_geoforge-bridge-final-tests.log/.json`.

```powershell
& 'C:\Program Files\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe' 'G:\assets\voxel project\WYRMFALL.uproject' -EnablePlugins=GeoForgeRuntime -WyrmGeoForgeSyncProbe -WyrmGeoForgeBoundaryProbe -WyrmGeoForgeRefreshProbe -WyrmGeoForgeNavProbe -WyrmGeoForgeExplicitNavRefresh -unattended -nop4 -nosplash -nosound -windowed -ResX=1280 -ResY=720 "-ExecCmds=py exec(open('G:/assets/voxel project/tools/unreal/probe_geoforge_pie_collision.py').read())" -stdout -FullStdOutLogOutput
```

Final PIE PASS for the stated assertions; process exit 0:
`20260915T175816Z_b9d9fe1718de_geoforge-pie-final-bridge.log/.json`.
The script begins/ends real PIE through LevelEditorSubsystem and exits its fresh
editor. Do not run it in an editor holding unsaved user work. JSON status and
actual log evidence must be read; process exit alone is insufficient.

Portable checks: `py -3.12 tools/wyrm.py verify` PASS;
`py -3.12 tools/wyrm.py test` ran 124 tests in 9.109 seconds, OK with two
platform/privilege skips. Outputs: `Saved/Diagnostics/WP00_bridge_verify.txt` and
`Saved/Diagnostics/WP00_bridge_tests.txt`. `git -c safe.directory="G:/assets/voxel project" diff --check`
passed. These checks do not substitute for the native build/automation/PIE above.

After the final editor exited, its appended AndroidFileServer settings were backed
up to `Saved/Diagnostics/DefaultEngine_after_bridge_probes.ini`. The remaining
content was checked against HEAD before restoring the original tracked file bytes.
No intentional engine/project configuration change was retained.

## Failed/intermediate attempts retained

- Initial trace formatting attempted `GameplayStatics.break_hit_result`, which
  Python does not expose; corrected to `HitResult.to_tuple`. Raw ERROR report:
  `GeoForge_refresh_trace_api_error.json`.
- A nested-double-quoted `-ExecCmds` invocation executed only `py`; it did not run
  the probe. Its task-owned editor was closed and invocation corrected to inline
  `exec(open('...').read())`. No pass attributed to that run.
- Without explicit navigation-data submission, baseline projection timed out
  despite Dynamic Recast, valid bounds, terrain navigation enabled and no build
  pending. Reports: `GeoForge_PIE_nav_baseline_timeout.json` and
  `GeoForge_PIE_nav_probe.json`.
- Early direct Python calls on NavigationSystemV1 emitted a class-default-object
  ensure. Final C++ wrappers removed it; the final run was inspected for recurrence.
- An overly strong refill check demanded navigation on the mound top and timed
  out. `GeoForge_PIE_nav_mound_top_timeout.json` retains it. The corrected check
  tests surviving ordinary navigation and removal of the obsolete lower surface.
  It does not claim that the mound top became walkable.

## Changed files, remaining gates and next task

- `Source/WYRMFALL/Public/Terrain/WyrmTerrainDiagnostics.h` and corresponding
  private `.cpp`: small development-only navigation refresh/query helpers.
- `tools/unreal/probe_geoforge_completion.py`: explicit refresh control and three
  native collision traces.
- `tools/unreal/probe_geoforge_pie_collision.py`: reproducible PIE collision/nav
  checks with bounded waits, fail-closed assertions and retained observations.
- Current STATUS, WP-00 scoped readiness, completion-inspection cross-reference,
  IntegrationReadiness, VALIDATION and this report: current scoped evidence.

WP-00 remains PARTIAL and GeoForge remains unselected. This fixes neither the
original async completion contract nor general streaming/concurrent edit handling.
The synchronous sequence is demonstrated only for this small, explicitly primed
fixture. It is not yet the gameplay adapter with action/revision callbacks.

Full WRLD/G1 and SAVE acceptance remain NOT_RUN: no useful traversable cave,
humanoid/AI/click route through new ground, active-path cancellation, finite
depletion/rewards/overflow, protected/occupied fills, water-edge gameplay,
save/quit/reload, representative streaming or repeated-edit stress was exercised.
Bootstrap was not rerun, and no cook was performed. Mutable/GAS remain unchanged.

**Next bounded task — remaining WP-00 owner inspection:** inspect actual Easy
Building System resource/interaction/building/save graphs and the supplied
inventory/water implementations; resolve one owner per responsibility before
resource/save integration. Complete only relevant real-content readiness gaps.
Use this compiled diagnostic sequence as evidence when selecting the one WP-01
candidate, not as a substitute for the remaining G1 acceptance sequence.
